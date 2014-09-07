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

Filename: RtrMesh.cpp
---------------------------------------------------------------------------*/
#include "RtrMesh.h"
#include "..\RtrModel.h"
#include "mesh.h"

const UINT CRtrMesh::m_MaxBonesPerVertex = 8;
const UINT CRtrMesh::m_InvalidVertexOffset = UINT(-1);

CRtrMesh::CRtrMesh(ID3D11Device* pDevice, const CRtrModel* pModel, const aiMesh* pAiMesh)
{
	m_VertexCount = pAiMesh->mNumVertices;
	m_PrimitiveCount = m_VertexCount / pAiMesh->mFaces[0].mNumIndices;
	CreateIndexBuffer(pDevice, pAiMesh);
	CreateVertexBuffer(pDevice, pAiMesh, pModel->GetBones());
	switch(pAiMesh->mFaces[0].mNumIndices)
	{
	case 1:
		m_Topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case 2:
		m_Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case 3:
		m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	default:
		assert(0);
	}

	m_pMaterial = pModel->GetMaterial(pAiMesh->mMaterialIndex);
	assert(m_pMaterial);
}

void CRtrMesh::SetVertexElementOffsets(const aiMesh* pAiMesh)
{
	for(int i = 0; i < VERTEX_ELEMENT_COUNT; i++)
	{
		m_VertexElementsOffsets[i] = m_InvalidVertexOffset;
	}

	UINT Offset = 0;
	// Must have position!!!
	if(pAiMesh->HasPositions() == false)
	{
		trace(L"Loaded mesh with no positions!");
		return;
	}
	m_VertexElementsOffsets[VERTEX_ELEMENT_POSITION] = Offset;
	Offset += sizeof(float3);

	if(pAiMesh->HasNormals())
	{
		m_VertexElementsOffsets[VERTEX_ELEMENT_NORMAL] = Offset;
		Offset += sizeof(float3);
	}

	if(pAiMesh->HasTangentsAndBitangents())
	{
		m_VertexElementsOffsets[VERTEX_ELEMENT_TANGENT] = Offset;
		Offset += sizeof(float3);
		m_VertexElementsOffsets[VERTEX_ELEMENT_BITANGENT] = Offset;
		Offset += sizeof(float3);
	}

	// Supporting only tex coord0
	if(pAiMesh->HasTextureCoords(0))
	{
		m_VertexElementsOffsets[VERTEX_ELEMENT_TEXCOORD_0] = Offset;
		Offset += sizeof(float3);
	}

	for(int i = 1; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
	{
		if(pAiMesh->HasTextureCoords(i))
		{
			trace(L"RtrModel only support texcoord 0");
			return;
		}
	}

	if(pAiMesh->HasVertexColors(0))
	{
		m_VertexElementsOffsets[VERTEX_ELEMENT_DIFFUSE_COLOR] = Offset;
		Offset += sizeof(DWORD); // To save space we will store it as RGBA8_UNORM
	}

	if(pAiMesh->HasBones())
	{
		m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_IDS] = Offset;
		Offset += sizeof(UINT8)*m_MaxBonesPerVertex;
		m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_WEIGHTS] = Offset;
		Offset += sizeof(float)*m_MaxBonesPerVertex;
	}

	m_VertexStride = Offset;
}

template<typename IndexType>
void CRtrMesh::CreateIndexBufferInternal(ID3D11Device* pDevice, const aiMesh* pAiMesh)
{
	std::unique_ptr<IndexType[]> Indices = std::unique_ptr<IndexType[]>(new IndexType[m_IndexCount]);

	if(Indices.get() == nullptr)
	{
		trace(L"Can't allocate space for indices");
		return;
	}

	const UINT FirstFaceIndexCount = pAiMesh->mFaces[0].mNumIndices;

	for(UINT i = 0; i < pAiMesh->mNumFaces; i++)
	{
		UINT IndexCount = pAiMesh->mFaces[i].mNumIndices;
		assert(IndexCount == FirstFaceIndexCount); // Mesh contains mixed primitive types, can be solved using aiProcess_SortByPType
		for(UINT j = 0; j < FirstFaceIndexCount; j++)
		{
			Indices[i * IndexCount + j] = (IndexType)(pAiMesh->mFaces[i].mIndices[j]);

		}
	}

	D3D11_BUFFER_DESC IbDesc;
	IbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IbDesc.ByteWidth = sizeof(IndexType)*m_IndexCount;
	IbDesc.CPUAccessFlags = 0;
	IbDesc.MiscFlags = 0;
	IbDesc.StructureByteStride = 0;
	IbDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = Indices.get();
	InitData.SysMemPitch = IbDesc.ByteWidth;
	InitData.SysMemSlicePitch = IbDesc.ByteWidth;

	verify(pDevice->CreateBuffer(&IbDesc, &InitData, &m_IB));	
}

