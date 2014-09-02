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
#include "SolidTech.h"

#define _USE_MATH_DEFINES
#include <math.h>

const WCHAR* gWindowName = L"Model Viewer";
const int gWidth = 1280;
const int gHeight = 1024;

CModelViewer::CModelViewer() : m_LightDir(0.5f, 0, 1), m_LightIntensity(0.66f, 0.66f, 0.66f)
{
	SetWindowParams(gWindowName, gWidth, gHeight);
}


HRESULT CModelViewer::OnCreateDevice(ID3D11Device* pDevice)
{
 	m_pWireframeTech = std::make_unique<CWireframeTech>(pDevice);
 	m_pSolidTech = std::make_unique<CSolidTech>(pDevice, m_LightDir, m_LightIntensity);
	return S_OK;
}

void CModelViewer::RenderText(ID3D11DeviceContext* pContext)
{
    m_pTextRenderer->Begin(pContext, float2(10, 10));
    std::wstring line = L"Model Viewer";
    if(m_pModel)
    {
        line += L", drawing " + std::to_wstring(m_pModel->GetVertexCount()) + L" vertices, " + std::to_wstring(m_pModel->GetPrimitiveCount()) + L"primitives.";
    }
    m_pTextRenderer->RenderLine(line);
    m_pTextRenderer->RenderLine(GetFPSString());
    m_pTextRenderer->RenderLine(L"Press 'R' to reset the camera position");
    m_pTextRenderer->End();

}

void CModelViewer::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	float clearColor[] = { 0.32f, 0.41f, 0.82f, 1 };
	pContext->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
	pContext->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);

	if(m_pModel)
	{
		if(m_bWireframe)
		{
            CWireframeTech::SPerFrameCb WireframceCb;
            WireframceCb.WvpMat = m_pModel->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();;
			m_pWireframeTech->PrepareForDraw(pContext, WireframceCb);
			m_pWireframeTech->DrawModel(m_pModel.get(), pContext);
		}
		else
		{
			CSolidTech::SPerFrameCb SolidTechCB;
			SolidTechCB.VpMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
			SolidTechCB.LightIntensity = m_LightIntensity;
			SolidTechCB.LightDirW = m_LightDir;
			m_pSolidTech->PrepareForDraw(pContext, SolidTechCB);
			m_pSolidTech->DrawModel(m_pModel.get(), pContext);
		}
	}

    RenderText(pContext);
}

void CModelViewer::OnInitUI()
{
	CGui::SetGlobalHelpMessage("Sample application to load and display a model.\nUse the UI to switch between wireframe and solid mode.");
	m_pGui->AddButton("Load Model", &CModelViewer::LoadModelCallback, this);
	m_pGui->AddCheckBox("Wireframe", &m_bWireframe);
	m_pGui->AddDir3FVar("Light Direction", &m_LightDir);
	m_pGui->AddRgbColor("Light Intensity", &m_LightIntensity);
}

void CModelViewer::OnResizeWindow()
{
	float Height = float(m_Window.GetClientHeight());
	float Width = float(m_Window.GetClientWidth());

	m_Camera.SetProjectionParams(float(M_PI / 8), Width / Height, 1, 5000.0f);
}

void CModelViewer::OnDestroyDevice()
{

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

        ResetCamera();
	}
}

void CModelViewer::ResetCamera()
{
    if(m_pModel)
    {
        // update the camera position
        float Radius = m_pModel->GetRadius();
        const float3& modelCenter = m_pModel->GetCenter();
        m_Camera.SetModelParams(modelCenter, Radius);
    }
}

bool CModelViewer::OnKeyPress(WPARAM KeyCode)
{
	bool bHandled = true;
    switch(KeyCode)
    {
    case 'R':
        ResetCamera();
    default:
		bHandled = false;
    }
    return bHandled;
}

bool CModelViewer::OnMouseEvent(const SMouseData& Data)
{
	return m_Camera.OnMouseEvent(Data);
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	CModelViewer p;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	p.Run(hIcon);
	return 0;
}
