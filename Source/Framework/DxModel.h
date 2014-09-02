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

Filename - DxModel.h
---------------------------------------------------------------------------
*/
#pragma  once
#include "Common.h"
#include <string>
#include <map>
#include <vector>

class CLightingPass;

typedef std::map<std::string, int> string_int_map;
// Forward declarations
class CDxMesh;
class CDxModel;
struct SMaterial;
struct aiScene;
struct aiNode;
struct aiMesh;

#define INVALID_ELEMENT_OFFSET ((UINT)-1)
#define INVALID_BONE_ID ((UINT)-1)
#define MAX_BONES_PER_MODEL (255)
#define MESH_MAX_BONE_PER_VERTEX 8

class CDxMesh;
typedef std::vector<CDxMesh*> dx_mesh_vector;

enum MESH_ELEMENT_TYPE
{
    MESH_ELEMENT_POSITION,
    MESH_ELEMENT_NORMAL,
    MESH_ELEMENT_TANGENT,
    MESH_ELEMENT_BI_TANGENT,

    MESH_ELEMENT_BONE_INDICES,
    MESH_ELEMENT_BONE_WEIGHTS,
    MESH_ELEMENT_DIFFUSE,

    MESH_ELEMENT_TEXCOORD0,

    // Must be last
    MESH_NUM_ELEMENT_TYPES
};

enum MESH_TEXTURE_TYPE
{
    MESH_TEXTURE_DIFFUSE,
    MESH_TEXTURE_NORMALS,
    MESH_TEXTURE_SPEC,
    MESH_TEXTURE_HEIGHT,
    MESH_TEXTURE_ALPHA,
    // Must be last
    MESH_NUM_TEXTURE_TYPES
};

struct SBoundingBox
{
    float3 Min;
    float3 Max;

    SBoundingBox() : Min(FLT_MAX, FLT_MAX, FLT_MAX), Max(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}
};

struct SMaterial
{
    SMaterial() { ZeroMemory(m_SRV, sizeof(m_SRV)); }

    ID3D11ShaderResourceViewPtr m_SRV[MESH_NUM_TEXTURE_TYPES];
    float3 m_DiffuseColor;
    std::string m_MaterialName;
};

struct SBone
{
    SBone() { ZeroMemory(this, sizeof(this)); }
    std::string Name;
    UINT BoneID;
    std::vector<UINT> ChildIDs;
    UINT ParentID;
    float4x4 Matrix;
    float4x4 InvMatrix;
};

class CDxMesh
{
    friend class CDxModel;
 
public:
    static CDxMesh* CreateMesh(const aiMesh* pMesh, const CDxModel* pModel, ID3D11Device* pDevice, string_int_map& BonesMap);
	~CDxMesh();

    UINT GetIndexCount() const { return m_IndexCount; }

    UINT GetMaterialIndex() const { return m_MaterialIndex; }
    const SMaterial* GetMaterial() const { return m_pMaterial; }
    void SetMaterial(const SMaterial* pMaterial) { m_pMaterial = pMaterial; }

    const SBoundingBox& GetBoundingBox() { return m_BoundingBox; }
    void SetDrawState(ID3D11DeviceContext* pContext, ID3DBlob* pVsBlob) const;

