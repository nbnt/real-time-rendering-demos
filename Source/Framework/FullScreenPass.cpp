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

Filename: FullScreenPass.cpp
---------------------------------------------------------------------------*/
#include "FullScreenPass.h"

struct SVertex
{
	float2 PosS;   // Screen space position
	float2 TexC;   // Texture coordinates
};

static const SVertex gFullScreenQuadVertices[] =
{
	{ float2(-1.0f, -1.0f), float2(0.0f, 1.0f) },
	{ float2(-1.0f, 1.0f), float2(0.0f, 0.0f) },
	{ float2(1.0f, -1.0f), float2(1.0f, 1.0f) },
	{ float2(1.0f, 1.0f), float2(1.0f, 0.0f) },
};

CFullScreenPass::CFullScreenPass(ID3D11Device* pDevice)
{
	const WCHAR* ShaderName = L"Framework\\FullScreenPass.hlsl";

	// Create the VS
	m_VS = CreateVsFromFile(pDevice, ShaderName, "VS");

	// Create the input-layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, PosS), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SVertex, TexC), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	verify(pDevice->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), m_VS->GetBlob()->GetBufferPointer(), m_VS->GetBlob()->GetBufferSize(), &m_InputLayout));

	// Create the vertex buffer
	D3D11_BUFFER_DESC bufDesc;
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.ByteWidth = sizeof(gFullScreenQuadVertices);
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.StructureByteStride = sizeof(SVertex);

	D3D11_SUBRESOURCE_DATA vbData;
	vbData.pSysMem = gFullScreenQuadVertices;
	vbData.SysMemPitch = sizeof(gFullScreenQuadVertices);

	verify(pDevice->CreateBuffer(&bufDesc, &vbData, &m_VB));

	m_pNoDepthTest = SDepthState::NoTests(pDevice);
}

void CFullScreenPass::Draw(ID3D11DeviceContext* pCtx, ID3D11PixelShader* pPs) const
{
	CSetDepthState Ds(pCtx, m_pNoDepthTest, 0);
	CSetVertexShader Vs(pCtx, m_VS->GetShader());
	CSetPixelShader Ps(pCtx, pPs);

	UINT stride = sizeof(SVertex);
	UINT offset = 0;
	ID3D11Buffer* pVB = m_VB.GetInterfacePtr();
	pCtx->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
	pCtx->IASetInputLayout(m_InputLayout);
	pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pCtx->Draw(4, 0);
}