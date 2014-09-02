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

Filename: TransparencyTech.cpp
---------------------------------------------------------------------------*/
#include "TransparencyTech.h"
#include "DxModel.h"

const std::wstring ShaderFileName = L"02-Transparency\\Transparency.hlsl";

CTransparencyTech::CTransparencyTech(ID3D11Device* pDevice)
{
	m_VS = CreateVsFromFile(pDevice, ShaderFileName, "VS");
	VerifyConstantLocation(m_VS->pReflector, "gWVP", 0, offsetof(SPerModelCb, Wvp));
	VerifyConstantLocation(m_VS->pReflector, "gWorld", 0, offsetof(SPerModelCb, World));

	m_SolidPS = CreatePsFromFile(pDevice, ShaderFileName, "SolidPS");
	VerifyConstantLocation(m_SolidPS->pReflector, "gLightDirW", 0, offsetof(SPerModelCb, LightDirW));
	VerifyConstantLocation(m_SolidPS->pReflector, "gLightIntensity", 0, offsetof(SPerModelCb, LightIntensity));
    VerifyConstantLocation(m_SolidPS->pReflector, "gMeshColor", 1, offsetof(SPerMeshCb, MeshColor));

	m_UnorderedBlendingPS = CreatePsFromFile(pDevice, ShaderFileName, "UnorderedBlendPS");
	VerifyConstantLocation(m_SolidPS->pReflector, "gAlphaOut", 0, offsetof(SPerModelCb, AlphaOut));

	// Constant buffer
	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.ByteWidth = sizeof(SPerModelCb);
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerModelCb));

	BufferDesc.ByteWidth = sizeof(SPerMeshCb);
	verify(pDevice->CreateBuffer(&BufferDesc, nullptr, &m_PerMeshCb));

	// Create depth test
	m_NoDepthTest = CreateNoDepthStencilTests(pDevice);
	m_NoCullRastState = CreateSolidNoCullRasterizerState(pDevice);
	m_SrcAlphaBlend = CreateSrcAlphaBlendState(pDevice);
}

void CTransparencyTech::PrepareForDraw(ID3D11DeviceContext* pContext, const SPerModelCb& CbData, TransparencyMode Mode)
{
	CMapBufferWriteDiscard MapCb(pContext, m_PerModelCb);
	*(SPerModelCb*)MapCb.MapInfo.pData = CbData;

	((SPerModelCb*)MapCb.MapInfo.pData)->LightDirW = -CbData.LightDirW;

	pContext->VSSetShader(m_VS->pShader, nullptr, 0);
	ID3D11Buffer* CBuffers[2] = { m_PerModelCb.GetInterfacePtr(), m_PerMeshCb.GetInterfacePtr() };
	pContext->VSSetConstantBuffers(0, 2, CBuffers);
	pContext->PSSetConstantBuffers(0, 2, CBuffers);

	switch(Mode)
	{
	case TransparencyMode::Solid:
		pContext->PSSetShader(m_SolidPS->pShader, nullptr, 0);
		pContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		pContext->OMSetDepthStencilState(nullptr, 0);
		pContext->RSSetState(nullptr);
		break;
	case TransparencyMode::UnorderedBlend:
		pContext->PSSetShader(m_UnorderedBlendingPS->pShader, nullptr, 0);
		pContext->OMSetBlendState(m_SrcAlphaBlend, nullptr, 0xFFFFFFFF);
		pContext->OMSetDepthStencilState(m_NoDepthTest, 0);
		pContext->RSSetState(m_NoCullRastState);
		break;
	}
}

void CTransparencyTech::SetPerMeshData(ID3D11DeviceContext* pContext, const SPerMeshCb& CbData)
{
	CMapBufferWriteDiscard MapCb(pContext, m_PerMeshCb);
	SPerMeshCb* pCb = (SPerMeshCb*)MapCb.MapInfo.pData;
	*pCb = CbData;
}

void CTransparencyTech::DrawMesh(ID3D11DeviceContext* pCtx, const CDxMesh* pMesh)
{
    pMesh->SetDrawState(pCtx, m_VS->pCodeBlob);

	SPerMeshCb MeshData;
    MeshData.MeshColor = pMesh->GetMaterial()->m_DiffuseColor;
	SetPerMeshData(pCtx, MeshData);

	UINT IndexCount = pMesh->GetIndexCount();
	pCtx->DrawIndexed(IndexCount, 0, 0);
}

void CTransparencyTech::DrawModel(const CDxModel* pModel, ID3D11DeviceContext* pCtx)
{
	for(UINT MeshID = 0; MeshID < pModel->GetMeshesCount(); ++MeshID)
	{
		const CDxMesh* pMesh = pModel->GetMesh(MeshID);
		DrawMesh(pCtx, pMesh);
	}
}