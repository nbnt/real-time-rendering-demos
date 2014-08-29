/*
---------------------------------------------------------------------------
Real Time Rendering Demos
---------------------------------------------------------------------------

Copyright (c) 2014 - Nir Benty

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
copyright notice, this list of conditions and the
following disclaimer.

* Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the
following disclaimer in the documentation and/or other
materials provided with the distribution.

* Neither the name of Nir Benty, nor the names of other
contributors may be used to endorse or promote products
derived from this software without specific prior
written permission from Nir Benty.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Filename: ModelViewer.cpp
---------------------------------------------------------------------------*/
#include "ModelViewer.h"
#include "resource.h"
#include "DxModel.h"
#include "WireframeTech.h"
#define _USE_MATH_DEFINES
#include <math.h>

const WCHAR* gWindowName = L"Model Viewer";
const int gWidth = 1280;
const int gHeight = 1024;

CModelViewer::CModelViewer()
{
	SetWindowParams(gWindowName, gWidth, gHeight);
}


HRESULT CModelViewer::OnCreateDevice(ID3D11Device* pDevice)
{
	m_pWireframeTech = std::make_unique<CWireframeTech>(pDevice);
	return S_OK;
}

void CModelViewer::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	float clearColor[] = { 0.32f, 0.41f, 0.82f, 1 };
	pContext->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
	pContext->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);

	if(m_pModel)
	{
		m_pWireframeTech->PrepareForDraw(pContext, m_Camera);
		m_pWireframeTech->DrawModel(m_pModel.get(), pContext);
	}

	m_pTextRenderer->Begin(pContext, float2(10, 10));
	m_pTextRenderer->RenderLine(pContext, L"Model Viewer");
    m_pTextRenderer->RenderLine(pContext, GetFPSString());
	m_pTextRenderer->End();
}

void CModelViewer::OnInitUI()
{
	CGui::SetGlobalHelpMessage("Sample application to load and display a model.\nUse the UI to switch between wireframe and solid mode.");
	m_pGui->AddButton("Load Model", &CModelViewer::LoadModelCallback, this);
}

void CModelViewer::OnResizeWindow()
{
	float Height = float(m_Window.GetClientHeight());
	float Width = float(m_Window.GetClientWidth());

	m_Camera.SetProjectionParams(float(M_PI / 4), Width / Height, 0.1f, 1000.0f);
}

void CModelViewer::OnDestroyDevice()
{

}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	CModelViewer p;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	p.Run(hIcon);
	return 0;
}

void GUI_CALL CModelViewer::LoadModelCallback(void* pUserData)
{
	CModelViewer* pViewer = reinterpret_cast<CModelViewer*>(pUserData);
	pViewer->LoadModel();
}

void CModelViewer::LoadModel()
{
	OPENFILENAME ofn;
	WCHAR filename[MAX_PATH] = L"";
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"";

	ID3D11Device* pDevice = m_pDevice->GetD3DDevice();
	
	if(GetOpenFileName(&ofn))
	{
		m_pModel = std::unique_ptr<CDxModel>(CDxModel::LoadModelFromFile(filename, m_pDevice->GetD3DDevice()));
		
		if(m_pModel.get() == NULL)
		{
			trace(L"Could not load model");
			return;
		}

		// update the camera position
		float Radius = m_pModel->GetRadius();

//		m_Camera.SetRadius(fRadius * 2, fRadius * 0.25f);

		const float3& modelCenter = m_pModel->GetCenter();
//		m_Camera.SetModelCenter(modelCenter);
		float3 up(0, 1, 0);
		float3 CameraPosition(modelCenter.x, modelCenter.y, -Radius * 3);
		m_Camera.SetViewParams(CameraPosition, modelCenter, up);
	
		m_VertexCount = 0;
		for(UINT i = 0; i < m_pModel->GetMeshesCount(); i++)
		{
			m_VertexCount += m_pModel->GetMesh(i)->GetVertexCount();
		}
	}
}