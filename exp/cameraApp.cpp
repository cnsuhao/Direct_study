//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: cameraApp.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Demonstrates using the Camera class.
//         
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include "camera.h"
#include <vector>
#include <map>
#include <sys\stat.h>

//
// Globals
//

struct Vertex
{
	Vertex(){}
	Vertex(float _x, float _y, float _z)
	{
		x = _x;  y = _y;  z = _z;
	}
	float x, y, z;
	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ;

struct VertexUV
{
	VertexUV(){}
	VertexUV(float _x, float _y, float _z, float _u, float _v)
	{
		x = _x;  y = _y;  z = _z;
		u = _u;  v = _v;
	}
	float x, y, z;
	float u, v;
	static const DWORD FVFUV;
};
const DWORD VertexUV::FVFUV = D3DFVF_XYZ | D3DFVF_TEX1;

struct STexInfo
{
	STexInfo()
	{
		nPointCount = 0;
		tex = NULL;
	}
	std::string name;
	std::string key;
	int nPointCount;
	IDirect3DTexture9* tex;
};

IDirect3DDevice9* Device = 0; 
IDirect3DVertexBuffer9* VB = 0;
int nSize = 0;

const int Width  = 640;
const int Height = 480;

Camera TheCamera(Camera::LANDOBJECT);
std::map<std::string, STexInfo*> info;
//
// Framework functions
//
bool Setup()
{
	//
	// Setup a basic scene.  The scene will be created the
	// first time this function is called.
	//

	//d3d::DrawBasicScene(Device, 0.0f); 
	
	FILE* file = fopen("test.PLAA", "rb");
	if (!file) return false;
	std::vector<VertexUV> vertexVec;
	while (!feof(file))
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 uv;
		char key[255] = { 0 };
		char StrLine[1024];
		fgets(StrLine, 1024, file);
		char material[255] = {0};
		char name[255] = { 0 };
		if (sscanf(StrLine, " material %s %s", &material, &name) == 2)
		{
			STexInfo* texInfo = new STexInfo;
			texInfo->name = name;
			texInfo->key = material;
			info.insert(std::make_pair(texInfo->key, texInfo));
		}
		else if (sscanf(StrLine, "%f,%f,%f (%f,%f) (%[^)]", &pos.x, &pos.y, &pos.z, &uv.x, &uv.y, &key) == 6)
		{
			std::string strKey = key;
			STexInfo* texInfo = info.find(strKey)->second;
			VertexUV verUv;
			verUv.x = pos.x;
			verUv.y = pos.y;
			verUv.z = pos.z;
			verUv.u = uv.x;
			verUv.v = uv.y;
			texInfo->nPointCount++;
			vertexVec.push_back(verUv);
		}
	}

	fclose(file);
	file = NULL;
	nSize = vertexVec.size();
	Device->CreateVertexBuffer(
		nSize * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Vertex* vertices;
	VB->Lock(0, 0, (void**)&vertices, 0);

	// vertices of a unit cube
	for (int i = 0; i < nSize; i++)
	{
		vertices[i].x = vertexVec[i].x;
		vertices[i].y = vertexVec[i].y;
		vertices[i].z = vertexVec[i].z;
	}

	VB->Unlock();

	//
	// Set projection matrix.
	//

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI * 0.25f, // 45 - degree
			(float)Width / (float)Height,
			1.0f,
			1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	for (std::map<std::string, STexInfo*>::iterator it = info.begin(); it != info.end(); ++it)
	{
		STexInfo* texInfo = it->second;
		D3DXCreateTextureFromFile(
			Device,
			texInfo->name.c_str(),
			&texInfo->tex);
	}

	return true;
}

void Cleanup()
{
	// pass 0 for the first parameter to instruct cleanup.
	//d3d::DrawBasicScene(0, 0.0f);
	VB->Release();
	VB = 0;
}

bool Display(float timeDelta)
{
	if( Device )
	{
		//
		// Update: Update the camera.
		//

		if( ::GetAsyncKeyState('W') & 0x8000f )
			TheCamera.walk(4.0f * timeDelta);

		if( ::GetAsyncKeyState('S') & 0x8000f )
			TheCamera.walk(-4.0f * timeDelta);

		if( ::GetAsyncKeyState('A') & 0x8000f )
			TheCamera.strafe(-4.0f * timeDelta);

		if( ::GetAsyncKeyState('D') & 0x8000f )
			TheCamera.strafe(4.0f * timeDelta);

		if( ::GetAsyncKeyState('R') & 0x8000f )
			TheCamera.fly(4.0f * timeDelta);

		if( ::GetAsyncKeyState('F') & 0x8000f )
			TheCamera.fly(-4.0f * timeDelta);

		if( ::GetAsyncKeyState(VK_UP) & 0x8000f )
			TheCamera.pitch(1.0f * timeDelta);

		if( ::GetAsyncKeyState(VK_DOWN) & 0x8000f )
			TheCamera.pitch(-1.0f * timeDelta);

		if( ::GetAsyncKeyState(VK_LEFT) & 0x8000f )
			TheCamera.yaw(-1.0f * timeDelta);
			
		if( ::GetAsyncKeyState(VK_RIGHT) & 0x8000f )
			TheCamera.yaw(1.0f * timeDelta);

		if( ::GetAsyncKeyState('N') & 0x8000f )
			TheCamera.roll(1.0f * timeDelta);

		if( ::GetAsyncKeyState('M') & 0x8000f )
			TheCamera.roll(-1.0f * timeDelta);

		// Update the view matrix representing the cameras 
        // new position/orientation.
		D3DXMATRIX V;
		TheCamera.getViewMatrix(&V);
		Device->SetTransform(D3DTS_VIEW, &V);

		//
		// Render
		//

		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();

		//d3d::DrawBasicScene(Device, 1.0f);

		Device->SetStreamSource(0, VB, 0, sizeof(Vertex));
		//Device->SetFVF(Vertex::FVF);
		Device->SetFVF(VertexUV::FVFUV);
		Device->SetMaterial(&d3d::WHITE_MTRL);

		// Draw one triangle.
		int nStartCount = 0;
		for (std::map<std::string, STexInfo*>::iterator it = info.begin(); it != info.end(); ++it)
		{
			STexInfo* texInfo = it->second;
			Device->SetTexture(0, texInfo->tex);
			//Device->SetStreamSource(0, VB, texInfo->nPointCount*sizeof(VertexUV), sizeof(VertexUV));
			Device->DrawPrimitive(D3DPT_TRIANGLELIST, nStartCount, texInfo->nPointCount / 3);
			nStartCount = nStartCount + texInfo->nPointCount;
		}

		//Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, nSize/3);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
		
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);

		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
		
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	Device->Release();

	return 0;
}