void CRtrMesh::CreateIndexBuffer(ID3D11Device* pDevice, const aiMesh* pAiMesh)
{
	// Assuming everything is triangles. I've never seen lines/points used directly in models
	m_IndexCount = pAiMesh->mNumFaces * pAiMesh->mFaces[0].mNumIndices;

	// Save some space by choosing the best index buffer type (16/32 bit)
	if(m_IndexCount < D3D11_16BIT_INDEX_STRIP_CUT_VALUE)
	{
		m_IndexType = DXGI_FORMAT_R16_UINT;
		return CreateIndexBufferInternal<UINT16>(pDevice, pAiMesh);
	}
	else
	{
		m_IndexType = DXGI_FORMAT_R32_UINT;
		return CreateIndexBufferInternal<UINT32>(pDevice, pAiMesh);
	}
}

#define MESH_LOAD_INPUT(_vertex_index, _element, _field)                                            \
    Offset = m_VertexElementsOffsets[_element];                                                     \
if(Offset != m_InvalidVertexOffset)                                                                 \
{                                                                                                   \
    BYTE* pDst = pVertex + Offset;                                                                  \
    BYTE* pSrc = (BYTE*)&(pAiMesh->_field[_vertex_index]);                                          \
    memcpy(pDst, pSrc, sizeof(pAiMesh->_field[0]));                                                 \
}

void CRtrMesh::CreateVertexBuffer(ID3D11Device* pDevice, const aiMesh* pAiMesh, const CRtrBones* pBones)
{
	SetVertexElementOffsets(pAiMesh);
	auto InitData = std::unique_ptr<BYTE[]>(new BYTE[m_VertexStride * m_VertexCount]);
	ZeroMemory(InitData.get(), m_VertexStride * m_VertexCount);

	for(UINT i = 0; i < m_VertexCount; i++)
	{
		BYTE* pVertex = InitData.get() + (m_VertexStride * i);
		UINT Offset;

		MESH_LOAD_INPUT(i, VERTEX_ELEMENT_POSITION, mVertices);
		MESH_LOAD_INPUT(i, VERTEX_ELEMENT_NORMAL, mNormals);
		MESH_LOAD_INPUT(i, VERTEX_ELEMENT_TANGENT, mTangents);
		MESH_LOAD_INPUT(i, VERTEX_ELEMENT_BITANGENT, mBitangents);

		for(UINT j = 0; j < pAiMesh->GetNumUVChannels(); j++)
		{
			MESH_LOAD_INPUT(i, VERTEX_ELEMENT_TEXCOORD_0 + j, mTextureCoords[j]);
		}

		float3 xyz(pAiMesh->mVertices[i].x, pAiMesh->mVertices[i].y, pAiMesh->mVertices[i].z);
		m_BoundingBox.Min = float3::Min(m_BoundingBox.Min, xyz);
		m_BoundingBox.Max = float3::Max(m_BoundingBox.Max, xyz);

		// Colors require special handling since we need to normalize them
		Offset = m_VertexElementsOffsets[VERTEX_ELEMENT_DIFFUSE_COLOR];
		if(Offset != m_InvalidVertexOffset)
		{
			BYTE* pColor = pVertex + Offset;
			float f = pAiMesh->mColors[0][i].r;
			pColor[0] = (BYTE)(255 * min(max(f, 0), 1));
			f = pAiMesh->mColors[0][i].g;
			pColor[1] = (BYTE)(255 * min(max(f, 0), 1));
			f = pAiMesh->mColors[0][i].b;
			pColor[2] = (BYTE)(255 * min(max(f, 0), 1));
			f = pAiMesh->mColors[0][i].a;
			pColor[3] = (BYTE)(255 * min(max(f, 0), 1));
		}
	}

	if(pAiMesh->HasBones())
	{
		m_bHasBones = true;
		LoadBones(pAiMesh, InitData.get(), pBones);
	}

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.ByteWidth = m_VertexStride * m_VertexCount;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = m_VertexStride;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA VbData;
	VbData.pSysMem = InitData.get();
	VbData.SysMemPitch = vbDesc.ByteWidth;
	VbData.SysMemSlicePitch = vbDesc.ByteWidth;

	verify(pDevice->CreateBuffer(&vbDesc, &VbData, &m_VB));
}

