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

Filename - DxModel.cpp
---------------------------------------------------------------------------
*/
#include "DxModel.h"
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"
#include "mesh.h"
#include "types.h"
#include "material.h"
#include "StringUtils.h"

#include <fstream>
#include <intsafe.h>

C_ASSERT(AI_MAX_NUMBER_OF_TEXTURECOORDS == MESH_MAX_BONE_PER_VERTEX);
C_ASSERT(MAX_BONES_PER_MODEL <= BYTE_MAX);

using namespace Assimp;

void DumpBonesHeirarchy(const std::string& filename, SBone* pBone, UINT count)
{
    std::ofstream dotfile;
    dotfile.open(filename.c_str(), 'w');

    // Header
    dotfile << "digraph BonesGraph {" << std::endl;

    for(UINT i = 0; i < count; i++)
    {
        const SBone& bone = pBone[i];
        if(bone.ParentID != INVALID_BONE_ID)
        {
            dotfile << pBone[bone.ParentID].Name << " -> " << bone.Name << std::endl;
        }
    }

    // Close the file
    dotfile << "}" << std::endl; // closing graph scope
    dotfile.close();
}

bool VerifyScene(const aiScene* pScene)
{
    bool b = true;

    // No internal textures
    if(pScene->mTextures != 0)
    {
        trace(L"Model has internal textures");
        b = false;
    }

    // Make sure all the meshes has the exact same input layout
    bool bHasPosition = pScene->mMeshes[0]->HasPositions();
    bool bHasNormals = pScene->mMeshes[0]->HasNormals();
    bool bHasTangent = pScene->mMeshes[0]->HasTangentsAndBitangents();
    bool bHasColors = pScene->mMeshes[0]->HasVertexColors(0);
    bool bHasBones = pScene->mMeshes[0]->HasBones();

    bool bHasTexcoord[AI_MAX_NUMBER_OF_TEXTURECOORDS];

    for(int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
    {
        bHasTexcoord[i] = pScene->mMeshes[0]->HasTextureCoords(i);
    }


    for(UINT i = 1; i < pScene->mNumMeshes; i++)
    {
        if(pScene->mMeshes[i]->HasPositions() != bHasPosition)
        {
            trace(L"VerifyScene() - position declaration doesn't match");
            return false;
        }

        if(pScene->mMeshes[i]->HasNormals() != bHasNormals)
        {
            trace(L"VerifyScene() - normal declaration doesn't match");
            return false;
        }

        if(pScene->mMeshes[i]->HasTangentsAndBitangents() != bHasTangent)
        {
            trace(L"VerifyScene() - tangent declaration doesn't match");
            return false;
        }

        if(pScene->mMeshes[i]->HasVertexColors(0) != bHasColors)
        {
            trace(L"VerifyScene() - color declaration doesn't match");
            return false;
        }

        if(pScene->mMeshes[i]->HasBones() != bHasBones)
        {
            trace(L"VerifyScene() - bones declaration doesn't match");
            return false;
        }

        for(int j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
        {
            if(bHasTexcoord[j] != pScene->mMeshes[i]->HasTextureCoords(j))
            {
                trace(L"VerifyScene() - texccord declaration doesn't match");
                return false;
            }
        }
    }

    return b;
}

CDxModel::CDxModel()
{
    m_bHasTextures = false;
    m_Bones = nullptr;
    m_BonesCount = 0;
}

CDxModel::~CDxModel()
{
    for(UINT i = 0; i < m_Materials.size(); i++)
    {
        SAFE_DELETE(m_Materials[i]);
    }

    for(UINT i = 0; i < m_pMeshes.size(); i++)
    {
        SAFE_DELETE(m_pMeshes[i]);
    }

    SAFE_DELETE_ARRAY(m_Bones);
}

float4x4 aiMatToD3D(const aiMatrix4x4& aiMat)
{
	float4x4 d3dMat;
    d3dMat.m[0][0] = aiMat.a1; d3dMat.m[0][1] = aiMat.a2; d3dMat.m[0][2] = aiMat.a3; d3dMat.m[0][3] = aiMat.a4;
    d3dMat.m[1][0] = aiMat.b1; d3dMat.m[1][1] = aiMat.b2; d3dMat.m[1][2] = aiMat.b3; d3dMat.m[1][3] = aiMat.b4;
    d3dMat.m[2][0] = aiMat.c1; d3dMat.m[2][1] = aiMat.c2; d3dMat.m[2][2] = aiMat.c3; d3dMat.m[2][3] = aiMat.c4;
    d3dMat.m[3][0] = aiMat.d1; d3dMat.m[3][1] = aiMat.d2; d3dMat.m[3][2] = aiMat.d3; d3dMat.m[3][3] = aiMat.d4;
    return d3dMat;
}

UINT CDxModel::InitBone(const aiNode* pCurNode, UINT ParentID, UINT BoneID, string_int_map& BoneMap)
{
    assert(BoneMap.find(pCurNode->mName.C_Str()) != BoneMap.end());
    BoneMap[pCurNode->mName.C_Str()] = BoneID;

    assert(BoneID < m_BonesCount);
    SBone& Bone = m_Bones[BoneID];
    Bone.Name = pCurNode->mName.C_Str();
	float4x4 Matrix = aiMatToD3D(pCurNode->mTransformation);
	Matrix.Transpose(Bone.Matrix);
	Bone.Matrix.Invert(Bone.InvMatrix);

    Bone.ParentID = ParentID;
    Bone.BoneID = BoneID;
    Bone.ChildIDs.clear();

    BoneID++;

    for(UINT i = 0; i < pCurNode->mNumChildren; i++)
    {
        // Check that the child is actually used
        if(BoneMap.find(pCurNode->mChildren[i]->mName.C_Str()) != BoneMap.end())
        {
            Bone.ChildIDs.push_back(BoneID);
            BoneID = InitBone(pCurNode->mChildren[i], Bone.BoneID, BoneID, BoneMap);
        }
    }

    return BoneID;
}

bool VerifySceneNames(const aiNode* pNode, string_int_map& Names)
{
    // Check that the current node is not already found
    if(Names.find(pNode->mName.C_Str()) != Names.end())
    {
        return false;
    }

    Names[pNode->mName.C_Str()] = INVALID_BONE_ID;
    // Now check the children
    bool b = true;
    for(UINT i = 0; i < pNode->mNumChildren; i++)
    {
        b &= VerifySceneNames(pNode->mChildren[i], Names);
    }
    return b;
}

void CDxModel::BuildBonesHierarchy(const aiScene* pScene, string_int_map& BoneMap)
{
    // First, go over the meshes and file all the bones that are used
    for(UINT i = 0; i < pScene->mNumMeshes; i++)
    {
        const aiMesh* pMesh = pScene->mMeshes[i];
        if(pMesh->HasBones())
        {
            for(UINT j = 0; j < pMesh->mNumBones; j++)
            {
                BoneMap[pMesh->mBones[j]->mName.C_Str()] = INVALID_BONE_ID;
            }
        }
    }

    if(BoneMap.size() != 0)
    {
        // Make sure we have unique names in the scene. The bones algorithm relies on unique names
        string_int_map tempmap;
        assert(VerifySceneNames(pScene->mRootNode, tempmap));

        // For every bone used, all its ancestors are bones too. Mark them
        string_int_map::iterator it = BoneMap.begin();
        while(it != BoneMap.end())
        {
            aiNode* pCurNode = pScene->mRootNode->FindNode(it->first.c_str());
            while(pCurNode)
            {
                BoneMap[pCurNode->mName.C_Str()] = INVALID_BONE_ID;
                pCurNode = pCurNode->mParent;
            }
            it++;
        }

        // Now initialize the bones
        m_BonesCount = BoneMap.size();
        m_Bones = new SBone[m_BonesCount];
        UINT bonesCount = InitBone(pScene->mRootNode, INVALID_BONE_ID, 0, BoneMap);
        _Unreferenced_parameter_(bonesCount);
        assert(m_BonesCount == bonesCount);
        //        DumpBonesHeirarchy("bones.dot", m_Bones, m_BonesCount);
    }
}

void MarkUsedMeshes(const aiNode* pNode, std::vector<bool>& MeshesToLoad)
{
    for(UINT i = 0; i < pNode->mNumMeshes; i++)
    {
        MeshesToLoad[pNode->mMeshes[i]] = true;
    }

    for(UINT i = 0; i < pNode->mNumChildren; i++)
    {
        MarkUsedMeshes(pNode->mChildren[i], MeshesToLoad);
    }
}

CDxModel* CDxModel::LoadModelFromFile(const std::wstring& Filename, ID3D11Device* pDevice)
{
    Importer importer;
	std::wstring WideFullpath;
	HRESULT hr = FindFileInCommonDirs(Filename, WideFullpath);
	if(FAILED(hr))
	{
		trace(std::wstring(L"Can't find model file ") + Filename);
		return nullptr;
	}
    // aiProcess_ConvertToLeftHanded will make necessary adjusments so that the model is ready for D3D. Check the assimp documenation for more info.	
	std::string Fullpath = wstring_2_string(WideFullpath);
    const aiScene* pScene = importer.ReadFile(std::string(Fullpath),
                                              aiProcess_ConvertToLeftHanded |
                                              aiProcess_Triangulate |
                                              aiProcess_JoinIdenticalVertices |
                                              aiProcess_ImproveCacheLocality |
                                              aiProcess_RemoveRedundantMaterials |
                                              aiProcess_OptimizeMeshes |
                                              aiProcess_CalcTangentSpace |
                                              aiProcess_GenSmoothNormals |
                                              aiProcess_FixInfacingNormals |
                                              aiProcess_FindInvalidData |
                                              aiProcess_ValidateDataStructure |
                                              aiProcess_LimitBoneWeights);

    if(pScene == nullptr)
    {
        std::wstring str(L"Can't open mesh file ");
        str = str + std::wstring(Filename);
        trace(str.c_str());
        return nullptr;
    }

    // Verify we support this scene
    if(VerifyScene(pScene) == false)
    {
        return nullptr;
    }

    CDxModel* pModel = new CDxModel;
    if(pModel)
    {
        // Create the materials first
		auto last = Fullpath.find_last_of("/\\");
		std::string Folder = Fullpath.substr(0, last);
        if(pModel->CreateMaterials(pDevice, pScene, Folder) != S_OK)
        {
            SAFE_DELETE(pModel);
        }

        string_int_map BonesMap;

        // Initialize the bones map
//        pModel->BuildBonesHierarchy(pScene, BonesMap);

        std::vector<bool> MeshesToLoad;
        MeshesToLoad.resize(pScene->mNumMeshes);
        MarkUsedMeshes(pScene->mRootNode, MeshesToLoad);

        pModel->m_pMeshGroups[""] = dx_mesh_vector();

        for(UINT i = 0; i < pScene->mNumMeshes; i++)
        {
            if(MeshesToLoad[i])
            {
                CDxMesh* pMesh = CDxMesh::CreateMesh(pScene->mMeshes[i], pModel, pDevice, BonesMap);
                if(pMesh == nullptr)
                {
                    return nullptr;
                }
                const SMaterial* pMaterial = pModel->m_Materials[pMesh->GetMaterialIndex()];
                pMesh->SetMaterial(pMaterial);
                pModel->m_pMeshes.push_back(pMesh);
                pModel->m_pMeshGroups[pMaterial->m_MaterialName].push_back(pMesh);
            }
        }

        // Calculate the radius and the center, vertex and primitive count
        pModel->m_Vertices = pModel->m_Primitives = 0;

        for(UINT i = 0; i < pModel->m_pMeshes.size(); i++)
        {
            const SBoundingBox& MeshBox = pModel->m_pMeshes[i]->GetBoundingBox();

            pModel->m_BoundingBox.Min.x = min(pModel->m_BoundingBox.Min.x, MeshBox.Min.x);
            pModel->m_BoundingBox.Min.y = min(pModel->m_BoundingBox.Min.y, MeshBox.Min.y);
            pModel->m_BoundingBox.Min.z = min(pModel->m_BoundingBox.Min.z, MeshBox.Min.z);

            pModel->m_BoundingBox.Max.x = max(pModel->m_BoundingBox.Max.x, MeshBox.Max.x);
            pModel->m_BoundingBox.Max.y = max(pModel->m_BoundingBox.Max.y, MeshBox.Max.y);
            pModel->m_BoundingBox.Max.z = max(pModel->m_BoundingBox.Max.z, MeshBox.Max.z);
            pModel->m_Vertices += pModel->m_pMeshes[i]->GetVertexCount();
            pModel->m_Primitives += (pModel->m_pMeshes[i]->GetIndexCount()) / 3;
        }

        pModel->m_Center.x = ((pModel->m_BoundingBox.Max.x + pModel->m_BoundingBox.Min.x) / 2);
        pModel->m_Center.y = ((pModel->m_BoundingBox.Max.y + pModel->m_BoundingBox.Min.y) / 2);
        pModel->m_Center.z = ((pModel->m_BoundingBox.Max.z + pModel->m_BoundingBox.Min.z) / 2);

		float3 distMax = float3(pModel->m_BoundingBox.Max.x, pModel->m_BoundingBox.Max.y, pModel->m_BoundingBox.Max.z) - pModel->m_Center;
        pModel->m_Radius = distMax.Length();
    }
    else
    {
        trace(L"Failed to allocate memory for the model\n");
    }

    // If something fails, I don't deallocate anything, since I assume the process will exit anyway
    return pModel;
}

HRESULT CDxModel::CreateMaterials(ID3D11Device* pDevice, const aiScene* pScene, const std::string& Folder)
{
    HRESULT hr = S_OK;

    for(UINT i = 0; i < pScene->mNumMaterials; i++)
    {
        const aiMaterial* paiMaterial = pScene->mMaterials[i];
        SMaterial* pRtrMaterial = new SMaterial;
        if(pRtrMaterial == nullptr)
        {
            trace(L"Can't allocate memory for material");
            return E_FAIL;
        }

        UINT textureCount[MESH_NUM_TEXTURE_TYPES];
        textureCount[MESH_TEXTURE_DIFFUSE] = paiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
        textureCount[MESH_TEXTURE_NORMALS] = paiMaterial->GetTextureCount(aiTextureType_NORMALS);
        textureCount[MESH_TEXTURE_HEIGHT] = paiMaterial->GetTextureCount(aiTextureType_HEIGHT);
        textureCount[MESH_TEXTURE_ALPHA] = paiMaterial->GetTextureCount(aiTextureType_OPACITY);
        textureCount[MESH_TEXTURE_SPEC] = paiMaterial->GetTextureCount(aiTextureType_SPECULAR);

        for(int i = 0; i < MESH_NUM_TEXTURE_TYPES; ++i)
        {
            if(textureCount[i] >= 1)
            {
                if(textureCount[i] != 1)
                {
                    trace(L"More then 1 textures");
                    return E_FAIL;
                }

                // Get the texture name
                aiString path;
                bool bSrgb = false;
                switch(i)
                {
                case MESH_TEXTURE_DIFFUSE:
                    bSrgb = true;
                    paiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path);
                    break;
                case MESH_TEXTURE_NORMALS:
                    paiMaterial->GetTexture(aiTextureType_NORMALS, 0, &path);
                    break;
                case MESH_TEXTURE_SPEC:
                    paiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path);
                    break;
                case MESH_TEXTURE_HEIGHT:
                    paiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &path);
                    break;
                case MESH_TEXTURE_ALPHA:
                    paiMaterial->GetTexture(aiTextureType_OPACITY, 0, &path);
                    break;
                default:
                    trace(L"Unknown texture type");
                    return E_FAIL;
                }

                std::string s(path.data);
                s = Folder + '\\' + s;
                pRtrMaterial->m_SRV[i] = CreateShaderResourceViewFromFile(pDevice, string_2_wstring(s), bSrgb);
                assert(pRtrMaterial->m_SRV[i].GetInterfacePtr());
                m_bHasTextures = true;
            }
        }

        aiColor3D diffuse;
        aiString name;
        paiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        paiMaterial->Get(AI_MATKEY_NAME, name);
        std::string nameStr = std::string(name.C_Str());
        std::transform(nameStr.begin(), nameStr.end(), nameStr.begin(), ::tolower);

        pRtrMaterial->m_DiffuseColor = float3(diffuse.r, diffuse.g, diffuse.b);
        pRtrMaterial->m_MaterialName = nameStr;
        m_Materials.push_back(pRtrMaterial);
    }

    return hr;
}


