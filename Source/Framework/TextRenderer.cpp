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
	m_DepthStencilState = SDepthState::NoTests(pDevice);
	m_RasterizerState = SRasterizerState::SolidNoCull(pDevice);
	m_BlendState = SBlendState::SrcAlpha(pDevice);
	CreateConstantBuffer(pDevice);
}

void CTextRenderer::SetFont(std::unique_ptr<CFont>& pFont)
{
	m_pFont = move(pFont);
	assert(m_pFont.get());
}

void CTextRenderer::Begin(ID3D11DeviceContext* pCtx, const float2& StartPos)
{
	assert(m_pContext == nullptr);
    m_pContext = pCtx;
	m_CurPos = StartPos;
    m_StartPos = StartPos;

    // Set shaders
	pCtx->PSSetShader(m_PS->GetShader(), nullptr, 0);
	pCtx->VSSetShader(m_VS->GetShader(), nullptr, 0);

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
	SPerBatchCB CbData;
	CbData.vpTransform = DirectX::XMMatrixIdentity();
	CbData.vpTransform._11 = 2 / vp.Width;
	CbData.vpTransform._22 = -2 / vp.Height;
	CbData.vpTransform._41 = -(vp.TopLeftX + vp.Width) / vp.Width;
	CbData.vpTransform._42 = (vp.TopLeftY + vp.Height) / vp.Height;
	UpdateEntireConstantBuffer(pCtx, m_PerBatchCB, CbData);

	ID3D11Buffer* pCB = m_PerBatchCB.GetInterfacePtr();
	pCtx->VSSetConstantBuffers(0, 1, &pCB);

	// Set state
	pCtx->OMSetDepthStencilState(m_DepthStencilState, 0);
	pCtx->RSSetState(m_RasterizerState);
	pCtx->OMSetBlendState(m_BlendState, nullptr, 0xFF);
}

void CTextRenderer::End()
{
	assert(m_pContext);
    m_pContext = nullptr;
}

void CTextRenderer::RenderLine(const std::wstring& line)
{ 
	assert(m_pContext);
    size_t CurChar = 0;
    while(CurChar < line.size())
    {
        // Map the VB
        D3D11_MAPPED_SUBRESOURCE VbMap;
        verify(m_pContext->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &VbMap));
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
                m_CurPos.x += m_pFont->GetLettersSpacing();
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

                m_CurPos.x += m_pFont->GetLettersSpacing();
            }
        }
        m_pContext->Unmap(m_VertexBuffer, 0);
        m_pContext->Draw(VertexID, 0);
    }
    m_CurPos.y += m_pFont->GetFontHeight();
    m_CurPos.x = m_StartPos.x;
}

void CTextRenderer::CreateVertexShader(ID3D11Device* pDevice)
{
	m_VS = CreateVsFromFile(pDevice, L"Framework\\TextRenderer.hlsl", "VS");
	m_VS->VerifyConstantLocation("vpTransform", 0, offsetof(SPerBatchCB, vpTransform));
}

void CTextRenderer::CreatePixelShader(ID3D11Device* pDevice)
{
	m_PS = CreatePsFromFile(pDevice, L"Framework\\TextRenderer.hlsl", "PS");
	m_PS->VerifyResourceLocation("gFontTex", 0, 1);
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
	assert(m_VS->GetBlob());
	D3D11_INPUT_ELEMENT_DESC desc[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, ScreenPos), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	verify(pDevice->CreateInputLayout(desc, ARRAYSIZE(desc), m_VS->GetBlob()->GetBufferPointer(), m_VS->GetBlob()->GetBufferSize(), &m_InputLayout));
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
