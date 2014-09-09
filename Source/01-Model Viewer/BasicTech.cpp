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

Filename: BasicTech.cpp
---------------------------------------------------------------------------
*/
#include "BasicTech.h"
#include "Camera.h"
#include "RtrModel.h"

CBasicTech::CBasicTech(ID3D11Device* pDevice, const float3& LightDir, const float3& LightIntesity) : m_LightDir(LightDir), m_LightIntensity(LightIntesity)
{
    static const std::wstring ShaderFile = L"01-ModelViewer\\BasicTech.hlsl";

    m_StaticVS = CreateVsFromFile(pDevice, ShaderFile, "VS");
	VerifyConstantLocation(m_StaticVS->pReflector, "gVPMat", 0, offsetof(SPerFrameData, VpMat));
	VerifyConstantLocation(m_StaticVS->pReflector, "gLightDirW", 0, offsetof(SPerFrameData, LightDirW));
	VerifyConstantLocation(m_StaticVS->pReflector, "gLightIntensity", 0, offsetof(SPerFrameData, LightIntensity));

	VerifyConstantLocation(m_StaticVS->pReflector, "gBones", 1, offsetof(SPerMeshData, Bones));

	const D3D_SHADER_MACRO VsDefines[] = {"_USE_BONES", "", nullptr };
    m_AnimatedVS = CreateVsFromFile(pDevice, ShaderFile, "VS", VsDefines);
	VerifyConstantLocation(m_AnimatedVS->pReflector, "gBones", 1, offsetof(SPerMeshData, Bones));

	D3D_SHADER_MACRO PsDefines[] = { "_USE_TEXTURE", "", nullptr };
    m_TexPS = CreatePsFromFile(pDevice, ShaderFile, "SolidPS", PsDefines);
	VerifyResourceLocation(m_TexPS->pReflector, "gAlbedo", 0, 1);
	VerifySamplerLocation(m_TexPS->pReflector, "gLinearSampler", 0);
    VerifyConstantLocation(m_TexPS->pReflector, "gbDoubleSided", 1, offsetof(SPerMeshData, bDoubleSided));

    m_ColorPS = CreatePsFromFile(pDevice, ShaderFile, "SolidPS");
    m_WireframePS = CreatePsFromFile(pDevice, ShaderFile, "WireframePS");

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

    // Rasterizer state
    m_pNoCullRastState = CreateSolidNoCullRasterizerState(pDevice);
    m_pWireframeRastState = CreateWireframeRasterizerState(pDevice);
}

void CBasicTech::PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameData& PerFrameData, bool bWireframe)
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
    m_bWireframe = bWireframe;
}

void CBasicTech::DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat, const CRtrModel* pModel)
{
	// Update constant buffer
	const CRtrMaterial* pMaterial = pMesh->GetMaterial();
	SPerMeshData CbData;
	CbData.bDoubleSided = pMaterial->IsDoubleSided() ? 1 : 0;
	
	const SVertexShader* pActiveVS;
	if(pMesh->HasBones())
	{
        const float4x4* pBoneTransforms = pModel->GetBonesMatrices();
        for(UINT i = 0; i < pModel->GetBonesCount(); i++)
		{
			CbData.Bones[i] = pBoneTransforms[i];
		}
		pActiveVS = m_AnimatedVS.get();
	}
	else
	{
		CbData.Bones[0] = WorldMat;
		pActiveVS = m_StaticVS.get();
	}
	UpdateEntireConstantBuffer(pCtx, m_PerModelCb, CbData);
	pMesh->SetDrawState(pCtx, pActiveVS->pCodeBlob);
	pCtx->VSSetShader(pActiveVS->pShader, nullptr, 0);

    if(m_bWireframe)
    {
        pCtx->PSSetShader(m_WireframePS->pShader, nullptr, 0);
        pCtx->RSSetState(m_pWireframeRastState);
    }
    else
    {
        // Set per-mesh resources
        ID3D11ShaderResourceView* pSrv = pMaterial->GetSRV(CRtrMaterial::DIFFUSE_MAP);

        if(pSrv)
        {
            pCtx->PSSetShader(m_TexPS->pShader, nullptr, 0);
            pCtx->PSSetShaderResources(0, 1, &pSrv);
        }
        else
        {
            pCtx->PSSetShader(m_ColorPS->pShader, nullptr, 0);
        }
        ID3D11RasterizerState* pRastState = pMaterial->IsDoubleSided() ? m_pNoCullRastState : nullptr;
        pCtx->RSSetState(pRastState);
    }

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CBasicTech::DrawModel(const CRtrModel* pModel, ID3D11DeviceContext* pCtx)
{
	for(const auto& DrawCmd : pModel->GetDrawList())
	{
		for(const auto& Mesh : DrawCmd.pMeshes)
		{
            DrawMesh(Mesh, pCtx, DrawCmd.Transformation, pModel);
		}
	}
}