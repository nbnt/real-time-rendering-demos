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

Filename: CelShading.cpp
---------------------------------------------------------------------------*/
#include "CelShading.h"
#include "resource.h"
#include "ToonShader.h"
#include "RtrModel.h"

#define _USE_MATH_DEFINES
#include <math.h>

const WCHAR* gWindowName = L"Cel Shading";
const int gWidth = 1280;
const int gHeight = 1024;
const UINT gSampleCount = 8;

HRESULT CCelShading::OnCreateDevice(ID3D11Device* pDevice)
{
    m_pModel = CRtrModel::CreateFromFile(L"armor\\armor.obj", pDevice);
    m_Camera.SetModelParams(m_pModel->GetCenter(), m_pModel->GetRadius());
    m_pToonShader = std::make_unique<CToonShader>(pDevice);
    
    float Radius = m_pModel->GetRadius();
    m_LightPosW = float3(Radius*0.25f, Radius, -Radius*3);
	return S_OK;
}

void CCelShading::RenderText(ID3D11DeviceContext* pContext)
{
	m_pTextRenderer->Begin(pContext, float2(10, 10));
	m_pTextRenderer->RenderLine(GetGlobalSampleMessage());
	m_pTextRenderer->End();
}

void CCelShading::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pCtx)
{
	float clearColor[] = { 0, 0.17f, 0.65f, 1 };
	pCtx->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
    pCtx->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);

    CToonShader::SPerFrameData CbData;
    CbData.VpMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
    CbData.LightPosW = m_LightPosW;
    CbData.LightIntensity = m_LightIntensity;
    m_pToonShader->PrepareForDraw(pCtx, CbData, m_ShadingMode);

    m_pToonShader->DrawModel(pCtx, m_pModel.get());

	RenderText(pCtx);
}

void CCelShading::OnDestroyDevice()
{

}

void CCelShading::OnResizeWindow()
{
    float Height = float(m_Window.GetClientHeight());
    float Width = float(m_Window.GetClientWidth());

    m_Camera.SetProjectionParams(float(M_PI / 8), Width / Height);
}

void CCelShading::OnInitUI()
{
	CGui::SetGlobalHelpMessage("Cel Shading Sample");
    CGui::dropdown_list ShadingTypesList;
    ShadingTypesList.push_back({CToonShader::BASIC_DIFFUSE, "Basic Diffuse"});
    ShadingTypesList.push_back({CToonShader::GOOCH_SHADING, "Gooch Shading"});
    m_pAppGui->AddDropdown("Shading Mode", ShadingTypesList, &m_ShadingMode);
}

bool CCelShading::OnKeyPress(WPARAM KeyCode)
{
	return false;
}

bool CCelShading::OnMouseEvent(const SMouseData& Data)
{
    return m_Camera.OnMouseEvent(Data);
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	CCelShading p;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	p.Run(gWindowName, gWidth, gHeight, gSampleCount, hIcon);
	return 0;
}


