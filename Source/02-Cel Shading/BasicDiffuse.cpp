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

Filename: BasicDiffuse.cpp
---------------------------------------------------------------------------
*/
#include "BasicDiffuse.h"
#include "RtrModel.h"

CBasicDiffuse::CBasicDiffuse(ID3D11Device* pDevice)
{
    static const std::wstring ShaderFile = L"02-CelShading\\BasicDiffuse.hlsl";

    m_VS = CreateVsFromFile(pDevice, ShaderFile, "VS");
    VerifyConstantLocation(m_VS->pReflector, "gVPMat", 0, offsetof(SPerFrameData, VpMat));
    VerifyConstantLocation(m_VS->pReflector, "gLightDirW", 0, offsetof(SPerFrameData, LightDirW));
    VerifyConstantLocation(m_VS->pReflector, "gLightIntensity", 0, offsetof(SPerFrameData, LightIntensity));

    VerifyConstantLocation(m_VS->pReflector, "gWorld", 1, offsetof(SPerMeshData, World));

    m_PS = CreatePsFromFile(pDevice, ShaderFile, "PS");
	VerifyResourceLocation(m_PS->pReflector, "gAlbedo", 0, 1);
	VerifySamplerLocation(m_PS->pReflector, "gLinearSampler", 0);

	// Constant buffer
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.ByteWidth = sizeof(SPerFrameData);
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerFrameCb));

	BufferDesc.ByteWidth = sizeof(SPerMeshData);
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerModelCb));

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

void CBasicDiffuse::PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameData& PerFrameData)
{
	pCtx->OMSetDepthStencilState(nullptr, 0);
	pCtx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	pCtx->RSSetState(nullptr);
	
	// Update CB
	UpdateEntireConstantBuffer(pCtx, m_PerFrameCb, PerFrameData);
	ID3D11Buffer* pCb = m_PerFrameCb.GetInterfacePtr();
	pCtx->VSSetConstantBuffers(0, 1, &pCb);
	pCtx->PSSetConstantBuffers(0, 1, &pCb);
	pCb = m_PerModelCb;
	pCtx->VSSetConstantBuffers(1, 1, &pCb);

	ID3D11SamplerState* pSampler = m_pLinearSampler;
	pCtx->PSSetSamplers(0, 1, &pSampler);

    pCtx->VSSetShader(m_VS->pShader, nullptr, 0);
    pCtx->PSSetShader(m_PS->pShader, nullptr, 0);
}

void CBasicDiffuse::DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat)
{
	// Update constant buffer
	const CRtrMaterial* pMaterial = pMesh->GetMaterial();
	SPerMeshData CbData;
    CbData.World = WorldMat;
	UpdateEntireConstantBuffer(pCtx, m_PerModelCb, CbData);

	pMesh->SetDrawState(pCtx, m_VS->pCodeBlob);
	// Set per-mesh resources
    ID3D11ShaderResourceView* pSrv = pMaterial->GetSRV(CRtrMaterial::DIFFUSE_MAP);
    assert(pSrv);
    pCtx->PSSetShaderResources(0, 1, &pSrv);

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CBasicDiffuse::DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel)
{
	for(const auto& DrawCmd : pModel->GetDrawList())
	{
		for(const auto& Mesh : DrawCmd.pMeshes)
		{
            DrawMesh(Mesh, pCtx, DrawCmd.Transformation);
		}
	}
}