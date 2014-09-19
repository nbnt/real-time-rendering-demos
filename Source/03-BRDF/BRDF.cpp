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
    m_pModel = CRtrModel::CreateFromFile(L"stanford models\\dragon.obj", pDevice);
    m_Camera.SetModelParams(m_pModel->GetCenter(), m_pModel->GetRadius());
    m_pShader = std::make_unique<CBrdfShader>(pDevice);

    InitUI();
	return S_OK;
}

void CBrdf::RenderText(ID3D11DeviceContext* pContext)
{
	m_pTextRenderer->Begin(pContext, float2(10, 10));
	std::wstring line(gWindowName);
	m_pTextRenderer->RenderLine(gWindowName);
	m_pTextRenderer->RenderLine(GetGlobalSampleMessage() + L"\n'B' cycles between BRDF modes");
	m_pTextRenderer->End();
}

void CBrdf::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pCtx)
{
    float clearColor[] = { 0.32f, 0.41f, 0.82f, 1 };
    pCtx->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
    pCtx->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);
    
    if(m_LightCutoffEnd <= m_LightCutoffStart)
    {
        m_LightCutoffEnd = m_LightCutoffStart + 1;
    }
    m_ShaderData.VPMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
    m_ShaderData.CameraPosW = m_Camera.GetPosition();
    m_ShaderData.AmbientIntensity = m_bAmbientEnabled ? m_AmbientIntensity : float3(0,0,0);
    m_ShaderData.SpecularColor = m_bSpecularEnabled ? m_SpecularColor : float3(0,0,0);
    m_ShaderData.DiffuseColor = m_bDiffuseEnabled ? m_DiffuseColor : float3(0,0,0);
    
    m_ShaderData.CutoffScale = 1.0f/(m_LightCutoffEnd - m_LightCutoffStart);
    m_ShaderData.CutoffOffset = -m_LightCutoffStart / (m_LightCutoffEnd - m_LightCutoffStart);

    m_pShader->PrepareForDraw(pCtx, m_ShaderData, m_BrdfModel);
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

void CBrdf::InitUI()
{
	CGui::SetGlobalHelpMessage(wstring_2_string(gWindowName));
    
    CGui::dropdown_list SpecModes;
    SpecModes.push_back({(int)CBrdfShader::BRDF_MODEL::NO_BRDF, "None"});
    SpecModes.push_back({ (int)CBrdfShader::BRDF_MODEL::PHONG, "Phong" });
    SpecModes.push_back({ (int)CBrdfShader::BRDF_MODEL::BLINN_PHONG, "Blinn-Phong" });
    m_pAppGui->AddDropdown("BRDF Model", SpecModes, &m_BrdfModel);

    m_pAppGui->AddFloatVar("X", &m_ShaderData.LightPosW.x, "Light Position", -1000, 1000, 0.5);
    m_pAppGui->AddFloatVar("Y", &m_ShaderData.LightPosW.y, "Light Position", -1000, 1000, 0.5);
    m_pAppGui->AddFloatVar("Z", &m_ShaderData.LightPosW.z, "Light Position", -1000, 1000, 0.5);
    m_pAppGui->AddCheckBox("Enable Diffuse", &m_bDiffuseEnabled);
    m_pAppGui->AddCheckBox("Enable Specular", &m_bSpecularEnabled);
    m_pAppGui->AddCheckBox("Enable Ambient", &m_bAmbientEnabled);
    m_pAppGui->AddRgbColor("Light Intensity", &m_ShaderData.LightIntensity);
    m_pAppGui->AddRgbColor("Ambient Intensity", &m_ShaderData.AmbientIntensity);
    m_pAppGui->AddFloatVar("Cutoff Start", &m_LightCutoffStart, "", 0, 100, 1);
    m_pAppGui->AddFloatVar("Cutoff End", &m_LightCutoffEnd, "", 10, 1000, 1);

    m_pAppGui->AddRgbColor("Diffuse Color", &m_DiffuseColor, "Material");
    m_pAppGui->AddRgbColor("Specular Color", &m_SpecularColor, "Material");
    m_pAppGui->AddFloatVar("Specular Power", &m_ShaderData.SpecPower, "Material", 0.1f, 100, 0.2f);
}

bool CBrdf::OnKeyPress(WPARAM KeyCode)
{
    if(KeyCode == 'B')
    {
        m_BrdfModel = (CBrdfShader::BRDF_MODEL)((m_BrdfModel + 1) % CBrdfShader::BRDF_COUNT);
        m_pAppGui->Refresh();
        return true;
    }
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
        case WM_MBUTTONDOWN:
            m_bMiddleButtonDown = true;
            m_LastMousePos = Data.Crd;
            bHandled = true;
            break;
        case WM_RBUTTONUP:
            m_bRightButtonDown = false;
            break;
        case WM_MBUTTONUP:
            m_bMiddleButtonDown = false;
            break;
        case WM_MOUSEMOVE:
            if(m_bRightButtonDown || m_bMiddleButtonDown)
            {
                float2 Delta = Data.Crd - m_LastMousePos;
                Delta *= 30;
                if(m_bRightButtonDown)
                {
                    m_ShaderData.LightPosW.x += Delta.x;
                    m_ShaderData.LightPosW.z += Delta.y;
                }
                if(m_bMiddleButtonDown)
                {
                    m_ShaderData.LightPosW.y = max(0, m_ShaderData.LightPosW.y + Delta.y);
                }
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