const dx_mesh_vector& CDxModel::GetMeshesByMaterialName(const std::string& sGroupName) const
{
    auto it = m_pMeshGroups.find(sGroupName);

    if(it != m_pMeshGroups.end())
    {
        return it->second;
    }
    else
    {
        return m_pMeshGroups.at("");
    }

}

const dx_mesh_vector& CDxModel::GetAllMeshes() const
{
    return m_pMeshes;
}

void CDxModel::Animate(const float4x4& WorldMatrix)
{
    m_WorldMatrix = WorldMatrix;
}

ID3D11InputLayout* CDxMesh::GetInputLayout(ID3D11DeviceContext* pCtx, ID3DBlob* pVsBlob) const
{
    if(m_InputLayouts.find(pVsBlob) == m_InputLayouts.end())
    {
        ID3D11DevicePtr pDevice;
        pCtx->GetDevice(&pDevice);
        CreateElementLayout(pDevice, pVsBlob);
    }
    return m_InputLayouts[pVsBlob];
}

void CDxMesh::SetMeshData(ID3D11DeviceContext* pCtx, ID3DBlob* pVsBlob)
{
    pCtx->IASetIndexBuffer(GetIndexBuffer(), GetIndexBufferFormat(), 0);
    pCtx->IASetInputLayout(GetInputLayout(pCtx, pVsBlob));
    UINT z = 0;
    UINT stride = GetVertexStride();
    ID3D11Buffer* pBuf = GetVertexBuffer();
    pCtx->IASetVertexBuffers(0, 1, &pBuf, &stride, &z);
    pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

template<typename IndexType>
HRESULT CDxMesh::CreateIndexBufferInternal(const aiMesh* pMesh, ID3D11Device* pDevice)
{
    IndexType* pIndices = new IndexType[m_IndexCount];
    if(pIndices == nullptr)
    {
        trace(L"Can't allocate space for indices");
        return E_OUTOFMEMORY;
    }

    for(UINT i = 0; i < pMesh->mNumFaces; i++)
    {
        if(pMesh->mFaces[i].mNumIndices != 3)
        {
            trace(L"Face is not a triangle");
            return E_INVALIDARG;
        }
        pIndices[i * 3 + 0] = (IndexType)(pMesh->mFaces[i].mIndices[0]);
        pIndices[i * 3 + 1] = (IndexType)(pMesh->mFaces[i].mIndices[1]);
        pIndices[i * 3 + 2] = (IndexType)(pMesh->mFaces[i].mIndices[2]);
    }

    D3D11_BUFFER_DESC IbDesc;
    IbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IbDesc.ByteWidth = sizeof(IndexType)*m_IndexCount;
    IbDesc.CPUAccessFlags = 0;
    IbDesc.MiscFlags = 0;
    IbDesc.StructureByteStride = 0;
    IbDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pIndices;
    InitData.SysMemPitch = IbDesc.ByteWidth;
    InitData.SysMemSlicePitch = IbDesc.ByteWidth;

    HRESULT hr = pDevice->CreateBuffer(&IbDesc, &InitData, &m_IB);
    if(hr != S_OK)
    {
        trace(L"Failed to create index buffer");
    }

    SAFE_DELETE_ARRAY(pIndices);
    return S_OK;
}

HRESULT CDxMesh::CreateIndexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice)
{
    // Assuming everything is triangles. I've never seen lines/points used directly in models
    m_IndexCount = pMesh->mNumFaces * 3;

    // Save some space by choosing the best index buffer type (16/32 bit)
    if(m_IndexCount < D3D11_16BIT_INDEX_STRIP_CUT_VALUE)
    {
        m_IndexType = DXGI_FORMAT_R16_UINT;
        return CreateIndexBufferInternal<UINT16>(pMesh, pDevice);
    }
    else
    {
        m_IndexType = DXGI_FORMAT_R32_UINT;
        return CreateIndexBufferInternal<UINT32>(pMesh, pDevice);
    }
}

