/*
---------------------------------------------------------------------------
Real Time Rendering Demos
---------------------------------------------------------------------------

Copyright (c) 2014 - Nir Benty

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

Filename: TextRenderer.cpp
---------------------------------------------------------------------------*/
#include "TextRenderer.h"

static const float2 gVertexPos[] =
{
	float2(0, 0),
	float2(0, 1),
	float2(1, 0),

	float2(1, 0),
	float2(0, 1),
	float2(1, 1),
};

CTextRenderer::CTextRenderer(ID3D11Device* pDevice)
{
	CreateVertexShader(pDevice);
	CreatePixelShader(pDevice);
	CreateInputLayout(pDevice);
	CreateVertexBuffer(pDevice);
	CreateDepthStencilState(pDevice);
	CreateRasterizerState(pDevice);
	CreateConstantBuffer(pDevice);
	CreateBlendState(pDevice);
}

void CTextRenderer::SetFont(std::unique_ptr<CFont>& pFont)
{
	m_pFont = move(pFont);
	assert(m_pFont.get());
}

void CTextRenderer::Begin(ID3D11DeviceContext* pCtx, const float2& StartPos)
{
	assert(m_bInDraw == false);
	m_CurPos = StartPos;
    m_StartPos = StartPos;
	m_bInDraw = true;

	// Set shaders
	pCtx->PSSetShader(m_PS->pShader, nullptr, 0);
	pCtx->VSSetShader(m_VS->pShader, nullptr, 0);

	// Set VB
	ID3D11Buffer* pVB = m_VertexBuffer.GetInterfacePtr();
	UINT Strides = sizeof(SVertex);
	UINT Offset = 0;
	pCtx->IASetVertexBuffers(0, 1, &pVB, &Strides, &Offset);

	// Set input layout
	pCtx->IASetInputLayout(m_InputLayout);
	pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set texture
	ID3D11ShaderResourceView* pSRV = m_pFont->GetSrv();
	pCtx->PSSetShaderResources(0, 1, &pSRV);

	// Get the VP size
	D3D11_VIEWPORT vp;
	UINT NumVP = 1;
	pCtx->RSGetViewports(&NumVP, &vp);

	// Set the constant buffer
	D3D11_MAPPED_SUBRESOURCE CbData;
	verify(pCtx->Map(m_PerBatchCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &CbData));
	SPerBatchCB* pCbData = (SPerBatchCB*)CbData.pData;
	pCbData->vpTransform = DirectX::XMMatrixIdentity();
	pCbData->vpTransform._11 = 2 / vp.Width;
	pCbData->vpTransform._22 = -2 / vp.Height;
	pCbData->vpTransform._41 = -(vp.TopLeftX + vp.Width) / vp.Width;
	pCbData->vpTransform._42 = (vp.TopLeftY + vp.Height) / vp.Height;
	pCtx->Unmap(m_PerBatchCB, 0);
	ID3D11Buffer* pCB = m_PerBatchCB.GetInterfacePtr();
	pCtx->VSSetConstantBuffers(0, 1, &pCB);

	// Set state
	pCtx->OMSetDepthStencilState(m_DepthStencilState, 0);
	pCtx->RSSetState(m_RasterizerState);
	pCtx->OMSetBlendState(m_BlendState, nullptr, 0xFF);
}

void CTextRenderer::End()
{
	assert(m_bInDraw);
	m_bInDraw = false;
}

