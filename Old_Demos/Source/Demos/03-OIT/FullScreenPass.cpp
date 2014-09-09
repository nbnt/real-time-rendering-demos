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

    Filename: FullScreenPass.cpp
---------------------------------------------------------------------------*/
#include "FullScreenPass.h"

CFullScreenPass* CFullScreenPass::Create(ID3D11Device* pDevice)
{
    CFullScreenPass* pFullScreenPass = new CFullScreenPass;
 
    if(pFullScreenPass && pFullScreenPass->CreateEffect(pDevice))
    {
        // Create the vertex buffer
        float pos[][3] = 
        {
            {-1, -1, 0.5f},
            {1, -1, 0.5f},
            {-1, 1, 0.5f},
            {1, 1, 0.5f}
        };

        D3D11_BUFFER_DESC BufDesc;
        BufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufDesc.ByteWidth = sizeof(pos);
        BufDesc.CPUAccessFlags = 0;
        BufDesc.MiscFlags = 0;
        BufDesc.StructureByteStride = 0;
        BufDesc.Usage = D3D11_USAGE_DEFAULT;

        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = (void*)pos;
        InitData.SysMemPitch = sizeof(pos);
        InitData.SysMemSlicePitch = sizeof(pos);

        if(pDevice->CreateBuffer(&BufDesc, &InitData, &pFullScreenPass->m_pVertexBuffer) != S_OK)
        {
            trace(L"Can't create QuadMesh VB");
            PostQuitMessage(0);
            return NULL;
        }

        // Create the element layout
        D3DX11_PASS_DESC PassDesc;
        pFullScreenPass->m_pTexLdTech->GetPassByIndex(0)->GetDesc(&PassDesc);
        D3D11_INPUT_ELEMENT_DESC desc = {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};

        HRESULT hr = pDevice->CreateInputLayout(&desc, 1, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pFullScreenPass->m_pInputLayout);
        if(FAILED(hr))
        {
            trace(L"Could not create input layout for quad mesh");
            PostQuitMessage(0);
        }

        pFullScreenPass->CreateBackToFrontBlendState(pDevice);
        pFullScreenPass->CreateNoDepthTestState(pDevice);
    }

    return pFullScreenPass;
}

void CFullScreenPass::DrawBackToFrontBlend(ID3D11DeviceContext* pContext, ID3D11ShaderResourceView* pSRV)
{
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    pContext->IASetInputLayout(m_pInputLayout);
    UINT Stride = sizeof(float)*3;
    UINT Offset = 0;
    pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);
    float zero[] = {0, 0, 0, 0};
    pContext->OMSetBlendState(m_pBackToFrontBlendState, zero, 0xff);
    pContext->OMSetDepthStencilState(m_pNoDepthTestState, 0);

    m_pSrvVar->SetResource(pSRV);
    m_pTexLdTech->GetPassByIndex(0)->Apply(0, pContext);
    pContext->Draw(4, 0);
}

bool CFullScreenPass::CreateEffect(ID3D11Device* pDevice)
{
    if(D3DX11CreateEffectFromFile(L"03-OIT\\03-FullScreenPass.fx", pDevice, &m_pFX) == S_OK)
    {
        m_pTexLdTech = m_pFX->GetTechniqueByName("TexLdTech");
        if(m_pTexLdTech->IsValid() == FALSE)
        {
            trace(L"Can't create TexLdTech technique");
            PostQuitMessage(0);
            return false;
        }

        m_pSrvVar = m_pFX->GetVariableByName("gTexIn")->AsShaderResource();
        return true;
    }
    else
    {
        trace(L"Can't load FullScreenPass effect");
        PostQuitMessage(0);
        return false;
    }
}

void CFullScreenPass::CreateBackToFrontBlendState(ID3D11Device* pDevice)
{
    D3D11_BLEND_DESC BlendDesc;
    BlendDesc.AlphaToCoverageEnable = FALSE;
    BlendDesc.IndependentBlendEnable = FALSE;
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;

    if(pDevice->CreateBlendState(&BlendDesc, &m_pBackToFrontBlendState) != S_OK)
    {
        trace(L"Can't create FullScreenPass blend state");
        PostQuitMessage(0);
        return;
    }
}

void CFullScreenPass::CreateNoDepthTestState(ID3D11Device* pDevice)
{
    D3D11_DEPTH_STENCIL_DESC DepthDesc;
    ZeroMemory(&DepthDesc, sizeof(DepthDesc));

    if(pDevice->CreateDepthStencilState(&DepthDesc, &m_pNoDepthTestState) != S_OK)
    {
        trace(L"Can't create FullScreenPass NoDepthTest state");
        PostQuitMessage(0);
        return;
    }
}