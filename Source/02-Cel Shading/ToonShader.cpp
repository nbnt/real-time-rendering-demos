/*
---------------------------------------------------------------------------
Real Time Rendering Demos
---------------------------------------------------------------------------

Copyright (c) 2011 - Nir Benty

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

Filename: ToonShader.cpp
---------------------------------------------------------------------------
*/
#include "ToonShader.h"
#include "RtrModel.h"
#include "FullScreenPass.h"

enum 
{
	PER_FRAME_CB_INDEX = 0,
	PER_MESH_CB_INDEX  = 1,
	PER_TECHNIQUE_CB_INDEX = 2,

	TOON_SHADE_MAX_CB
};

CToonShader::CToonShader(ID3D11Device* pDevice, const CFullScreenPass* pFullScreenPass)
{
	static const std::wstring ShaderFile = L"02-CelShading\\ToonShader.hlsl";

    m_VS = CreateVsFromFile(pDevice, ShaderFile, "VS");
	VerifyConstantLocation(m_VS->pReflector, "gVPMat", PER_FRAME_CB_INDEX, offsetof(SCommonSettings, VpMat));
	VerifyConstantLocation(m_VS->pReflector, "gLightPosW", PER_FRAME_CB_INDEX, offsetof(SCommonSettings, LightPosW));
	VerifyConstantLocation(m_VS->pReflector, "gLightIntensity", PER_FRAME_CB_INDEX, offsetof(SCommonSettings, LightIntensity));

	VerifyConstantLocation(m_VS->pReflector, "gWorld", PER_MESH_CB_INDEX, offsetof(SPerMeshData, World));

    m_BasicDiffusePS = CreatePsFromFile(pDevice, ShaderFile, "BasicDiffusePS");
    VerifyResourceLocation(m_BasicDiffusePS->pReflector, "gAlbedo", 0, 1);
    VerifySamplerLocation(m_BasicDiffusePS->pReflector, "gLinearSampler", 0);

    m_GoochPS = CreatePsFromFile(pDevice, ShaderFile, "GoochShadingPS");

	VerifyConstantLocation(m_GoochPS->pReflector, "gColdColor", PER_TECHNIQUE_CB_INDEX, offsetof(SGoochSettings, ColdColor));
	VerifyConstantLocation(m_GoochPS->pReflector, "gColdDiffuseFactor", PER_TECHNIQUE_CB_INDEX, offsetof(SGoochSettings, ColdDiffuseFactor));
	VerifyConstantLocation(m_GoochPS->pReflector, "gWarmColor", PER_TECHNIQUE_CB_INDEX, offsetof(SGoochSettings, WarmColor));
	VerifyConstantLocation(m_GoochPS->pReflector, "gWarmDiffuseFactor", PER_TECHNIQUE_CB_INDEX, offsetof(SGoochSettings, WarmDiffuseFactor));

	m_TwoTonePS = CreatePsFromFile(pDevice, ShaderFile, "TwoTonePS");
	VerifyConstantLocation(m_TwoTonePS->pReflector, "gShadowThreshold", PER_TECHNIQUE_CB_INDEX, offsetof(STwoToneSettings, ShadowThreshold));
	VerifyConstantLocation(m_TwoTonePS->pReflector, "gShadowFactor", PER_TECHNIQUE_CB_INDEX, offsetof(STwoToneSettings, ShadowFactor));
	VerifyConstantLocation(m_TwoTonePS->pReflector, "gLightFactor", PER_TECHNIQUE_CB_INDEX, offsetof(STwoToneSettings, LightFactor));

	// Pencil shader
	m_pFullScreenPass = pFullScreenPass;
	m_BackgroundPS = CreatePsFromFile(pDevice, ShaderFile, "BackgroundPS");
	VerifyResourceLocation(m_BackgroundPS->pReflector, "gBackground", 0, 1);
	VerifySamplerLocation(m_BackgroundPS->pReflector, "gLinearSampler", 0);

	m_PencilPS = CreatePsFromFile(pDevice, ShaderFile, "PencilPS");
	VerifyResourceLocation(m_BackgroundPS->pReflector, "gPencilStrokes", 1, ARRAYSIZE(m_PencilSRV));
	VerifySamplerLocation(m_BackgroundPS->pReflector, "gLinearSampler", 0);

	// Create background texture
	m_BackgroundSRV = CreateShaderResourceViewFromFile(pDevice, L"WhitePaper.jpg", true);

	// Create the pencil strokes
	for(UINT i = 0; i < ARRAYSIZE(m_PencilSRV); i++)
	{
		std::wstring Filename = L"Pencil\\Pencil" + std::to_wstring(i+1) + L".png";
		m_PencilSRV[i] = CreateShaderResourceViewFromFile(pDevice, Filename.c_str(), true);
	}

    // Constant buffer
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    BufferDesc.ByteWidth = sizeof(SCommonSettings);
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;
    BufferDesc.StructureByteStride = 0;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerFrameCB));

    BufferDesc.ByteWidth = sizeof(SPerMeshData);
    verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerMeshCB));

	BufferDesc.ByteWidth = sizeof(SGoochSettings);
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_GoochCB));

	BufferDesc.ByteWidth = sizeof(STwoToneSettings);
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_TwoToneCB));

    // Sampler state
	m_pLinearSampler = SSamplerState::TriLinear(pDevice);
}

