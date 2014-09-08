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

Filename: SkeletonRenderer.cpp
---------------------------------------------------------------------------
*/
#include "SkeletonRenderer.h"
#include <vector>
#include "RtrModel.h"

struct SSkeletonRendererCB
{
    float4x4 WVPMat;
    float4x4 LocalToWorld[256];
};

struct SVertex
{
    BYTE BoneID;
};

CSkeletonRenderer::CSkeletonRenderer(ID3D11Device* pDevice, const CRtrModel* pModel) : m_pModel(pModel)
{
// 	assert(pModel);
// 	assert(pModel->HasBones());
// 
//     m_VS = CreateVsFromFile(pDevice, L"01-ModelViewer\\SkeletonRenderer.hlsl", "VSMain");
//     m_PS = CreatePsFromFile(pDevice, L"01-ModelViewer\\SkeletonRenderer.hlsl", "PSMain");
// 
//     // Create the CB
//     D3D11_BUFFER_DESC Desc;
//     Desc.Usage = D3D11_USAGE_DYNAMIC;
//     Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//     Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//     Desc.MiscFlags = 0;
//     Desc.ByteWidth = sizeof(SSkeletonRendererCB);
//     verify(pDevice->CreateBuffer(&Desc, NULL, &m_pCB));
// 
//     // create the depth stencil state
//     D3D11_DEPTH_STENCIL_DESC dsDesc = { 0 };
//     dsDesc.DepthEnable = FALSE;
//     dsDesc.StencilEnable = FALSE;
// 
//     verify(pDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState));
// 
//     CreateVertexBuffer(pDevice);
//     CreateIndexBuffer(pDevice);
// 
//     // Create the input layout
//     // Create the input layout
//     D3D11_INPUT_ELEMENT_DESC desc[] =
//     {
//         { "BONEID", 0, DXGI_FORMAT_R8_UINT, 0, offsetof(SVertex, BoneID), D3D11_INPUT_PER_VERTEX_DATA, 0 },
//     };
// 
//     verify(pDevice->CreateInputLayout(desc, ARRAYSIZE(desc), m_VS->pCodeBlob->GetBufferPointer(), m_VS->pCodeBlob->GetBufferSize(), &m_InputLayout));
}

void CSkeletonRenderer::CreateVertexBuffer(ID3D11Device* pDevice)
{
//     std::vector<SVertex> Vertices;
// 	const CRtrBones* pBones = m_pModel->GetBones();
// 	Vertices.resize(pBones->GetCount());
// 
// 	for(UINT i = 0; i < pBones->GetCount(); i++)
//     {
// 		Vertices[i].BoneID = (BYTE)pBones->GetBoneDesc(i).BoneID;
//     }
// 
//     D3D11_SUBRESOURCE_DATA initData;
//     initData.pSysMem = &Vertices[0];
// 	initData.SysMemPitch = sizeof(SVertex)*pBones->GetCount();
// 
//     // Create the vb
//     D3D11_BUFFER_DESC Desc;
//     Desc.Usage = D3D11_USAGE_DEFAULT;
//     Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//     Desc.CPUAccessFlags = 0;
//     Desc.MiscFlags = 0;
//     Desc.ByteWidth = initData.SysMemPitch;
//     verify(pDevice->CreateBuffer(&Desc, &initData, &m_pVB));
}


void CSkeletonRenderer::CreateIndexBuffer(ID3D11Device* pDevice)
{
//     // Create the inital data
//     std::vector<UINT16> indices;
// 	const CRtrBones* pBones = m_pModel->GetBones();
//     for(UINT i = 0; i < pBones->GetCount(); i++)
//     {
// 		const SBoneDesc& Bone = pBones->GetBoneDesc(i);
// 		if(Bone.ParentID != INVALID_BONE_ID)
//         {
// 			indices.push_back((UINT16)Bone.ParentID);
//             indices.push_back((UINT16)i);
//         }
//     }
// 
//     m_IndexCount = indices.size();
// 
//     D3D11_SUBRESOURCE_DATA initData;
//     initData.pSysMem = &indices[0];
//     initData.SysMemPitch = sizeof(UINT16)*indices.size();
// 
//     // Create the ib
//     D3D11_BUFFER_DESC Desc;
//     Desc.Usage = D3D11_USAGE_DEFAULT;
//     Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//     Desc.CPUAccessFlags = 0;
//     Desc.MiscFlags = 0;
//     Desc.ByteWidth = initData.SysMemPitch;
//     verify(pDevice->CreateBuffer(&Desc, &initData, &m_pIB));
}

void CSkeletonRenderer::UpdateConstantBuffer(ID3D11DeviceContext* pCtx, const float4x4& VpMat)
{
//     SSkeletonRendererCB CbData;
//     CbData.WVPMat = VpMat;
// 	const CRtrBones* pBones = m_pModel->GetBones();
// 	memcpy(CbData.LocalToWorld, pBones->GetBonesTransform(), sizeof(float4x4)*pBones->GetCount());
//     UpdateEntireConstantBuffer(pCtx, m_pCB, CbData);
// 
//     ID3D11Buffer* pCb = m_pCB.GetInterfacePtr();
//     pCtx->VSSetConstantBuffers(0, 1, &pCb);
}

void CSkeletonRenderer::Draw(ID3D11DeviceContext* pCtx, const float4x4& VpMat)
{
//     // Update the CB
//     UpdateConstantBuffer(pCtx, VpMat);
// 
//     pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//     // Set the shaders
//     pCtx->OMSetDepthStencilState(m_pDepthStencilState, 0);
//     pCtx->VSSetShader(m_VS->pShader, nullptr, 0);
//     pCtx->PSSetShader(m_PS->pShader, nullptr, 0);
//     pCtx->IASetInputLayout(m_InputLayout);
//     UINT stride = sizeof(SVertex);
//     UINT offset = 0;
// 	ID3D11Buffer* pVB = m_pVB.GetInterfacePtr();
//     pCtx->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
//     pCtx->IASetIndexBuffer(m_pIB, DXGI_FORMAT_R16_UINT, 0);
// 
//     pCtx->DrawIndexed(m_IndexCount, 0, 0);
//     pCtx->OMSetDepthStencilState(NULL, 0);
}
