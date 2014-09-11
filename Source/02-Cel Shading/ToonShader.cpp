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

CToonShader::CToonShader(ID3D11Device* pDevice)
{
    static const std::wstring ShaderFile = L"02-CelShading\\ToonShader.hlsl";

    m_VS = CreateVsFromFile(pDevice, ShaderFile, "VS");
    VerifyConstantLocation(m_VS->pReflector, "gVPMat", 0, offsetof(SPerFrameData, VpMat));
    VerifyConstantLocation(m_VS->pReflector, "gLightPosW", 0, offsetof(SPerFrameData, LightPosW));
    VerifyConstantLocation(m_VS->pReflector, "gLightIntensity", 0, offsetof(SPerFrameData, LightIntensity));

    VerifyConstantLocation(m_VS->pReflector, "gWorld", 1, offsetof(SPerMeshData, World));

    m_BasicDiffusePS = CreatePsFromFile(pDevice, ShaderFile, "BasicDiffusePS");
    VerifyResourceLocation(m_BasicDiffusePS->pReflector, "gAlbedo", 0, 1);
    VerifySamplerLocation(m_BasicDiffusePS->pReflector, "gLinearSampler", 0);

    m_GoochPS = CreatePsFromFile(pDevice, ShaderFile, "GoochShadingPS");

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

    // Front-face culling rasterizer state
    D3D11_RASTERIZER_DESC RsDesc;
    RsDesc.AntialiasedLineEnable = FALSE;
    RsDesc.CullMode = D3D11_CULL_FRONT;
    RsDesc.DepthBias = 0;
    RsDesc.DepthBiasClamp = 0;
    RsDesc.DepthClipEnable = FALSE;
    RsDesc.FillMode = D3D11_FILL_SOLID;
    RsDesc.FrontCounterClockwise = FALSE;
    RsDesc.MultisampleEnable = FALSE;
    RsDesc.ScissorEnable = FALSE;
    RsDesc.SlopeScaledDepthBias = 0;
    verify(pDevice->CreateRasterizerState(&RsDesc, &m_CullFrontFaceRS));
}

void CToonShader::PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameData& PerFrameData, SHADING_MODE Mode)
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
    switch(Mode)
    {
    case BASIC_DIFFUSE:
        pCtx->PSSetShader(m_BasicDiffusePS->pShader, nullptr, 0);
        break;
    case GOOCH_SHADING:
        pCtx->PSSetShader(m_GoochPS->pShader, nullptr, 0);
        break;
    default:
        assert(0);
    }
}

void CToonShader::DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat)
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