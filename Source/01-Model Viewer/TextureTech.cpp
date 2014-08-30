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

Filename: CTextureTech.cpp
---------------------------------------------------------------------------
*/
#include "TextureTech.h"
#include "Camera.h"

CTextureTech::CTextureTech(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
	m_VS = CreateVsFromFile(pDevice, L"Texture.hlsl", "VS");
	VerifyConstantLocation(m_VS->pReflector, "gWVPMat", 0, offsetof(SPerFrameCb, WvpMat));
	m_PS = CreatePsFromFile(pDevice, L"Texture.hlsl", "PS");
	VerifyResourceLocation(m_PS->pReflector, "gAlbedo", 0, 1);
	VerifySamplerLocation(m_PS->pReflector, "gLinearSampler", 0);

	// Constant buffer
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.ByteWidth = sizeof(SPerFrameCb);
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerFrameCb));

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

void CTextureTech::PrepareForDraw(ID3D11DeviceContext* pCtx, const CCamera& Camera)
{
	pCtx->OMSetDepthStencilState(nullptr, 0);
	pCtx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	pCtx->RSSetState(nullptr);
	pCtx->VSSetShader(m_VS->pShader, nullptr, 0);
	pCtx->PSSetShader(m_PS->pShader, nullptr, 0);
	
	// Update CB
	D3D11_MAPPED_SUBRESOURCE Data;
	verify(pCtx->Map(m_PerFrameCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data));
	SPerFrameCb* pCB = (SPerFrameCb*)Data.pData;
	pCB->WvpMat = Camera.GetViewMatrix() * Camera.GetProjMatrix();
	pCtx->Unmap(m_PerFrameCb, 0);

	ID3D11Buffer* pCb = m_PerFrameCb;
	pCtx->VSSetConstantBuffers(0, 1, &pCb);

	ID3D11SamplerState* pSampler = m_pLinearSampler;
	pCtx->PSSetSamplers(0, 1, &pSampler);
}

void CTextureTech::DrawMesh(const CDxMesh* pMesh, ID3D11DeviceContext* pCtx)
{
	// Set per-mesh resources
	ID3D11InputLayout* pLayout = pMesh->GetInputLayout(pCtx, m_VS->pCodeBlob);
	pCtx->IASetInputLayout(pLayout);
	pCtx->IASetPrimitiveTopology(pMesh->GetPrimitiveTopology());

	pCtx->IASetIndexBuffer(pMesh->GetIndexBuffer(), pMesh->GetIndexBufferFormat(), 0);
	ID3D11Buffer* pVB = pMesh->GetVertexBuffer();
	UINT Stride = pMesh->GetVertexStride();
	UINT Offset = 0;
	pCtx->IASetVertexBuffers(0, 1, &pVB, &Stride, &Offset);


	ID3D11ShaderResourceView* pSrv = pMesh->GetMaterial()->m_SRV[MESH_TEXTURE_DIFFUSE];
	assert(pSrv);
	pCtx->PSSetShaderResources(0, 1, &pSrv);

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CTextureTech::DrawModel(const CDxModel* pModel, ID3D11DeviceContext* pCtx)
{
	for(UINT MeshID = 0; MeshID < pModel->GetMeshesCount(); MeshID++)
	{
		const CDxMesh* pMesh = pModel->GetMesh(MeshID);
		DrawMesh(pMesh, pCtx);
	}
}