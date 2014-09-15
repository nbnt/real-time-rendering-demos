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

Filename: ProjectTemplate.cpp
---------------------------------------------------------------------------*/
#include "ProjectTemplate.h"
#include "resource.h"
#include "RtrModel.h"
#include "ShaderTemplate.h"

#define _USE_MATH_DEFINES
#include <math.h>

const std::wstring gWindowName(L"Basic Project");
const int gWidth = 1280;
const int gHeight = 1024;
const UINT gSampleCount = 8;

HRESULT CProjectTemplate::OnCreateDevice(ID3D11Device* pDevice)
{
    m_pModel = CRtrModel::CreateFromFile(L"Tails\\Tails.obj", pDevice);
    m_Camera.SetModelParams(m_pModel->GetCenter(), m_pModel->GetRadius());
    m_pShader = std::make_unique<CShaderTemplate>(pDevice);
    InitUI();
	return S_OK;
}

void CProjectTemplate::RenderText(ID3D11DeviceContext* pContext)
{
	m_pTextRenderer->Begin(pContext, float2(10, 10));
	m_pTextRenderer->RenderLine(gWindowName);
	m_pTextRenderer->RenderLine(GetGlobalSampleMessage());
	m_pTextRenderer->End();
}

void CProjectTemplate::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pCtx)
{
    float clearColor[] = { 0.32f, 0.41f, 0.82f, 1 };
    pCtx->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
    pCtx->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);
    
    CShaderTemplate::SPerFrameData CbData;
    CbData.VpMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
    CbData.LightDirW = m_LightDir;
    CbData.LightIntensity = m_LightIntensity;
    m_pShader->PrepareForDraw(pCtx, CbData);
    m_pShader->DrawModel(pCtx, m_pModel.get());

    RenderText(pCtx);
}

void CProjectTemplate::OnDestroyDevice()
{

}

void CProjectTemplate::OnResizeWindow()
{
    float Height = float(m_Window.GetClientHeight());
    float Width = float(m_Window.GetClientWidth());

    m_Camera.SetProjectionParams(float(M_PI / 8), Width / Height);
}

void CProjectTemplate::InitUI()
{
	CGui::SetGlobalHelpMessage(wstring_2_string(gWindowName));
}

bool CProjectTemplate::OnKeyPress(WPARAM KeyCode)
{
	return false;
}

bool CProjectTemplate::OnMouseEvent(const SMouseData& Data)
{
    return m_Camera.OnMouseEvent(Data);
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	CProjectTemplate p;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	p.Run(gWindowName, gWidth, gHeight, gSampleCount, hIcon);
	return 0;
}


