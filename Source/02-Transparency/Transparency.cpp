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

Filename: Transparency.cpp
---------------------------------------------------------------------------*/
#include "Transparency.h"
#include "resource.h"
#include "DxModel.h"
#include "TransparencyTech.h"

#define _USE_MATH_DEFINES
#include <math.h>

const WCHAR* gWindowName = L"Transparency";
const int gWidth = 1280;
const int gHeight = 1024;

const WCHAR* gModelName = L"Engine.obj";

CTransparency::CTransparency() : m_LightDir(0.5f, 0, 1), m_LightIntensity(1, 1, 1), m_Alpha(0.5f), m_TransparencyMode(CTransparencyTech::TransparencyMode::Solid)
{
	SetWindowParams(gWindowName, gWidth, gHeight);
}


HRESULT CTransparency::OnCreateDevice(ID3D11Device* pDevice)
{
	// Load the model
	LoadModel(pDevice);
	m_pTransparencyTech = std::make_unique<CTransparencyTech>(pDevice);
	
	return S_OK;
}

void CTransparency::LoadModel(ID3D11Device* pDevice)
{
	std::wstring Fullpath;
	verify(FindFileInCommonDirs(gModelName, Fullpath));
	m_pModel = std::unique_ptr<CDxModel>(CDxModel::LoadModelFromFile(gModelName, pDevice));

	if(m_pModel.get() == NULL)
	{
		trace(L"Could not load model");
		return;
	}

	ResetCamera();
}

void CTransparency::ResetCamera()
{
	if(m_pModel)
	{
		// update the camera position
		float Radius = m_pModel->GetRadius();
		const float3& modelCenter = m_pModel->GetCenter();
		m_Camera.SetModelParams(modelCenter, Radius);
	}
}

void CTransparency::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	float clearColor[] = { 0.32f, 0.41f, 0.82f, 1 };
	pContext->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
	pContext->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	CTransparencyTech::SPerModelCb Cb;
	Cb.Wvp = m_pModel->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
	Cb.World = m_pModel->GetWorldMatrix();
	Cb.LightIntensity = m_LightIntensity;
	Cb.LightDirW = m_LightDir;
	Cb.AlphaOut = m_Alpha;

	m_pTransparencyTech->PrepareForDraw(pContext, Cb, m_TransparencyMode);
	m_pTransparencyTech->DrawModel(m_pModel.get(), pContext);

	m_pTextRenderer->Begin(pContext, float2(10, 10));
	m_pTextRenderer->RenderLine(L"Transparency techniques");
    m_pTextRenderer->RenderLine(GetFPSString());
	m_pTextRenderer->End();
}

void CTransparency::OnDestroyDevice()
{

}

void CTransparency::OnResizeWindow()
{
	float Height = float(m_Window.GetClientHeight());
	float Width = float(m_Window.GetClientWidth());

	m_Camera.SetProjectionParams(float(M_PI / 8), Width / Height, 0.1f, 10000.0f);
}

void CTransparency::OnInitUI()
{
	CGui::SetGlobalHelpMessage("Transparency techniques overview");
	// mode dropdown
	CGui::dropdown_list ModeList = 
	{
		{ (int)CTransparencyTech::TransparencyMode::Solid, "Solid"},
		{ (int)CTransparencyTech::TransparencyMode::UnorderedBlend, "Unordered blending" }
	};
	m_pGui->AddDropdown("Transparency mode", ModeList, &m_TransparencyMode);

	// light direction
	m_pGui->AddDir3FVar("Light direction", &m_LightDir);
	m_pGui->AddRgbColor("Light intensity", &m_LightIntensity);
	m_pGui->AddFloatVar("Alpha", &m_Alpha);
}

bool CTransparency::OnMouseEvent(const SMouseData& Data)
{
	return m_Camera.OnMouseEvent(Data);
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	CTransparency p;
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	p.Run(hIcon);
	return 0;
}