void CTextRenderer::RenderLine(ID3D11DeviceContext* pCtx, const std::wstring& line)
{ 
	assert(pCtx);
    size_t CurChar = 0;
    while(CurChar < line.size())
    {
        // Map the VB
        D3D11_MAPPED_SUBRESOURCE VbMap;
        verify(pCtx->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &VbMap));
        UINT Count = 0;
        SVertex* Vertices = (SVertex*)VbMap.pData;
        
        auto BatchEnd = min(CurChar + MaxBatchSize, line.size());
        auto BatchSize = BatchEnd - CurChar;

        UINT VertexID = 0;
        for(size_t CharIndex = 0; CharIndex < BatchSize; CharIndex++, CurChar++)
        {
            WCHAR c = line[CurChar];
            if(c == '\n')
            {
                m_CurPos.y += m_pFont->GetFontHeight();
                m_CurPos.x = m_StartPos.x;
            }
            else if(c == '\t')
            {
                m_CurPos.x += m_pFont->GetTabWidth();
            }
            else if (c == ' ')
            {
                m_CurPos.x += m_pFont->GetSpaceWidth();
            }
            else
            {
                // Regular character
                const CFont::SCharDesc& desc = m_pFont->GetCharDesc(c);
                for(UINT i = 0; i < ARRAYSIZE(gVertexPos); i++, VertexID++)
                {
					float2 Pos = desc.Size * gVertexPos[i];
                    Pos += m_CurPos;
                    Vertices[VertexID].ScreenPos = Pos;
                    Vertices[VertexID].TexCoord = desc.TopLeft + desc.Size * gVertexPos[i];
                }

                m_CurPos.x += desc.Size.x + m_pFont->GetLettersSpacing();
            }
        }
        pCtx->Unmap(m_VertexBuffer, 0);
		pCtx->Draw(VertexID, 0);
    }
    m_CurPos.y += m_pFont->GetFontHeight();
    m_CurPos.x = m_StartPos.x;
}

void CTextRenderer::CreateVertexShader(ID3D11Device* pDevice)
{
	m_VS = CreateVsFromFile(pDevice, L"Framework\\TextRenderer.hlsl", "VS");
	VerifyConstantLocation(m_VS->pReflector, "vpTransform", 0, offsetof(SPerBatchCB, vpTransform));
}

void CTextRenderer::CreatePixelShader(ID3D11Device* pDevice)
{
	m_PS = CreatePsFromFile(pDevice, L"Framework\\TextRenderer.hlsl", "PS");
	VerifyResourceLocation(m_PS->pReflector, "gFontTex", 0, 1);
}

void CTextRenderer::CreateVertexBuffer(ID3D11Device* pDevice)
{
	D3D11_BUFFER_DESC Desc;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.ByteWidth = sizeof(SVertex)*MaxBatchSize*ARRAYSIZE(gVertexPos);
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;
	Desc.StructureByteStride = 0;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	verify(pDevice->CreateBuffer(&Desc, nullptr, &m_VertexBuffer));
}

void CTextRenderer::CreateInputLayout(ID3D11Device* pDevice)
{
	assert(m_VS->pCodeBlob.GetInterfacePtr());
	D3D11_INPUT_ELEMENT_DESC desc[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, ScreenPos), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	verify(pDevice->CreateInputLayout(desc, ARRAYSIZE(desc), m_VS->pCodeBlob->GetBufferPointer(), m_VS->pCodeBlob->GetBufferSize(), &m_InputLayout));
}

void CTextRenderer::CreateDepthStencilState(ID3D11Device* pDevice)
{
	D3D11_DEPTH_STENCIL_DESC desc = {0};
	desc.StencilEnable = FALSE;
	desc.DepthEnable = FALSE;
	verify(pDevice->CreateDepthStencilState(&desc, &m_DepthStencilState));
}

void CTextRenderer::CreateRasterizerState(ID3D11Device* pDevice)
{
	D3D11_RASTERIZER_DESC desc;
	desc.AntialiasedLineEnable = FALSE;
	desc.CullMode = D3D11_CULL_NONE;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0;
	desc.DepthClipEnable = FALSE;
	desc.FillMode = D3D11_FILL_SOLID;
	desc.FrontCounterClockwise = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.ScissorEnable = FALSE;
	desc.SlopeScaledDepthBias = 0;
	verify(pDevice->CreateRasterizerState(&desc, &m_RasterizerState));
}

void CTextRenderer::CreateConstantBuffer(ID3D11Device* pDevice)
{
	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(SPerBatchCB);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;

	verify(pDevice->CreateBuffer(&desc, nullptr, &m_PerBatchCB));
}

void CTextRenderer::CreateBlendState(ID3D11Device* pDevice)
{
	D3D11_BLEND_DESC Desc;
	Desc.AlphaToCoverageEnable = FALSE;
	Desc.IndependentBlendEnable = FALSE;
	Desc.RenderTarget[0].BlendEnable = TRUE;
	Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	Desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	Desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	Desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	Desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	Desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;

	verify(pDevice->CreateBlendState(&Desc, &m_BlendState));
}