void CRtrMesh::SetDrawState(ID3D11DeviceContext* pCtx, ID3DBlob* pVsBlob) const
{
	pCtx->IASetIndexBuffer(m_IB, m_IndexType, 0);
	pCtx->IASetInputLayout(GetInputLayout(pCtx, pVsBlob));
	UINT z = 0;
	UINT stride = m_VertexStride;
	ID3D11Buffer* pBuf = m_VB;
	pCtx->IASetVertexBuffers(0, 1, &pBuf, &stride, &z);
	pCtx->IASetPrimitiveTopology(m_Topology);
}

ID3D11InputLayout* CRtrMesh::GetInputLayout(ID3D11DeviceContext* pCtx, ID3DBlob* pVsBlob) const
{
	if(m_InputLayouts.find(pVsBlob) == m_InputLayouts.end())
	{
		ID3D11DevicePtr pDevice;
		pCtx->GetDevice(&pDevice);

		UINT BonesIDOffset = HasBones() ? sizeof(UINT8) * 4 : 0;
		UINT BonesWeightOffset = HasBones() ? sizeof(float) * 4 : 0;

		D3D11_INPUT_ELEMENT_DESC desc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_POSITION], D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_NORMAL], D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_TANGENT], D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_BITANGENT], D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_WEIGHTS], D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONE_WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_WEIGHTS] + BonesWeightOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONE_IDS", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_IDS], D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONE_IDS", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_IDS] + BonesIDOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, m_VertexElementsOffsets[VERTEX_ELEMENT_TEXCOORD_0], D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ID3D11InputLayout* pLayout;
		verify(pDevice->CreateInputLayout(desc, ARRAYSIZE(desc), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &pLayout));
		m_InputLayouts[pVsBlob] = pLayout;
	}

	return m_InputLayouts[pVsBlob].GetInterfacePtr();
}

void CRtrMesh::LoadBones(const aiMesh* pAiMesh, BYTE* pVertexData, const CRtrBones* pRtrBones)
{
	if(pAiMesh->mNumBones > 0xff)
	{
		trace(L"Too many bones");
	}

	for(UINT Bone = 0; Bone < pAiMesh->mNumBones; Bone++)
	{
		const aiBone* pAiBone = pAiMesh->mBones[Bone];
		UINT AiBoneID = pRtrBones->GetIdFromName(pAiBone->mName.C_Str());

		// The way Assimp works, the weights holds the IDs of the vertices it affects.
		// We loop over all the weights, initializing the vertices data along the way
		for(UINT WeightID = 0; WeightID < pAiBone->mNumWeights; WeightID++)
		{
			// Get the vertex the current weight affects
			const aiVertexWeight& AiWeight = pAiBone->mWeights[WeightID];
			BYTE* pVertex = pVertexData + (AiWeight.mVertexId * m_VertexStride);

			// Get the address of the Bone ID and weight for the current vertex
			BYTE* pBoneIDs = (BYTE*)(pVertex + m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_IDS]);
			float* pVertexWeights = (float*)(pVertex + m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_WEIGHTS]);

			// Find the next unused slot in the bone array of the vertex, and initialize it with the current value
			bool bFoundEmptySlot = false;
			for(UINT j = 0; j < m_MaxBonesPerVertex; j++)
			{
				if(pVertexWeights[j] == 0)
				{
					pBoneIDs[j] = (BYTE)AiBoneID;
					pVertexWeights[j] = AiWeight.mWeight;
					bFoundEmptySlot = true;
					break;
				}
			}

			if(bFoundEmptySlot == false)
			{
				trace(L"Too many bones");
			}
		}
	}

	// Now we need to normalize the weights for each vertex, since in some models the sum is larger than 1
	for(UINT i = 0; i < m_VertexCount; i++)
	{
		BYTE* pVertex = pVertexData + (i * m_VertexStride);
		float* pVertexWeights = (float*)(pVertex + m_VertexElementsOffsets[VERTEX_ELEMENT_BONE_WEIGHTS]);

		float f = 0;
		// Sum the weights
		for(int j = 0; j < m_MaxBonesPerVertex; j++)
		{
			f += pVertexWeights[j];
		}
		// Normalize the weights
		for(int j = 0; j < m_MaxBonesPerVertex; j++)
		{
			pVertexWeights[j] /= f;
		}
	}
}