HRESULT CDxMesh::SetVertexElementOffsets(const aiMesh* pMesh)
{
    UINT Offset = 0;
    // Must have position!!!
    if(pMesh->HasPositions() == false)
    {
        trace(L"Loaded mesh with no positions!");
        return E_FAIL;
    }
    m_VertexElementOffsets[MESH_ELEMENT_POSITION] = Offset;
    Offset += sizeof(float3);

    if(pMesh->HasNormals())
    {
        m_VertexElementOffsets[MESH_ELEMENT_NORMAL] = Offset;
		Offset += sizeof(float3);
    }

    if(pMesh->HasTangentsAndBitangents())
    {
        m_VertexElementOffsets[MESH_ELEMENT_TANGENT] = Offset;
		Offset += sizeof(float3);
        m_VertexElementOffsets[MESH_ELEMENT_BI_TANGENT] = Offset;
		Offset += sizeof(float3);
    }

    // Supporting only tex coord0
    if(pMesh->HasTextureCoords(0))
    {
        m_VertexElementOffsets[MESH_ELEMENT_TEXCOORD0] = Offset;
		Offset += sizeof(float3);
    }

    for(int i = 1; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
    {
        if(pMesh->HasTextureCoords(i))
        {
            trace(L"DxModel only support texcoord 0");
            return E_FAIL;
        }
    }

    if(pMesh->HasVertexColors(0))
    {
        m_VertexElementOffsets[MESH_ELEMENT_DIFFUSE] = Offset;
        Offset += sizeof(DWORD); // To save space we will store it as RGBA8_UNORM
    }

    if(pMesh->HasBones())
    {
        m_VertexElementOffsets[MESH_ELEMENT_BONE_INDICES] = Offset;
        Offset += sizeof(UINT8)*MESH_MAX_BONE_PER_VERTEX;
        m_VertexElementOffsets[MESH_ELEMENT_BONE_WEIGHTS] = Offset;
        Offset += sizeof(float)*MESH_MAX_BONE_PER_VERTEX;
    }

    m_VertexStride = Offset;
    return S_OK;
}

#define MESH_LOAD_INPUT(_vertex_index, _element, _field)                                                \
    Offset = m_VertexElementOffsets[_element];                                                              \
if(Offset != INVALID_ELEMENT_OFFSET)                                                                    \
{                                                                                                       \
    BYTE* pDst = pVertex + Offset;                                                                      \
    BYTE* pSrc = (BYTE*)&(pMesh->_field[_vertex_index]);                                                \
    memcpy(pDst, pSrc, sizeof(pMesh->_field[0]));                                                       \
}

void CDxMesh::LoadBones(const aiMesh* pMesh, BYTE* pVertexData, string_int_map& BonesMap)
{
    if(pMesh->mNumBones > 0xff)
    {
        trace(L"Too many bones");
    }

    for(UINT Bone = 0; Bone < pMesh->mNumBones; Bone++)
    {
        const aiBone* pBone = pMesh->mBones[Bone];
        for(UINT WeightID = 0; WeightID < pBone->mNumWeights; WeightID++)
        {
            const aiVertexWeight* pWeight = &pBone->mWeights[WeightID];
            BYTE* pVertex = pVertexData + (pWeight->mVertexId * m_VertexStride);

            BYTE* pBoneIDs = (BYTE*)(pVertex + m_VertexElementOffsets[MESH_ELEMENT_BONE_INDICES]);
            float* pVertexWeights = (float*)(pVertex + m_VertexElementOffsets[MESH_ELEMENT_BONE_WEIGHTS]);
            int j;
            for(j = 0; j < MESH_MAX_BONE_PER_VERTEX; j++)
            {
                if(pVertexWeights[j] == 0)
                {
                    pBoneIDs[j] = (BYTE)BonesMap[pBone->mName.C_Str()];
                    pVertexWeights[j] = pWeight->mWeight;
                    break;
                }
            }

            if(j == MESH_MAX_BONE_PER_VERTEX)
            {
                trace(L"Too many bones");
            }
        }
    }

    for(UINT i = 0; i < m_VertexCount; i++)
    {
        BYTE* pVertex = pVertexData + (i * m_VertexStride);
        float* pVertexWeights = (float*)(pVertex + m_VertexElementOffsets[MESH_ELEMENT_BONE_WEIGHTS]);

        float f = 0;
        for(int j = 0; j < MESH_MAX_BONE_PER_VERTEX; j++)
        {
            f += pVertexWeights[j];
        }

        for(int j = 0; j < MESH_MAX_BONE_PER_VERTEX; j++)
        {
            pVertexWeights[j] /= f;
        }
    }
}

HRESULT CDxMesh::CreateVertexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice, string_int_map& BonesMap)
{
    BYTE* pInitData = new BYTE[m_VertexStride * m_VertexCount];
    ZeroMemory(pInitData, m_VertexStride * m_VertexCount);

    for(UINT i = 0; i < m_VertexCount; i++)
    {
        BYTE* pVertex = pInitData + (m_VertexStride * i);
        UINT Offset;

        MESH_LOAD_INPUT(i, MESH_ELEMENT_POSITION, mVertices);
        MESH_LOAD_INPUT(i, MESH_ELEMENT_NORMAL, mNormals);
        MESH_LOAD_INPUT(i, MESH_ELEMENT_TANGENT, mTangents);
        MESH_LOAD_INPUT(i, MESH_ELEMENT_BI_TANGENT, mBitangents);

        for(UINT j = 0; j < pMesh->GetNumUVChannels(); j++)
        {
            MESH_LOAD_INPUT(i, MESH_ELEMENT_TEXCOORD0 + j, mTextureCoords[j]);
        }

        m_BoundingBox.Min.x = min(m_BoundingBox.Min.x, pMesh->mVertices[i].x);
        m_BoundingBox.Min.y = min(m_BoundingBox.Min.y, pMesh->mVertices[i].y);
        m_BoundingBox.Min.z = min(m_BoundingBox.Min.z, pMesh->mVertices[i].z);
        m_BoundingBox.Max.x = max(m_BoundingBox.Max.x, pMesh->mVertices[i].x);
        m_BoundingBox.Max.y = max(m_BoundingBox.Max.y, pMesh->mVertices[i].y);
        m_BoundingBox.Max.z = max(m_BoundingBox.Max.z, pMesh->mVertices[i].z);

        // Colors require special handling since we need to normalize them
        Offset = m_VertexElementOffsets[MESH_ELEMENT_DIFFUSE];
        if(Offset != INVALID_ELEMENT_OFFSET)
        {
            BYTE* pColor = pVertex + Offset;
            float f = pMesh->mColors[0][i].r;
            pColor[0] = (BYTE)(255 * min(max(f, 0), 1));
            f = pMesh->mColors[0][i].g;
            pColor[1] = (BYTE)(255 * min(max(f, 0), 1));
            f = pMesh->mColors[0][i].b;
            pColor[2] = (BYTE)(255 * min(max(f, 0), 1));
            f = pMesh->mColors[0][i].a;
            pColor[3] = (BYTE)(255 * min(max(f, 0), 1));
        }
    }

    if(pMesh->HasBones())
    {
        LoadBones(pMesh, pInitData, BonesMap);
    }

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.ByteWidth = m_VertexStride * m_VertexCount;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = m_VertexStride;
    vbDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pInitData;
    InitData.SysMemPitch = vbDesc.ByteWidth;
    InitData.SysMemSlicePitch = vbDesc.ByteWidth;

    HRESULT hr = pDevice->CreateBuffer(&vbDesc, &InitData, &m_VB);
    if(hr != S_OK)
    {
        trace(L"Failed to create vertex buffer");
    }
    SAFE_DELETE_ARRAY(pInitData);
    return hr;
}

