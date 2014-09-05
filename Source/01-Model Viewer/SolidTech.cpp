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

Filename: SolidTech.cpp
---------------------------------------------------------------------------
*/
#include "SolidTech.h"
#include "Camera.h"
#include "RtrModel.h"

CSolidTech::CSolidTech(ID3D11Device* pDevice, const float3& LightDir, const float3& LightIntesity) : m_LightDir(LightDir), m_LightIntensity(LightIntesity)
{
    HRESULT hr = S_OK;
	m_VS = CreateVsFromFile(pDevice, L"01-ModelViewer\\Solid.hlsl", "VS");
	VerifyConstantLocation(m_VS->pReflector, "gVPMat", 0, offsetof(SPerFrameData, VpMat));
	VerifyConstantLocation(m_VS->pReflector, "gLightDirW", 0, offsetof(SPerFrameData, LightDirW));
	VerifyConstantLocation(m_VS->pReflector, "gLightIntensity", 0, offsetof(SPerFrameData, LightIntensity));

	VerifyConstantLocation(m_VS->pReflector, "gWorld", 1, offsetof(SPerDrawCmdData, WorldMat));

	D3D_SHADER_MACRO defines[] = { "_USE_TEXTURE", "", nullptr };
	m_TexPS = CreatePsFromFile(pDevice, L"01-ModelViewer\\Solid.hlsl", "PS", defines);
	VerifyResourceLocation(m_TexPS->pReflector, "gAlbedo", 0, 1);
	VerifySamplerLocation(m_TexPS->pReflector, "gLinearSampler", 0);

	m_ColorPS = CreatePsFromFile(pDevice, L"01-ModelViewer\\Solid.hlsl", "PS");

	// Constant buffer
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.ByteWidth = sizeof(SPerFrameData);
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerFrameCb));

	BufferDesc.ByteWidth = sizeof(SPerDrawCmdData);
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

void CSolidTech::PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameData& PerFrameData)
{
	pCtx->OMSetDepthStencilState(nullptr, 0);
	pCtx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	pCtx->RSSetState(nullptr);
	pCtx->VSSetShader(m_VS->pShader, nullptr, 0);
	
	// Update CB
	UpdateEntireConstantBuffer(pCtx, m_PerFrameCb, PerFrameData);
	ID3D11Buffer* pCb = m_PerFrameCb.GetInterfacePtr();
	pCtx->VSSetConstantBuffers(0, 1, &pCb);
	pCtx->PSSetConstantBuffers(0, 1, &pCb);
	pCb = m_PerModelCb;
	pCtx->VSSetConstantBuffers(1, 1, &pCb);

	ID3D11SamplerState* pSampler = m_pLinearSampler;
	pCtx->PSSetSamplers(0, 1, &pSampler);
}

void CSolidTech::DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx)
{
	// Set per-mesh resources
    pMesh->SetDrawState(pCtx, m_VS->pCodeBlob);
	ID3D11ShaderResourceView* pSrv = pMesh->GetMaterial()->GetSRV(CRtrMaterial::DIFFUSE_MAP);
	if(pSrv)
	{
		pCtx->PSSetShader(m_TexPS->pShader, nullptr, 0);
		pCtx->PSSetShaderResources(0, 1, &pSrv);
	}
	else
	{
		pCtx->PSSetShader(m_ColorPS->pShader, nullptr, 0);
	}

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CSolidTech::DrawModel(const CRtrModel* pModel, ID3D11DeviceContext* pCtx)
{
	for(const auto& DrawCmd : pModel->GetDrawList())
	{
		// Set the world matrix
		SPerDrawCmdData CbData;
		CbData.WorldMat = DrawCmd.Transformation;
		UpdateEntireConstantBuffer(pCtx, m_PerModelCb, CbData);

		for(const auto& Mesh : DrawCmd.pMeshes)
		{
			DrawMesh(Mesh, pCtx);
		}
	}
}