    const CDxModel* GetModel() const { return m_pModel; }
    bool HasBones() const { return (GetVertexElementOffset(MESH_ELEMENT_BONE_INDICES) != INVALID_ELEMENT_OFFSET); }
private:
    CDxMesh(const std::string& Name, const CDxModel* pModel);
    HRESULT CreateVertexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice, string_int_map& BonesMap);
    HRESULT CreateElementLayout(ID3D11Device* pDevice, ID3DBlob* pVsBlob) const;
    HRESULT CreateIndexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice);
    HRESULT SetVertexElementOffsets(const aiMesh* pMesh);
    UINT GetVertexElementOffset(MESH_ELEMENT_TYPE e) const { return m_VertexElementOffsets[e]; }

    template<typename IndexType>
    HRESULT CreateIndexBufferInternal(const aiMesh* pMesh, ID3D11Device* pDevice);

    void LoadBones(const aiMesh* pMesh, BYTE* pVertexData, string_int_map& BonesMap);

    ID3D11Buffer* GetIndexBuffer() const { return m_IB.GetInterfacePtr(); }
	ID3D11Buffer* GetVertexBuffer() const { return m_VB.GetInterfacePtr(); }
    ID3D11InputLayout* GetInputLayout(ID3D11DeviceContext* pCtx, ID3DBlob* pVsBlob) const;
    DXGI_FORMAT GetIndexBufferFormat() const { return m_IndexType; }
	D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return m_PrimitiveTopology; }

    UINT GetVertexStride() const { return m_VertexStride; }
    UINT GetVertexCount() const { return m_VertexCount; }

    UINT m_VertexCount;
    UINT m_IndexCount;
    DXGI_FORMAT m_IndexType;
    UINT m_VertexStride;
    D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;

    ID3D11BufferPtr m_VB;
    ID3D11BufferPtr m_IB;
    UINT m_MaterialIndex;
    UINT m_VertexElementOffsets[MESH_NUM_ELEMENT_TYPES];
    const CDxModel* m_pModel;

    SBoundingBox m_BoundingBox;
    const SMaterial* m_pMaterial;

    mutable std::map<ID3DBlob*, ID3D11InputLayout*> m_InputLayouts;

    std::string m_Name;
};

// Simple mesh loader. Used to load single mesh scenes, so make sure you know what you are doing
class CDxModel
{
public:
	CDxModel(const CDxModel&) = delete;
	CDxModel& operator=(const CDxModel&) = delete;
    ~CDxModel();
    static CDxModel* LoadModelFromFile(const std::wstring& Filename, ID3D11Device* pDevice);
    float GetRadius() const { return m_Radius; }
    const SBoundingBox& GetBoundingBox() const { return m_BoundingBox; }
    const float3& GetCenter() const { return m_Center; }
    bool HasTextures() const { return m_bHasTextures; }
    UINT GetMeshesCount() const { return (UINT)m_pMeshes.size(); }
    const CDxMesh* GetMesh(UINT MeshID) const { return m_pMeshes[MeshID]; }

    UINT GetBonesCount() const { return m_BonesCount; }
    const SBone* GetBones() const { return m_Bones; }
    const float4x4& GetWorldMatrix() const { return m_WorldMatrix; }
    const dx_mesh_vector& GetAllMeshes() const;
    const dx_mesh_vector& GetMeshesByMaterialName(const std::string& sGroupName) const;

    void Animate(const float4x4& WorldMatrix);
    void GetBonesMatrices(float4x4* pMatrices, UINT MatrixCount, bool bForSkeletonRendering) const;

    UINT GetVertexCount() const {return m_VertexCount;}
    UINT GetPrimitiveCount() const { return m_PrimitiveCount; }

private:
    CDxModel();
	HRESULT CreateMaterials(ID3D11Device* pDevice, const aiScene* pScene, const std::string& Folder);
    void BuildBonesHierarchy(const aiScene* pScene, string_int_map& BoneMap);
    UINT InitBone(const aiNode* pCurNode, UINT ParentID, UINT BoneID, string_int_map& BoneMap);
    SBone* m_Bones;
    UINT m_BonesCount;
    float m_Radius;
    float3 m_Center;
	float4x4 m_WorldMatrix;
    SBoundingBox m_BoundingBox;

    UINT m_VertexCount;
    UINT m_PrimitiveCount;
    dx_mesh_vector m_pMeshes;
    std::vector<SMaterial*> m_Materials;
    std::map<std::string, dx_mesh_vector> m_pMeshGroups;
    bool m_bHasTextures;
};