CDxMesh::CDxMesh(const std::string& Name, const CDxModel* pModel) : m_Name(Name), m_pModel(pModel)
{
    m_VertexCount = 0;
    m_IndexCount = 0;
    m_VertexStride = 0;

    for(int i = 0; i < ARRAYSIZE(m_VertexElementOffsets); i++)
    {
        m_VertexElementOffsets[i] = INVALID_ELEMENT_OFFSET;
    }
}

HRESULT CDxMesh::CreateElementLayout(ID3D11Device* pDevice, ID3DBlob* pVsBlob) const
{
    HRESULT hr = S_OK;
    UINT BonesIDOffset = HasBones() ? sizeof(UINT8)* 4 : 0;
    UINT BonesWeightOffset = HasBones() ? sizeof(float)* 4 : 0;

    D3D11_INPUT_ELEMENT_DESC desc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, GetVertexElementOffset(MESH_ELEMENT_POSITION), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, GetVertexElementOffset(MESH_ELEMENT_NORMAL), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, GetVertexElementOffset(MESH_ELEMENT_TANGENT), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, GetVertexElementOffset(MESH_ELEMENT_BI_TANGENT), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, GetVertexElementOffset(MESH_ELEMENT_BONE_WEIGHTS), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, GetVertexElementOffset(MESH_ELEMENT_BONE_WEIGHTS) + BonesWeightOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_IDS", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, GetVertexElementOffset(MESH_ELEMENT_BONE_INDICES), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_IDS", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, GetVertexElementOffset(MESH_ELEMENT_BONE_INDICES) + BonesIDOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, GetVertexElementOffset(MESH_ELEMENT_TEXCOORD0), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    ID3D11InputLayout* pLayout;
    verify_return(pDevice->CreateInputLayout(desc, ARRAYSIZE(desc), pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &pLayout));
    m_InputLayouts[pVsBlob] = pLayout;

    return hr;
}

CDxMesh* CDxMesh::CreateMesh(const aiMesh* pMesh, const CDxModel* pModel, ID3D11Device* pDevice, string_int_map& BonesMap)
{
    CDxMesh* pRtrMesh = new CDxMesh(pMesh->mName.C_Str(), pModel);
    if(pRtrMesh)
    {
        // OK, one mesh, let's initialize it
        pRtrMesh->m_VertexCount = pMesh->mNumVertices;
        HRESULT hr = pRtrMesh->SetVertexElementOffsets(pMesh);  // Should be called before CreateVertexBuffer()
        hr |= pRtrMesh->CreateIndexBuffer(pMesh, pDevice);
        hr |= pRtrMesh->CreateVertexBuffer(pMesh, pDevice, BonesMap);

        pRtrMesh->m_MaterialIndex = pMesh->mMaterialIndex;
        if(hr != S_OK)
        {
            SAFE_DELETE(pRtrMesh);
        }
    }
    else
    {
        trace(L"Failed to allocate memory for the mesh");
    }
    return pRtrMesh;
}

CDxMesh::~CDxMesh()
{
	for(auto il : m_InputLayouts)
	{
		il.second->Release();
	}
}