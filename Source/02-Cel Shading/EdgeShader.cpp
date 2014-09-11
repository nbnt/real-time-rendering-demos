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

Filename: EdgeShader.cpp
---------------------------------------------------------------------------
*/
#include "EdgeShader.h"
#include "RtrModel.h"

CEdgeShader::CEdgeShader(ID3D11Device* pDevice)
{
    static const std::wstring ShaderFile = L"02-CelShading\\EdgeShader.hlsl";

    m_BackfacePrimVS = CreateVsFromFile(pDevice, ShaderFile, "BackfacePrimVS");
    VerifyConstantLocation(m_BackfacePrimVS->pReflector, "gVPMat", 0, offsetof(SPerFrameData, VpMat));

    m_PS = CreatePsFromFile(pDevice, ShaderFile, "PS");

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

void CEdgeShader::PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameData& PerFrameData, EDGE_MODE Mode)
{
    m_Mode = Mode;
    if(m_Mode == BACKFACING_PRIMITIVES)
    {
        pCtx->OMSetDepthStencilState(nullptr, 0);
        pCtx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        pCtx->RSSetState(m_CullFrontFaceRS);

        // Update CB
        UpdateEntireConstantBuffer(pCtx, m_PerFrameCb, PerFrameData);
        ID3D11Buffer* pCb = m_PerFrameCb.GetInterfacePtr();
        pCtx->VSSetConstantBuffers(0, 1, &pCb);
        pCtx->PSSetConstantBuffers(0, 1, &pCb);
        pCb = m_PerModelCb;
        pCtx->VSSetConstantBuffers(1, 1, &pCb);

        pCtx->VSSetShader(m_BackfacePrimVS->pShader, nullptr, 0);
        pCtx->PSSetShader(m_PS->pShader, nullptr, 0);
    }
}

void CEdgeShader::DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat)
{
	// Update constant buffer
	SPerMeshData CbData;
    CbData.World = WorldMat;
	UpdateEntireConstantBuffer(pCtx, m_PerModelCb, CbData);
	pMesh->SetDrawState(pCtx, m_BackfacePrimVS->pCodeBlob);

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CEdgeShader::DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel)
{
    if(m_Mode == BACKFACING_PRIMITIVES)
    {
        for(const auto& DrawCmd : pModel->GetDrawList())
        {
            for(const auto& Mesh : DrawCmd.pMeshes)
            {
                DrawMesh(Mesh, pCtx, DrawCmd.Transformation);
            }
        }
    }
}