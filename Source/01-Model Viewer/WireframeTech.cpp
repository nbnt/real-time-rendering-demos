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

Filename: WireframeTech.cpp
---------------------------------------------------------------------------
*/
#include "WireframeTech.h"
#include "Camera.h"
#include "RtrModel.h"

CWireframeTech::CWireframeTech(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
	m_VS = CreateVsFromFile(pDevice, L"01-ModelViewer\\Wireframe.hlsl", "VS");
	VerifyConstantLocation(m_VS->pReflector, "gWVPMat", 0, offsetof(SPerFrameCb, WvpMat));
	m_PS = CreatePsFromFile(pDevice, L"01-ModelViewer\\Wireframe.hlsl", "PS");

	// Rasterizer state
    D3D11_RASTERIZER_DESC rast;
    rast.AntialiasedLineEnable = TRUE;
    rast.FillMode = D3D11_FILL_WIREFRAME;
    rast.CullMode = D3D11_CULL_NONE;
    rast.DepthBias = 0;
    rast.DepthBiasClamp = 0;
    rast.DepthClipEnable = FALSE;
    rast.FrontCounterClockwise = FALSE;
    rast.MultisampleEnable = FALSE;
    rast.ScissorEnable = FALSE;
    rast.SlopeScaledDepthBias = 0;
    verify(pDevice->CreateRasterizerState(&rast, &m_pRastState));

	// Constant buffer
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.ByteWidth = sizeof(SPerFrameCb);
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerFrameCb));
}

void CWireframeTech::PrepareForDraw(ID3D11DeviceContext* pCtx, const float4x4& VpMat)
{
	pCtx->OMSetDepthStencilState(nullptr, 0);
	pCtx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	pCtx->RSSetState(m_pRastState);
	pCtx->VSSetShader(m_VS->pShader, nullptr, 0);
	pCtx->PSSetShader(m_PS->pShader, nullptr, 0);

	// Update CB
	m_VpMat = VpMat;

	ID3D11Buffer* pCb = m_PerFrameCb;
	pCtx->VSSetConstantBuffers(0, 1, &pCb);
}

void CWireframeTech::DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx)
{
	// Set per-mesh resources
    pMesh->SetDrawState(pCtx, m_VS->pCodeBlob);

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CWireframeTech::DrawModel(const CRtrModel* pModel, ID3D11DeviceContext* pCtx)
{
	for(const auto& DrawCmd : pModel->GetDrawList())
	{
		SPerFrameCb PerFrameData;
		PerFrameData.WvpMat = DrawCmd.Transformation * m_VpMat;
		UpdateEntireConstantBuffer(pCtx, m_PerFrameCb, PerFrameData);

		for(const auto& pMesh : DrawCmd.pMeshes)
		{
			DrawMesh(pMesh, pCtx);
		}
	}
}