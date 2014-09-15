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

Filename: BRDF.cpp
---------------------------------------------------------------------------*/
#include "BRDF.h"
#include "resource.h"
#include "RtrModel.h"
#include "BrdfShader.h"

#define _USE_MATH_DEFINES
#include <math.h>

const std::wstring gWindowName(L"BRDF");
const int gWidth = 1280;
const int gHeight = 1024;
const UINT gSampleCount = 8;

HRESULT CBrdf::OnCreateDevice(ID3D11Device* pDevice)
{
    m_pModel = CRtrModel::CreateFromFile(L"test_scene.obj", pDevice);
    m_Camera.SetModelParams(m_pModel->GetCenter(), m_pModel->GetRadius());
    m_pShader = std::make_unique<CBrdfShader>(pDevice);
	return S_OK;
}

void CBrdf::RenderText(ID3D11DeviceContext* pContext)
{
	m_pTextRenderer->Begin(pContext, float2(10, 10));
	std::wstring line(gWindowName);
	m_pTextRenderer->RenderLine(gWindowName);
	m_pTextRenderer->RenderLine(GetGlobalSampleMessage());
	m_pTextRenderer->End();
}

void CBrdf::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pCtx)
{
    float clearColor[] = { 0.32f, 0.41f, 0.82f, 1 };
    pCtx->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
    pCtx->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);
    
    CBrdfShader::SPerFrameData CbData;
    CbData.VpMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
    CbData.LightPosW = m_LightPosW;
    CbData.DiffuseIntensity = m_DiffuseIntensity;
    CbData.AmbientIntensity = m_AmbientIntensity;
    CbData.ModelColor = m_ModelColor;
    m_pShader->PrepareForDraw(pCtx, CbData);
    m_pShader->DrawModel(pCtx, m_pModel.get());

    RenderText(pCtx);
}

void CBrdf::OnDestroyDevice()
{

}

void CBrdf::OnResizeWindow()
{
    float Height = float(m_Window.GetClientHeight());
    float Width = float(m_Window.GetClientWidth());

    m_Camera.SetProjectionParams(float(M_PI / 8), Width / Height);
}

void CBrdf::OnInitUI()
{
	CGui::SetGlobalHelpMessage(wstring_2_string(gWindowName));
    m_pAppGui->AddFloatVar("X", &m_LightPosW.x, "Light Position", -1000, 1000, 0.5);
    m_pAppGui->AddFloatVar("Y", &m_LightPosW.y, "Light Position", -1000, 1000, 0.5);
    m_pAppGui->AddFloatVar("Z", &m_LightPosW.z, "Light Position", -1000, 1000, 0.5);
    m_pAppGui->AddRgbColor("Diffuse Intensity", &m_DiffuseIntensity);
    m_pAppGui->AddRgbColor("Ambient Intensity", &m_AmbientIntensity);
    m_pAppGui->AddRgbColor("Model Color", &m_ModelColor);
}

bool CBrdf::OnKeyPress(WPARAM KeyCode)
{
	return false;
}

bool CBrdf::OnMouseEvent(const SMouseData& Data)
{
    bool bHandled = m_Camera.OnMouseEvent(Data);
    if(bHandled == false)
    {
        switch(Data.Event)
        {
        case WM_RBUTTONDOWN:
            m_bRightButtonDown = true;
            m_LastMousePos = Data.Crd;
            bHandled = true;
            break;
        case WM_RBUTTONUP:
            m_bRightButtonDown = false;
            break;
        case WM_MOUSEMOVE:
            if(m_bRightButtonDown)
            {
                float2 Delta = Data.Crd - m_LastMousePos;
                Delta *= 30;
                m_LightPosW.x += Delta.x;
                m_LightPosW.z += Delta.y;
                m_LastMousePos = Data.Crd;
                bHandled = true;
                m_pAppGui->Refresh();
                break;
            }
        }
    }
    return bHandled;
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	CBrdf p;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	p.Run(gWindowName, gWidth, gHeight, gSampleCount, hIcon);
	return 0;
}


