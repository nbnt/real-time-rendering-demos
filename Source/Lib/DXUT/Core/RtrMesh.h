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
#include "SDKmisc.h"
#include <vector>

// Forward declarations
class CRtrMesh;
struct SRtrMaterial;
struct aiScene;

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

// Simple mesh loader. Used to load single mesh scenes, so make sure you know what you are doing
class CRtrModel
{
public:
    ~CRtrModel();
    static CRtrModel* LoadModelFromFile(WCHAR Filename[], ID3D11Device* pDevice);
    UINT GetVertexElementOffset(RTR_MESH_ELEMENT_TYPE e);
    void Draw(ID3D11DeviceContext* pd3dImmediateContext);
    float GetRadius() {return m_Radius;}
    D3DXVECTOR3 GetCenter(){return m_Center;}

    bool HasTextures() {return m_bHasTextures;}

    bool SetMeshData(UINT MeshID, ID3D11DeviceContext* pd3dImmediateContext);
    UINT GetMeshVertexCount(UINT MeshID);
    UINT GetMeshIndexCount(UINT MeshID);
    UINT GetMeshesCount() {return (UINT)m_pMeshes.size();}
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