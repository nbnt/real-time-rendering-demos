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

enum 
{
	PER_FRAME_CB_INDEX = 0,
	PER_MESH_CB_INDEX  = 1,
	PER_TECHNIQUE_CB_INDEX = 2,

	TOON_SHADE_MAX_CB
};

CToonShader::CToonShader(ID3D11Device* pDevice)
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

	m_HardShadingPS = CreatePsFromFile(pDevice, ShaderFile, "HardShadingPS");
	VerifyConstantLocation(m_HardShadingPS->pReflector, "gShadowThreshold", PER_TECHNIQUE_CB_INDEX, offsetof(SHardShadingSettings, ShadowThreshold));
	VerifyConstantLocation(m_HardShadingPS->pReflector, "gShadowFactor", PER_TECHNIQUE_CB_INDEX, offsetof(SHardShadingSettings, ShadowFactor));
	VerifyConstantLocation(m_HardShadingPS->pReflector, "gLightFactor", PER_TECHNIQUE_CB_INDEX, offsetof(SHardShadingSettings, LightFactor));


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

	BufferDesc.ByteWidth = sizeof(SHardShadingSettings);
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_HardShadingCB));

    // Sampler state
    D3D11_SAMPLER_DESC SamplerDesc;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.MaxAnisotropy = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MipLODBias = 0;
    verify(pDevice->CreateSamplerState(&SamplerDesc, &m_pLinearSampler));
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
    switch(DrawSettings.Mode)
    {
	case BLINN_PHONG:
		pCtx->PSSetShader(m_BasicDiffusePS->pShader, nullptr, 0);
		break;
    case GOOCH_SHADING:
        pCtx->PSSetShader(m_GoochPS->pShader, nullptr, 0);
		UpdateEntireConstantBuffer(pCtx, m_GoochCB, DrawSettings.Gooch);
		pCBs[PER_TECHNIQUE_CB_INDEX] = m_GoochCB;
		break;
	case HARD_SHADING:
		pCtx->PSSetShader(m_HardShadingPS->pShader, nullptr, 0);
		UpdateEntireConstantBuffer(pCtx, m_HardShadingCB, DrawSettings.HardShading);
		pCBs[PER_TECHNIQUE_CB_INDEX] = m_HardShadingCB;
		break;
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
	// Set per-mesh resources
    ID3D11ShaderResourceView* pSrv = pMaterial->GetSRV(CRtrMaterial::DIFFUSE_MAP);
    assert(pSrv);
    pCtx->PSSetShaderResources(0, 1, &pSrv);

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CToonShader::DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel)
{
	for(const auto& DrawCmd : pModel->GetDrawList())
	{
		for(const auto& Mesh : DrawCmd.pMeshes)
		{
            DrawMesh(Mesh, pCtx, DrawCmd.Transformation);
		}
	}

}