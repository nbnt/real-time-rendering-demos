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

            Filename - RtrMesh.cpp
---------------------------------------------------------------------------
*/
#include "RtrMesh.h"
#include "assimp.hpp"
#include "aiPostProcess.h"

using namespace Assimp;

std::string string_from_wchar(WCHAR src[])
{
    // Simple function to create single byte strings. Will only work with english characters
    std::wstring wStr(src);
    std::string dst(wStr.length(), ' ');

    for(std::wstring::size_type i = 0 ; i < wStr.length() ; i++)
    {
        dst[i] = (char)wStr[i];
    }

    return dst;
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
    
    for(int i = 0 ; i < AI_MAX_NUMBER_OF_TEXTURECOORDS ; i++)
    {
        bHasTexcoord[i] = pScene->mMeshes[0]->HasTextureCoords(i);
    }


    for(UINT i = 1 ; i < pScene->mNumMeshes ; i++)
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

        for(int j = 0 ; j < AI_MAX_NUMBER_OF_TEXTURECOORDS ; j++)
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

CRtrModel::CRtrModel()
{
    m_bHasTextures = false;
}

CRtrModel::~CRtrModel()
{
    for(UINT i = 0 ; i < m_Materials.size(); i++)
    {
        SAFE_DELETE(m_Materials[i]);
    }

    for(UINT i = 0 ; i < m_pMeshes.size() ; i++)
    {
        SAFE_DELETE(m_pMeshes[i]);
    }
}

CRtrModel* CRtrModel::LoadModelFromFile(WCHAR Filename[], UINT flags, ID3D11Device* pDevice)
{
    Importer importer;    
    std::string file = string_from_wchar(Filename);
    // aiProcess_ConvertToLeftHanded will make necessary adjusments so that the model is ready for D3D. Check the assimp documenation for more info.

#ifdef _DEBUG
    flags = flags | aiProcess_ValidateDataStructure;
#endif

    const aiScene* pScene = importer.ReadFile(file, flags | 
        aiProcess_ConvertToLeftHanded | 
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_OptimizeMeshes | 
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_FixInfacingNormals |
        aiProcess_FindInvalidData );

    if(pScene == NULL)
    {
        std::wstring str(L"Can't open mesh file");
        str = str+std::wstring(Filename);
        trace(str.c_str());
        return NULL;
    }

    // Verify we support this scene
    if(VerifyScene(pScene) == false)
    {
        return NULL;
    }
     
    CRtrModel* pModel = new CRtrModel;
    if(pModel)
    {
        for(UINT i = 0 ; i < pScene->mNumMeshes ; i++)
        {
            CRtrMesh* pMesh = CRtrMesh::CreateMesh(pScene->mMeshes[i], pDevice);
            if(pMesh == NULL)
            {
                return NULL;
            }
            pModel->m_pMeshes.push_back(pMesh);
        }
        if(pModel->CreateMaterials(pScene) != S_OK)
        {
            SAFE_DELETE(pModel);
        }
        
        // Calculate the radius and the center, vertex and primitive count
        SBoundingBox box;
        pModel->m_Vertices = pModel->m_Primitives = 0;

        for(UINT i = 0 ; i < pModel->m_pMeshes.size() ; i++)
        {
            const SBoundingBox& MeshBox = pModel->m_pMeshes[i]->GetBoundingBox();
            box.xMin = min(box.xMin, MeshBox.xMin);
            box.yMin = min(box.yMin, MeshBox.yMin);
            box.zMin = min(box.zMin, MeshBox.zMin);

            box.xMax = max(box.xMax, MeshBox.xMax);
            box.yMax = max(box.yMax, MeshBox.yMax);
            box.zMax = max(box.zMax, MeshBox.zMax);
            pModel->m_Vertices += pModel->m_pMeshes[i]->GetVertexCount();
            pModel->m_Primitives += (pModel->m_pMeshes[i]->GetIndexCount()) / 3;
        }

        pModel->m_Center.x = ((box.xMax + box.xMin) / 2);
        pModel->m_Center.y = ((box.yMax + box.yMin) / 2);
        pModel->m_Center.z = ((box.zMax + box.zMin) / 2);

        float maxLength1 = sqrtf(box.xMax*box.xMax + box.yMax*box.yMax + box.zMax*box.zMax);
        float maxLength2 = sqrtf(box.xMin*box.xMin + box.yMin*box.yMin + box.zMin*box.zMin);
        pModel->m_Radius = max(maxLength1, maxLength2);
    }
    else
    {
        trace(L"Failed to allocate memory for the model\n");
    }

    // If something fails, I don't deallocate anything, since I assume the process will exit anyway
    return pModel;
}


HRESULT CRtrModel::CreateMaterials(const aiScene* pScene)
{
    HRESULT hr = S_OK;

    for(UINT i = 0 ; i < pScene->mNumMaterials ; i++)
    {
        const aiMaterial* paiMaterial = pScene->mMaterials[i];
        SRtrMaterial* pRtrMaterial = new SRtrMaterial;
        if(pRtrMaterial == NULL)
        {
            trace(L"Can't allocate memory for material");
            return E_FAIL;
        }

        UINT diffuseCount = paiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
        if(diffuseCount >= 1)
        {
            if(diffuseCount != 1)
            {
                trace(L"More then 1 diffuse textures");
                return E_FAIL;
            }

            // Get the texture name
            aiString path;
            paiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            std::string s(path.data);
            // null-call to get the size
            std::wstring w;
            w.resize(s.length() + 1);
            ::mbstowcs_s(NULL, &w[0], s.length() + 1, s.c_str(), s.length());

            DXUTGetGlobalResourceCache().CreateTextureFromFile(DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext(), w.c_str(), &pRtrMaterial->m_SRV[RTR_MESH_TEXTURE_DIFFUSE], DXUTIsInGammaCorrectMode());
            m_bHasTextures = true;
        }
        aiColor3D diffuse;
        paiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        pRtrMaterial->m_DiffuseColor = D3DXVECTOR3(diffuse.r, diffuse.g, diffuse.b);
        m_Materials.push_back(pRtrMaterial);
    }

    return hr;
}

void CRtrModel::Draw(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Set general mesh parameters
    for(UINT i = 0 ; i < m_pMeshes.size() ; i++)
    {
        CRtrMesh* pMesh = m_pMeshes[i];

        pd3dImmediateContext->IASetIndexBuffer(pMesh->GetIndexBuffer(), pMesh->GetIndexBufferFormat(), 0);
        UINT z = 0;
        UINT stride = pMesh->GetVertexStride();
        ID3D11Buffer* pBuf = pMesh->GetVertexBuffer();
        pd3dImmediateContext->IASetVertexBuffers(0, 1, &pBuf, &stride, &z);
        pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        UINT MaterialIndex = pMesh->GetMaterialIndex();
        ID3D11ShaderResourceView* pSRV = m_Materials[MaterialIndex]->m_SRV[RTR_MESH_TEXTURE_DIFFUSE];
        pd3dImmediateContext->PSSetShaderResources(0, 1, &pSRV);

        pd3dImmediateContext->DrawIndexed(pMesh->GetIndexCount(), 0, 0);
    }
}

template<typename IndexType>
HRESULT CRtrMesh::CreateIndexBufferInternal(const aiMesh* pMesh, ID3D11Device* pDevice)
{
    IndexType* pIndices = new IndexType[m_IndexCount];
    if(pIndices == NULL)
    {
        trace(L"Can't allocate space for indices");
        return E_OUTOFMEMORY;
    }

    for(UINT i = 0 ; i < pMesh->mNumFaces ; i++)
    {
        if(pMesh->mFaces[i].mNumIndices != 3)
        {
            trace(L"Face is not a triangle");
            return E_INVALIDARG;
        }
        pIndices[i*3 + 0] = (IndexType)(pMesh->mFaces[i].mIndices[0]);
        pIndices[i*3 + 1] = (IndexType)(pMesh->mFaces[i].mIndices[1]);
        pIndices[i*3 + 2] = (IndexType)(pMesh->mFaces[i].mIndices[2]);
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

    HRESULT hr = pDevice->CreateBuffer(&IbDesc, &InitData, &m_pIB);
    if(hr != S_OK)
    {
        trace(L"Failed to create index buffer");
    }
    
    SAFE_DELETE_ARRAY(pIndices);
    return S_OK;
}

HRESULT CRtrMesh::CreateIndexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice)
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

HRESULT CRtrMesh::SetVertexElementOffsets(const aiMesh* pMesh)
{
    UINT Offset = 0;
    // Must have position!!!
    if(pMesh->HasPositions() == false)
    {
        trace(L"Loaded mesh with no positions!");
        return E_FAIL;
    }
    m_VertexElementOffsets[RTR_MESH_ELEMENT_POSITION] = Offset;
    Offset += sizeof(D3DXVECTOR3);

    if(pMesh->HasNormals())
    {
        m_VertexElementOffsets[RTR_MESH_ELEMENT_NORMAL] = Offset;
        Offset += sizeof(D3DXVECTOR3);
    }

    if(pMesh->HasTangentsAndBitangents())
    {
        m_VertexElementOffsets[RTR_MESH_ELEMENT_TANGENT] = Offset;
        Offset += sizeof(D3DXVECTOR3);
        m_VertexElementOffsets[RTR_MESH_ELEMENT_BI_TANGENT] = Offset;
        Offset += sizeof(D3DXVECTOR3);
    }

    for(int i = 0 ; i < AI_MAX_NUMBER_OF_TEXTURECOORDS ; i++)
    {
        if(pMesh->HasTextureCoords(i))
        {
            m_VertexElementOffsets[RTR_MESH_ELEMENT_TEXCOORD0 + i] = Offset;
            Offset += sizeof(D3DXVECTOR3);
        }
    }

    if(pMesh->HasVertexColors(0))
    {
        m_VertexElementOffsets[RTR_MESH_ELEMENT_DIFFUSE] = Offset;
        Offset += sizeof(DWORD); // To save space we will store it as RGBA8_UNORM
    }

    if(pMesh->HasBones())
    {
        m_VertexElementOffsets[RTR_MESH_ELEMENT_BONE_INDICES] = Offset;
        Offset += sizeof(BYTE)*4;
        m_VertexElementOffsets[RTR_MESH_ELEMENT_BONE_INDICES] = Offset;
        Offset += sizeof(BYTE)*4;
    }

    m_VertexStride = Offset;
    return S_OK;
}

#define RTR_MESH_LOAD_INPUT(_vertex_index, _element, _field)                                                \
    Offset = m_VertexElementOffsets[_element];                                                              \
    if(Offset != INVALID_ELEMENT_OFFSET)                                                                    \
    {                                                                                                       \
        BYTE* pDst = pVertex + Offset;                                                                      \
        BYTE* pSrc = (BYTE*)&(pMesh->_field[_vertex_index]);                                                \
        memcpy(pDst, pSrc, sizeof(pMesh->_field[0]));                                                       \
    }

HRESULT CRtrMesh::CreateVertexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice)
{
    BYTE* pInitData = new BYTE[m_VertexStride * m_VertexCount];

    for(UINT  i = 0 ; i < m_VertexCount ; i++)
    {
        BYTE* pVertex = pInitData + (m_VertexStride * i);
        UINT Offset;

        RTR_MESH_LOAD_INPUT(i, RTR_MESH_ELEMENT_POSITION, mVertices);
        RTR_MESH_LOAD_INPUT(i, RTR_MESH_ELEMENT_NORMAL, mNormals);
        RTR_MESH_LOAD_INPUT(i, RTR_MESH_ELEMENT_TANGENT, mTangents);
        RTR_MESH_LOAD_INPUT(i, RTR_MESH_ELEMENT_BI_TANGENT, mBitangents);
        
        for(int j = 0 ; j < AI_MAX_NUMBER_OF_TEXTURECOORDS ; j++)
        {
            RTR_MESH_LOAD_INPUT(i, RTR_MESH_ELEMENT_TEXCOORD0 + j, mTextureCoords[j]);
        }

        m_BoundingBox.xMin = min(m_BoundingBox.xMin, pMesh->mVertices[i].x);
        m_BoundingBox.yMin = min(m_BoundingBox.yMin, pMesh->mVertices[i].y);
        m_BoundingBox.zMin = min(m_BoundingBox.zMin, pMesh->mVertices[i].z);
        m_BoundingBox.xMax = max(m_BoundingBox.xMax, pMesh->mVertices[i].x);
        m_BoundingBox.yMax = max(m_BoundingBox.yMax, pMesh->mVertices[i].y);
        m_BoundingBox.zMax = max(m_BoundingBox.zMax, pMesh->mVertices[i].z);

        // Colors require special handling since we need to normalize them
        Offset = m_VertexElementOffsets[RTR_MESH_ELEMENT_DIFFUSE];
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

    HRESULT hr = pDevice->CreateBuffer(&vbDesc, &InitData, &m_pVB);
    if(hr != S_OK)
    {
        trace(L"Failed to create vertex buffer");
    }
    SAFE_DELETE_ARRAY(pInitData);
    return hr;
}

CRtrMesh::CRtrMesh()
{
    m_pVB = NULL;
    m_pIB = NULL;
    m_VertexCount = 0;
    m_IndexCount = 0;
    m_VertexStride = 0;

    for(int i = 0 ; i < ARRAYSIZE(m_VertexElementOffsets) ; i++)
    {
        m_VertexElementOffsets[i] = INVALID_ELEMENT_OFFSET;
    }
}

CRtrMesh::~CRtrMesh()
{
    SAFE_RELEASE(m_pIB);
    SAFE_RELEASE(m_pVB);
}

CRtrMesh* CRtrMesh::CreateMesh(const aiMesh* pMesh, ID3D11Device* pDevice)
{
    CRtrMesh* pRtrMesh = new CRtrMesh;
    if(pRtrMesh)
    {
        // OK, one mesh, let's initialize it
        pRtrMesh->m_VertexCount = pMesh->mNumVertices;
        HRESULT hr = pRtrMesh->SetVertexElementOffsets(pMesh);  // Should be called before CreateVertexBuffer()
        hr |= pRtrMesh->CreateIndexBuffer(pMesh, pDevice);
        hr |= pRtrMesh->CreateVertexBuffer(pMesh, pDevice);

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
