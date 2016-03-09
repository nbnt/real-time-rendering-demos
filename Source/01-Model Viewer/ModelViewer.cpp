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
#include "RtrModel.h"
#include "BasicTech.h"

#define _USE_MATH_DEFINES
#include <math.h>

const WCHAR* gWindowName = L"Model Viewer";
const int gWidth = 1280;
const int gHeight = 1024;
const UINT gSampleCount = 1;

HRESULT CModelViewer::OnCreateDevice(ID3D11Device* pDevice)
{
 	m_pBasicTech = std::make_unique<CBasicTech>(pDevice);
    InitUI();
	return S_OK;
}

void CModelViewer::RenderText(ID3D11DeviceContext* pContext)
{
    m_pTextRenderer->Begin(pContext, float2(10, 10));
    std::wstring line = L"Model Viewer";
    if(m_pModel)
    {
        line += L", drawing " + std::to_wstring(m_pModel->GetVertexCount()) + L" vertices, " + std::to_wstring(m_pModel->GetPrimitiveCount()) + L" primitives.";
    }
    m_pTextRenderer->RenderLine(line);
    m_pTextRenderer->RenderLine(GetGlobalSampleMessage());
    m_pTextRenderer->RenderLine(L"Press 'R' to reset the camera position");
    m_pTextRenderer->End();

}

void CModelViewer::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pCtx)
{
	float clearColor[] = { 0.32f, 0.41f, 0.82f, 1 };
	pCtx->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
	pCtx->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);

	if(m_pModel)
	{
        if(m_ActiveAnimationID != m_SelectedAnimationID)
        {
            m_ActiveAnimationID = m_SelectedAnimationID;
            m_pModel->SetActiveAnimation(m_ActiveAnimationID);
        }
        float ElapsedTime = m_bAnimate ? m_Timer.GetElapsedTime() : 0;
        m_pModel->Animate(ElapsedTime);

        CBasicTech::SPerFrameData TechCB;
        TechCB.VpMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
        TechCB.LightIntensity = m_LightIntensity;
        TechCB.LightDirW = m_LightDir;
        m_pBasicTech->PrepareForDraw(pCtx, TechCB, m_bWireframe);
        m_pBasicTech->DrawModel(pCtx, m_pModel.get());
	}

    RenderText(pCtx);
}

void CModelViewer::InitUI()
{
	CGui::SetGlobalHelpMessage("Sample application to load and display a model.\nUse the UI to switch between wireframe and solid mode.");
	m_pAppGui->AddButton("Load Model", &CModelViewer::LoadModelCallback, this);
	m_pAppGui->AddCheckBox("Wireframe", &m_bWireframe);
	m_pAppGui->AddDir3FVar("Light Direction", &m_LightDir);
	m_pAppGui->AddRgbColor("Light Intensity", &m_LightIntensity);
}

void CModelViewer::SetAnimationUIElements()
{
    bool bAnim = m_pModel && m_pModel->HasAnimations();
    static const char* AnimateStr = "Animate";
    static const char* ActiveAnimStr = "Active Animation";

    if(bAnim)
    {
        m_SelectedAnimationID = m_ActiveAnimationID = BIND_POSE_ANIMATION_ID;

        m_pAppGui->AddCheckBox(AnimateStr, &m_bAnimate);
        CGui::dropdown_list List;
        List.resize(m_pModel->GetAnimationsCount() + 1);
        List[0].Label = "Bind Pose";
        List[0].Value = BIND_POSE_ANIMATION_ID;

        for(UINT i = 0; i < m_pModel->GetAnimationsCount(); i++)
        {
            List[i + 1].Value = i;
            List[i + 1].Label = m_pModel->GetAnimationName(i);
            if(List[i + 1].Label.size() == 0)
            {
                List[i + 1].Label = std::to_string(i);
            }
        }
        m_pAppGui->AddDropdown(ActiveAnimStr, List, &m_SelectedAnimationID);
    }
    else
    {
        m_pAppGui->RemoveVar(AnimateStr);
        m_pAppGui->RemoveVar(ActiveAnimStr);
    }
}

void CModelViewer::OnResizeWindow()
{
    float Height = float(m_Window.GetClientHeight());
    float Width = float(m_Window.GetClientWidth());

    m_Camera.SetProjectionParams(float(M_PI / 8), Width / Height);
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
	ofn.lpstrFilter = L"Supported Formats\0*.obj;*.dae;*.x;*.md5mesh\0\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"";

	ID3D11Device* pDevice = m_pDevice->GetD3DDevice();
	
	if(GetOpenFileName(&ofn))
	{
        m_pModel = CRtrModel::CreateFromFile(filename, m_pDevice->GetD3DDevice());
		
		if(m_pModel.get() == NULL)
		{
			trace(L"Could not load model");
			return;
		}

        SetAnimationUIElements();
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
	p.Run(gWindowName, gWidth, gHeight, gSampleCount, hIcon);
	return 0;
}
