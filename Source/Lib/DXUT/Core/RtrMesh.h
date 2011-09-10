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

            Filename - RtrMesh.h
---------------------------------------------------------------------------
*/
#include "DXUT.h"
#include "aiScene.h"
#include "SDKmisc.h"
#include <vector>

C_ASSERT(AI_MAX_NUMBER_OF_TEXTURECOORDS == 4);
#define INVALID_ELEMENT_OFFSET ((UINT)-1)

enum RTR_MESH_ELEMENT_TYPE
{
    RTR_MESH_ELEMENT_POSITION,
    RTR_MESH_ELEMENT_NORMAL,
    RTR_MESH_ELEMENT_TANGENT,
    RTR_MESH_ELEMENT_BI_TANGENT,
    RTR_MESH_ELEMENT_TEXCOORD0,
    RTR_MESH_ELEMENT_TEXCOORD1,
    RTR_MESH_ELEMENT_TEXCOORD2,
    RTR_MESH_ELEMENT_TEXCOORD3,
    RTR_MESH_ELEMENT_DIFFUSE,
    RTR_MESH_ELEMENT_BONE_INDICES,
    RTR_MESH_ELEMENT_BONE_WEIGHTS,

    // Must be last
    RTR_MESH_NUM_ELEMENT_TYPES
};

enum RTR_MESH_TEXTURE_TYPE
{
    RTR_MESH_TEXTURE_DIFFUSE,

    // Must be last
    RTR_MESH_NUM_TEXTURE_TYPES
};

struct SBoundingBox
{
    float xMax, xMin, yMax, yMin, zMax, zMin;
    SBoundingBox()
    {
        xMin = yMin = zMin = FLT_MAX;
        xMax = yMax = zMax = FLT_MIN;
    }
};

class CRtrMesh
{
public:
    static CRtrMesh* CreateMesh(const aiMesh* pMesh, ID3D11Device* pDevice);
    ~CRtrMesh();
    UINT GetVertexElementOffset(RTR_MESH_ELEMENT_TYPE e) {return m_VertexElementOffsets[e];}
    
    ID3D11Buffer* GetIndexBuffer()  {return m_pIB;}
    ID3D11Buffer* GetVertexBuffer() {return m_pVB;}
    DXGI_FORMAT GetIndexBufferFormat() {return m_IndexType;}
    UINT GetVertexStride() {return m_VertexStride;}
    UINT GetIndexCount() {return m_IndexCount;}
    UINT GetVertexCount() {return m_VertexCount;}
    UINT GetMaterialIndex() {return m_MaterialIndex;}
    const SBoundingBox& GetBoundingBox() {return m_BoundingBox;}
private:
    CRtrMesh();
    HRESULT CreateVertexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice);
    HRESULT CreateIndexBuffer(const aiMesh* pMesh, ID3D11Device* pDevice);
    HRESULT SetVertexElementOffsets(const aiMesh* pMesh);

    template<typename IndexType>
    HRESULT CreateIndexBufferInternal(const aiMesh* pMesh, ID3D11Device* pDevice);

    UINT m_VertexCount;
    UINT m_IndexCount;
    DXGI_FORMAT m_IndexType;
    UINT m_VertexStride;

    ID3D11Buffer* m_pVB;
    ID3D11Buffer* m_pIB;
    UINT m_MaterialIndex;
    UINT m_VertexElementOffsets[RTR_MESH_NUM_ELEMENT_TYPES];

    SBoundingBox m_BoundingBox;
};

struct SRtrMaterial
{
    SRtrMaterial() {ZeroMemory(m_SRV, sizeof(m_SRV));}
    ~SRtrMaterial()
    {
        for(int i = 0 ; i < RTR_MESH_NUM_TEXTURE_TYPES ; i++)
        {
            SAFE_RELEASE(m_SRV[i]);
        }
    }

    ID3D11ShaderResourceView* m_SRV[RTR_MESH_NUM_TEXTURE_TYPES];
    D3DXVECTOR3 m_DiffuseColor;   
};

// Simple mesh loader. Used to load single mesh scenes, so make sure you know what you are doing
class CRtrModel
{
public:
    ~CRtrModel();
    static CRtrModel* LoadModelFromFile(WCHAR Filename[], UINT flags, ID3D11Device* pDevice);
    UINT GetVertexElementOffset(RTR_MESH_ELEMENT_TYPE e) {return m_pMeshes[0]->GetVertexElementOffset(e);}
    void Draw(ID3D11DeviceContext* pd3dImmediateContext);
    float GetRadius() {return m_Radius;}
    D3DXVECTOR3 GetCenter(){return m_Center;}

    UINT GetVertexCount() {return m_Vertices;}
    UINT GetPrimitiveCount() {return m_Primitives;}
    bool HasTextures() {return m_bHasTextures;}
private:
    CRtrModel();
    HRESULT CreateMaterials(const aiScene* pScene);
    float m_Radius;
    D3DXVECTOR3 m_Center;

    UINT m_Vertices;
    UINT m_Primitives;

    std::vector<CRtrMesh*> m_pMeshes;
    std::vector<SRtrMaterial*> m_Materials;
    bool m_bHasTextures;
};