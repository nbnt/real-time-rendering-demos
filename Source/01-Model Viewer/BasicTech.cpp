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

CBasicTech::CBasicTech(ID3D11Device* pDevice)
{
    static const std::wstring ShaderFile = L"01-ModelViewer\\BasicTech.hlsl";

    m_StaticVS = CreateVsFromFile(pDevice, ShaderFile, "VS");
	m_StaticVS->VerifyConstantLocation("gVPMat", 0, offsetof(SPerFrameData, VpMat));
	m_StaticVS->VerifyConstantLocation("gLightDirW", 0, offsetof(SPerFrameData, LightDirW));
	m_StaticVS->VerifyConstantLocation("gLightIntensity", 0, offsetof(SPerFrameData, LightIntensity));

	m_StaticVS->VerifyConstantLocation("gBones", 1, offsetof(SPerMeshData, Bones));

	const D3D_SHADER_MACRO VsDefines[] = {"_USE_BONES", "", nullptr };
    m_AnimatedVS = CreateVsFromFile(pDevice, ShaderFile, "VS", VsDefines);
	m_AnimatedVS->VerifyConstantLocation("gBones", 1, offsetof(SPerMeshData, Bones));

	D3D_SHADER_MACRO PsDefines[] = { "_USE_TEXTURE", "", nullptr };
    m_TexPS = CreatePsFromFile(pDevice, ShaderFile, "SolidPS", PsDefines);
	m_TexPS->VerifyResourceLocation("gAlbedo", 0, 1);
	m_TexPS->VerifySamplerLocation("gLinearSampler", 0);
	m_TexPS->VerifyConstantLocation("gbDoubleSided", 1, offsetof(SPerMeshData, bDoubleSided));

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
	m_pLinearSampler = SSamplerState::TriLinear(pDevice);

    // Rasterizer state
    m_pNoCullRastState = SRasterizerState::SolidNoCull(pDevice);
    m_pWireframeRastState = SRasterizerState::Wireframe(pDevice);
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
	
	const CVertexShader* pActiveVS;
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
	pMesh->SetDrawState(pCtx, pActiveVS->GetBlob());
	pCtx->VSSetShader(pActiveVS->GetShader(), nullptr, 0);

    if(m_bWireframe)
    {
        pCtx->PSSetShader(m_WireframePS->GetShader(), nullptr, 0);
        pCtx->RSSetState(m_pWireframeRastState);
    }
    else
    {
        // Set per-mesh resources
        ID3D11ShaderResourceView* pSrv = pMaterial->GetSRV(CRtrMaterial::DIFFUSE_MAP);

        if(pSrv)
        {
            pCtx->PSSetShader(m_TexPS->GetShader(), nullptr, 0);
            pCtx->PSSetShaderResources(0, 1, &pSrv);
        }
        else
        {
            pCtx->PSSetShader(m_ColorPS->GetShader(), nullptr, 0);
        }
        ID3D11RasterizerState* pRastState = pMaterial->IsDoubleSided() ? m_pNoCullRastState : nullptr;
        pCtx->RSSetState(pRastState);
    }

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CBasicTech::DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel)
{
	for(const auto& DrawCmd : pModel->GetDrawList())
	{
		for(const auto& Mesh : DrawCmd.pMeshes)
		{
            DrawMesh(Mesh, pCtx, DrawCmd.Transformation, pModel);
		}
	}
}