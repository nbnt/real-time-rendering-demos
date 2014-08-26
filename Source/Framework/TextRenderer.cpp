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

static const CTextRenderer::SVertex gVertexData[] = 
{
	{ float2(-1, -1), float2(0, 0) },
	{ float2(-1, 1), float2(1, 0) },
	{ float2(1, -1), float2(1, 1) },
	{ float2(1, 1), float2(0, 1) },
};

CTextRenderer::CTextRenderer(ID3D11Device* pDevice)
{
	CreateVertexShader(pDevice);
	CreatePixelShader(pDevice);
	CreateInputLayout(pDevice);
	CreateVertexBuffer(pDevice);
	CreateDepthStencilState(pDevice);
}

void CTextRenderer::SetFont(std::unique_ptr<CFont>& pFont)
{
	m_pFont = move(pFont);
	assert(m_pFont.get());
}

void CTextRenderer::Begin(ID3D11DeviceContext* pCtx, float2 StartPos)
{
	assert(m_bInDraw == false);
	m_CurPos = StartPos;
	m_bInDraw = true;

	// Set shaders
	pCtx->PSSetShader(m_pPS->pShader, nullptr, 0);
	pCtx->VSSetShader(m_pVS->pShader, nullptr, 0);

	// Set VB
	ID3D11Buffer* pVB = m_pVertexBuffer.GetInterfacePtr();
	UINT Strides = sizeof(SVertex);
	UINT Offset = 0;
	pCtx->IASetVertexBuffers(0, 1, &pVB, &Strides, &Offset);

	// Set input layout
	pCtx->IASetInputLayout(m_pInputLayout);
	pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Set texture
	ID3D11ShaderResourceView* pSRV = m_pFont->GetSrv();
	pCtx->PSSetShaderResources(0, 1, &pSRV);

	// Get the VP size
	D3D11_VIEWPORT vp;
	UINT NumVP = 1;
	pCtx->RSGetViewports(&NumVP, &vp);
	m_VpFactor = float2(1 / vp.Width, 1 / vp.Height);

	// Set state
	pCtx->OMSetDepthStencilState(m_pDepthStencilState, 0);
}

void CTextRenderer::End()
{
	assert(m_bInDraw);
	m_bInDraw = false;
}

void CTextRenderer::RenderLine(ID3D11DeviceContext* pCtx, const std::wstring& line)
{ 
	assert(pCtx);
	// Prepare the vertex-buffer
	D3D11_MAPPED_SUBRESOURCE VbMap;
	verify(pCtx->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &VbMap));
	UINT Count = 0;
	SVertex* Vertices = (SVertex*)VbMap.pData;
	const CFont::SCharDesc& desc = m_pFont->GetCharDesc(line[0]);

	for(UINT i = 0 ; i < 4 ; i++)
	{
		float2 Pos = desc.Size * gVertexData[i].ScreenPos;
		Pos += m_CurPos;
		Pos *= m_VpFactor;
		Vertices[i].ScreenPos = Pos;
	}
	pCtx->Unmap(m_pVertexBuffer, 0);

	pCtx->Draw(4, 0);
}

void CTextRenderer::CreateVertexShader(ID3D11Device* pDevice)
{
	m_pVS = CreateVsFromFile(pDevice, L"Framework\\TextRenderer.hlsl", "VS");
}

void CTextRenderer::CreatePixelShader(ID3D11Device* pDevice)
{
	m_pPS = CreatePsFromFile(pDevice, L"Framework\\TextRenderer.hlsl", "PS");
	VerifyResourceLocation(m_pPS->pReflector, "gFontTex", 0, 1);
}

void CTextRenderer::CreateVertexBuffer(ID3D11Device* pDevice)
{
	D3D11_BUFFER_DESC Desc;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.ByteWidth = sizeof(SVertex)*MaxBatchSize;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;
	Desc.StructureByteStride = 0;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	verify(pDevice->CreateBuffer(&Desc, nullptr, &m_pVertexBuffer));
}

void CTextRenderer::CreateInputLayout(ID3D11Device* pDevice)
{
	assert(m_pVS->pCodeBlob.GetInterfacePtr());
	D3D11_INPUT_ELEMENT_DESC desc[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, ScreenPos), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	verify(pDevice->CreateInputLayout(desc, ARRAYSIZE(desc), m_pVS->pCodeBlob->GetBufferPointer(), m_pVS->pCodeBlob->GetBufferSize(), &m_pInputLayout));
}

void CTextRenderer::CreateDepthStencilState(ID3D11Device* pDevice)
{
	D3D11_DEPTH_STENCIL_DESC desc = {0};
	desc.StencilEnable = FALSE;
	desc.DepthEnable = FALSE;
	verify(pDevice->CreateDepthStencilState(&desc, &m_pDepthStencilState));
}