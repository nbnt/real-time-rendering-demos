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
const char* gGoochUiGroup = "Gooch Shading";
const char* gShellExpansionUiGroup = "Shell Expansion";

HRESULT CCelShading::OnCreateDevice(ID3D11Device* pDevice)
{
    m_pModel = CRtrModel::CreateFromFile(L"armor\\armor.obj", pDevice);
    m_Camera.SetModelParams(m_pModel->GetCenter(), m_pModel->GetRadius());
    m_pToonShader = std::make_unique<CToonShader>(pDevice);
    m_pSilhouetteShader = std::make_unique<CSilhouetteShader>(pDevice);

    float Radius = m_pModel->GetRadius();
    m_ToonSettings.Common.LightPosW = float3(Radius*0.25f, Radius, -Radius*3) + m_pModel->GetCenter();
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
	HandleRenderModeChange();
	AnimateLight();
	float clearColor[] = { 0, 0.17f, 0.65f, 1 };
	pCtx->ClearRenderTargetView(m_pDevice->GetBackBufferRTV(), clearColor);
    pCtx->ClearDepthStencilView(m_pDevice->GetBackBufferDSV(), D3D11_CLEAR_DEPTH, 1.0, 0);

    // Run the color shader
    m_ToonSettings.Common.VpMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
    m_pToonShader->PrepareForDraw(pCtx, m_ToonSettings);
    m_pToonShader->DrawModel(pCtx, m_pModel.get());

    // Run the edge shader
    m_SilhouetteSettings.ShellExpansion.VpMat = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
    m_pSilhouetteShader->PrepareForDraw(pCtx, m_SilhouetteSettings);
    m_pSilhouetteShader->DrawModel(pCtx, m_pModel.get());
	RenderText(pCtx);
}

void CCelShading::AnimateLight()
{
	const float RotSpeed = float(M_PI)/3;

	if(m_bAnimateLight)
	{
		float3 Pos = m_ToonSettings.Common.LightPosW - m_pModel->GetCenter();
		float ElpasedTime = m_Timer.GetElapsedTime();
		float Rot = RotSpeed * ElpasedTime;
		float4x4 RotMat = float4x4::CreateFromYawPitchRoll(Rot, Rot, 0);
		Pos = float3::Transform(Pos, RotMat);
		Pos += m_pModel->GetCenter();
		m_ToonSettings.Common.LightPosW = Pos;
	}
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

void CCelShading::HandleRenderModeChange()
{
	if(m_ShadeMode != m_ToonSettings.Mode)
	{
		SwitchToonUI(false, m_ToonSettings.Mode);
		SwitchToonUI(true, m_ShadeMode);
		m_ToonSettings.Mode = m_ShadeMode;
	}

	if(m_SilhouetteMode != m_SilhouetteSettings.Mode)
	{
		SwitchSilhouetteUI(false, m_SilhouetteSettings.Mode);
		SwitchSilhouetteUI(true, m_SilhouetteMode);
		m_SilhouetteSettings.Mode = m_SilhouetteMode;
	}
}

void CCelShading::SwitchToonUI(bool bVisible, CToonShader::SHADING_MODE Mode)
{
	switch(Mode)
	{
	case CToonShader::BLINN_PHONG:
		break;
	case CToonShader::GOOCH_SHADING:
		m_pAppGui->SetVarVisibility(gGoochUiGroup, bVisible);
		break;
	default:
		break;
	}
}

void CCelShading::SwitchSilhouetteUI(bool bVisible, CSilhouetteShader::SHADING_MODE Mode)
{
	switch(Mode)
	{
	case CSilhouetteShader::NO_SILHOUETTE:
		break;
	case CSilhouetteShader::SHELL_EXPANSION:
		m_pAppGui->SetVarVisibility(gShellExpansionUiGroup, bVisible);
		break;
	default:
		break;
	}
}

void CCelShading::OnInitUI()
{
	CGui::SetGlobalHelpMessage("Cel Shading Sample");
    CGui::dropdown_list ShadingTypesList;
	ShadingTypesList.push_back({ CToonShader::BLINN_PHONG, "Blinn-Phong" });
    ShadingTypesList.push_back({CToonShader::GOOCH_SHADING, "Gooch Shading"});
	ShadingTypesList.push_back({ CToonShader::HARD_SHADING, "Hard Shading" });
    m_pAppGui->AddDropdown("Shading Mode", ShadingTypesList, &m_ShadeMode);

    CGui::dropdown_list EdgeTypeList;
    EdgeTypeList.push_back({ CSilhouetteShader::NO_SILHOUETTE, "No Silhouette" });
    EdgeTypeList.push_back({ CSilhouetteShader::SHELL_EXPANSION, "Shell Expansion" });
    m_pAppGui->AddDropdown("Silhouette Mode", EdgeTypeList, &m_SilhouetteMode);

	m_pAppGui->AddCheckBox("Animate Light", &m_bAnimateLight);
	// Gooch UI
	const char* ColdColor = "Cold Color";
	const char* WarmColor = "Warm Color";
	const char* WarmFactor = "Warm Factor";
	const char* ColdFactor = "Cold Factor";
	m_pAppGui->AddRgbColor(ColdColor, &m_ToonSettings.Gooch.ColdColor, gGoochUiGroup);
	m_pAppGui->AddRgbColor(WarmColor, &m_ToonSettings.Gooch.WarmColor, gGoochUiGroup);
	m_pAppGui->AddFloatVar(ColdFactor, &m_ToonSettings.Gooch.ColdDiffuseFactor, gGoochUiGroup);
	m_pAppGui->AddFloatVar(WarmFactor, &m_ToonSettings.Gooch.WarmDiffuseFactor, gGoochUiGroup);
	m_pAppGui->SetVarVisibility(gGoochUiGroup, (m_ShadeMode == CToonShader::GOOCH_SHADING));

	// Shell Expansion
	const char* LineWidth = "Line Width";
	m_pAppGui->AddFloatVar(LineWidth, &m_SilhouetteSettings.ShellExpansion.LineWidth, gShellExpansionUiGroup, 0, 25, 0.1f);
	m_pAppGui->SetVarVisibility(gShellExpansionUiGroup, (m_SilhouetteMode == CSilhouetteShader::SHELL_EXPANSION));
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