void CToonShader::PrepareForDraw(ID3D11DeviceContext* pCtx, const SDrawSettings& DrawSettings)
{
	pCtx->OMSetDepthStencilState(nullptr, 0);
	pCtx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	pCtx->RSSetState(nullptr);
	
	std::vector<ID3D11Buffer*> pCBs(TOON_SHADE_MAX_CB);
	pCBs[PER_FRAME_CB_INDEX] = m_PerFrameCB;
	pCBs[PER_MESH_CB_INDEX] = m_PerMeshCB;

	// Update CB
	UpdateEntireConstantBuffer(pCtx, m_PerFrameCB, DrawSettings.Common);

	ID3D11SamplerState* pSampler = m_pLinearSampler;
	pCtx->PSSetSamplers(0, 1, &pSampler);

    pCtx->VSSetShader(m_VS->pShader, nullptr, 0);
	m_Mode = DrawSettings.Mode;
    switch(m_Mode)
    {
	case BLINN_PHONG:
		pCtx->PSSetShader(m_BasicDiffusePS->pShader, nullptr, 0);
		break;
    case GOOCH_SHADING:
        pCtx->PSSetShader(m_GoochPS->pShader, nullptr, 0);
		UpdateEntireConstantBuffer(pCtx, m_GoochCB, DrawSettings.Gooch);
		pCBs[PER_TECHNIQUE_CB_INDEX] = m_GoochCB;
		break;
	case TWO_TONE_SHADING:
		pCtx->PSSetShader(m_TwoTonePS->pShader, nullptr, 0);
		UpdateEntireConstantBuffer(pCtx, m_TwoToneCB, DrawSettings.HardShading);
		pCBs[PER_TECHNIQUE_CB_INDEX] = m_TwoToneCB;
		break;
	case PENCIL_SHADING:
	{
		std::vector<ID3D11ShaderResourceView*> pStrokes(ARRAYSIZE(m_PencilSRV));
		for(UINT i = 0; i < ARRAYSIZE(m_PencilSRV); i++)
		{
			pStrokes[i] = m_PencilSRV[i];
		}
		pCtx->PSSetShader(m_PencilPS->pShader, nullptr, 0);
		pCtx->PSSetShaderResources(1, ARRAYSIZE(m_PencilSRV), &pStrokes[0]);
		break;
	}
	default:
        assert(0);
    }

	pCtx->PSSetConstantBuffers(0, pCBs.size(), &pCBs[0]);
	pCtx->VSSetConstantBuffers(0, pCBs.size(), &pCBs[0]);
}

void CToonShader::DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat)
{
	// Update constant buffer
	const CRtrMaterial* pMaterial = pMesh->GetMaterial();
	SPerMeshData CbData;
    CbData.World = WorldMat;
	UpdateEntireConstantBuffer(pCtx, m_PerMeshCB, CbData);

	pMesh->SetDrawState(pCtx, m_VS->pCodeBlob);

	if(m_Mode != PENCIL_SHADING)
	{
		// Set per-mesh resources
		ID3D11ShaderResourceView* pSrv = pMaterial->GetSRV(CRtrMaterial::DIFFUSE_MAP);
		assert(pSrv);
		pCtx->PSSetShaderResources(0, 1, &pSrv);
	}

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CToonShader::DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel)
{
	if(m_Mode == PENCIL_SHADING)
	{
		DrawPencilBackground(pCtx);
	}

	for(const auto& DrawCmd : pModel->GetDrawList())
	{
		for(const auto& Mesh : DrawCmd.pMeshes)
		{
            DrawMesh(Mesh, pCtx, DrawCmd.Transformation);
		}
	}
}

void CToonShader::DrawPencilBackground(ID3D11DeviceContext* pCtx)
{
	ID3D11ShaderResourceView* pSrv = m_BackgroundSRV.GetInterfacePtr();
	pCtx->PSSetShaderResources(0, 1, &pSrv);
	m_pFullScreenPass->Draw(pCtx, m_BackgroundPS->pShader);
}