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

Filename: RtrMesh.h
---------------------------------------------------------------------------*/
#pragma once
#include "..\Common.h"
#include <map>

class CRtrModel;
class CRtrMaterial;
struct aiMesh;
class CRtrAnimationController;

class CRtrMesh
{
public:
	CRtrMesh(ID3D11Device* pDevice, const CRtrModel* pModel, const aiMesh* pAiMesh);

	enum
	{
		VERTEX_ELEMENT_POSITION,
		VERTEX_ELEMENT_NORMAL,
		VERTEX_ELEMENT_TANGENT,
		VERTEX_ELEMENT_BITANGENT,
		VERTEX_ELEMENT_BONE_IDS,
		VERTEX_ELEMENT_BONE_WEIGHTS,
		VERTEX_ELEMENT_DIFFUSE_COLOR,
		VERTEX_ELEMENT_TEXCOORD_0,

		VERTEX_ELEMENT_COUNT
	};

	void SetDrawState(ID3D11DeviceContext* pCtx, ID3DBlob* pVsBlob) const;

	const RTR_BOX_F& GetBoundingBox() const { return m_BoundingBox; }
	UINT GetVertexCount() const { return m_VertexCount; }
	UINT GetPrimiveCount() const { return m_PrimitiveCount; }
	UINT GetIndexCount() const { return m_IndexCount; }
	const CRtrMaterial* GetMaterial() const { return m_pMaterial; }

	bool HasBones() const { return m_bHasBones; }
private:
	UINT m_IndexCount		= 0;
	DXGI_FORMAT m_IndexType = DXGI_FORMAT_UNKNOWN;
	UINT m_VertexCount		= 0;
	UINT m_PrimitiveCount	= 0;
	UINT m_VertexStride		= 0;
	bool m_bHasBones		= false;
	UINT m_VertexElementsOffsets[VERTEX_ELEMENT_COUNT];
	const CRtrMaterial* m_pMaterial = nullptr;
	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	RTR_BOX_F m_BoundingBox;

	ID3D11BufferPtr m_IB;
	ID3D11BufferPtr m_VB;

	void SetVertexElementOffsets(const aiMesh* pAiMesh);
	void CreateIndexBuffer(ID3D11Device* pDevice, const aiMesh* pAiMesh);
	template<typename IndexType>
	void CreateIndexBufferInternal(ID3D11Device* pDevice, const aiMesh* pAiMesh);
    void CreateVertexBuffer(ID3D11Device* pDevice, const aiMesh* pAiMesh, const CRtrAnimationController* pAnimationController);
    void LoadBones(const aiMesh* pAiMesh, BYTE* pVertexData, const CRtrAnimationController* pAnimationController);

	ID3D11InputLayout* GetInputLayout(ID3D11DeviceContext* pCtx, ID3DBlob* pVsBlob) const;

	static const UINT m_MaxBonesPerVertex;
	static const UINT m_InvalidVertexOffset;

	mutable std::map<ID3DBlob*, ID3D11InputLayoutPtr> m_InputLayouts;
};