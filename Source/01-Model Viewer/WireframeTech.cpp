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

CWireframeTech::CWireframeTech(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
	m_VS = CreateVsFromFile(pDevice, L"Wireframe.hlsl", "VS");
	VerifyConstantLocation(m_VS->pReflector, "gWVPMat", 0, offsetof(SPerFrameCb, WvpMat));
	m_PS = CreatePsFromFile(pDevice, L"Wireframe.hlsl", "PS");

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

void CWireframeTech::PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameCb& PerFrameData)
{
	pCtx->OMSetDepthStencilState(nullptr, 0);
	pCtx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	pCtx->RSSetState(m_pRastState);
	pCtx->VSSetShader(m_VS->pShader, nullptr, 0);
	pCtx->PSSetShader(m_PS->pShader, nullptr, 0);

	// Update CB
	D3D11_MAPPED_SUBRESOURCE Data;
	verify(pCtx->Map(m_PerFrameCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data));
	SPerFrameCb* pCB = (SPerFrameCb*)Data.pData;
    *pCB = PerFrameData;
	pCtx->Unmap(m_PerFrameCb, 0);

	ID3D11Buffer* pCb = m_PerFrameCb;
	pCtx->VSSetConstantBuffers(0, 1, &pCb);
}

void CWireframeTech::DrawMesh(const CDxMesh* pMesh, ID3D11DeviceContext* pCtx)
{
	// Set per-mesh resources
	ID3D11InputLayout* pLayout = pMesh->GetInputLayout(pCtx, m_VS->pCodeBlob);
	pCtx->IASetInputLayout(pLayout);
	pCtx->IASetPrimitiveTopology(pMesh->GetPrimitiveTopology());

	pCtx->IASetIndexBuffer(pMesh->GetIndexBuffer(), pMesh->GetIndexBufferFormat(), 0);
	ID3D11Buffer* pVB = pMesh->GetVertexBuffer();
	UINT Stride = pMesh->GetVertexStride();
	UINT Offset = 0;
	pCtx->IASetVertexBuffers(0, 1, &pVB, &Stride, &Offset);


	ID3D11ShaderResourceView* rt[] = { pMesh->GetMaterial()->m_SRV[MESH_TEXTURE_DIFFUSE].GetInterfacePtr(),
		pMesh->GetMaterial()->m_SRV[MESH_TEXTURE_NORMALS].GetInterfacePtr() };

	pCtx->PSSetShaderResources(0, 2, rt);

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CWireframeTech::DrawModel(const CDxModel* pModel, ID3D11DeviceContext* pCtx)
{
	for(UINT MeshID = 0; MeshID < pModel->GetMeshesCount(); MeshID++)
	{
		const CDxMesh* pMesh = pModel->GetMesh(MeshID);
		DrawMesh(pMesh, pCtx);
	}
}