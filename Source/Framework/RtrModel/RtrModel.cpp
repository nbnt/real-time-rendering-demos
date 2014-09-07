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

Filename: RtrModel.cpp
---------------------------------------------------------------------------*/
#include "..\RtrModel.h"
#include "..\StringUtils.h"
#include "Importer.hpp"
#include "postprocess.h"
#include "scene.h"

float4x4 aiMatToD3D(const aiMatrix4x4& aiMat)
{
	float4x4 d3dMat;
	d3dMat.m[0][0] = aiMat.a1; d3dMat.m[0][1] = aiMat.a2; d3dMat.m[0][2] = aiMat.a3; d3dMat.m[0][3] = aiMat.a4;
	d3dMat.m[1][0] = aiMat.b1; d3dMat.m[1][1] = aiMat.b2; d3dMat.m[1][2] = aiMat.b3; d3dMat.m[1][3] = aiMat.b4;
	d3dMat.m[2][0] = aiMat.c1; d3dMat.m[2][1] = aiMat.c2; d3dMat.m[2][2] = aiMat.c3; d3dMat.m[2][3] = aiMat.c4;
	d3dMat.m[3][0] = aiMat.d1; d3dMat.m[3][1] = aiMat.d2; d3dMat.m[3][2] = aiMat.d3; d3dMat.m[3][3] = aiMat.d4;
	return d3dMat;
}


CRtrModel::CRtrModel()
{

}

CRtrModel::~CRtrModel()
{
	for(auto pMesh : m_Meshes)
	{
		delete pMesh;
	}

	for(auto pMaterial : m_Materials)
	{
		delete pMaterial;
	}
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

	return b;
}

CRtrModel* CRtrModel::CreateFromFile(const std::wstring& Filename, ID3D11Device* pDevice)
{
	Assimp::Importer importer;
	std::wstring WideFullpath;
	HRESULT hr = FindFileInCommonDirs(Filename, WideFullpath);
	if(FAILED(hr))
	{
		trace(std::wstring(L"Can't find model file ") + Filename);
		return nullptr;
	}

	// aiProcess_ConvertToLeftHanded will make necessary adjustments so that the model is ready for D3D. Check the assimp documentation for more info.	
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
		aiProcess_LimitBoneWeights|
		0);

	if((pScene == nullptr) || (VerifyScene(pScene) == false))
	{
		std::wstring str(L"Can't open model file ");
		str = str + std::wstring(Filename);
		trace(str.c_str());
		return nullptr;
	}

	CRtrModel* pModel = new CRtrModel;

	// Extract the folder name
	auto last = Fullpath.find_last_of("/\\");
	std::string Folder = Fullpath.substr(0, last);

	// Init the model
	if(pModel->Init(pScene, pDevice, Folder) == false)
	{
		delete pModel;
		pModel = nullptr;
	}
	return pModel;
}

bool CRtrModel::Init(const aiScene* pScene, ID3D11Device* pDevice, const std::string& ModelFolder)
{
	// Order of initialization matters, materials, bones and animations need to loaded before mesh initialization
	if(CreateMaterials(pScene, pDevice, ModelFolder) == false)
	{

		return false;
	}

	if(CreateDrawList(pScene, pDevice) == false)
	{
		return false;
	}

	CalculateModelProperties();
	return true;
}

bool CRtrModel::CreateMaterials(const aiScene* pScene, ID3D11Device* pDevice, const std::string& ModelFolder)
{
	for(UINT i = 0; i < pScene->mNumMaterials; i++)
	{
		const aiMaterial* pAiMaterial = pScene->mMaterials[i];
		const CRtrMaterial* pRtrMaterial = new CRtrMaterial(pAiMaterial, pDevice, ModelFolder);
		if(pRtrMaterial == nullptr)
		{
			trace(L"Can't allocate memory for material");
			return false;
		}

		m_Materials.push_back(pRtrMaterial);
	}

	return true;
}

bool CRtrModel::ParseAiSceneNode(const aiNode* pCurrnet, const aiScene* pScene, ID3D11Device* pDevice, std::map<UINT, UINT>& AiToRtrMeshId)
{
	if(pCurrnet->mNumMeshes)
	{
		SDrawListNode DrawNode;

		// Initialize the meshes
		for(UINT i = 0; i < pCurrnet->mNumMeshes; i++)
		{
			UINT AiId = pCurrnet->mMeshes[i];
			CRtrMesh* pMesh;
			if(AiToRtrMeshId.find(AiId) == AiToRtrMeshId.end())
			{
				// New mesh
				pMesh = new CRtrMesh(pDevice, this, pScene->mMeshes[AiId]);
				m_Meshes.push_back(pMesh);
			}
			else
			{
				pMesh = m_Meshes[AiToRtrMeshId[AiId]];
			}
			if(!pMesh)
			{
				assert(0);
				return false;
			}
			DrawNode.pMeshes.push_back(pMesh);
		}

		// Init the transformation
		aiMatrix4x4 Transform = pCurrnet->mTransformation;
		const aiNode* pParent = pCurrnet->mParent;
		while(pParent)
		{
			Transform *= pParent->mTransformation;
			pParent = pParent->mParent;
		}

		DrawNode.Transformation = aiMatToD3D(Transform).Invert();
		m_DrawList.push_back(DrawNode);
	}

	bool b = true;
	// visit the children
	for(UINT i = 0; i < pCurrnet->mNumChildren; i++)
	{
		b |= ParseAiSceneNode(pCurrnet->mChildren[i], pScene, pDevice, AiToRtrMeshId);
	}
	return b;
}

bool CRtrModel::CreateDrawList(const aiScene* pScene, ID3D11Device* pDevice)
{
	// First create bones
	m_pBones = std::make_unique<CRtrBones>(pScene);

	std::map<UINT, UINT> AiToRtrMeshId;
	aiNode* pRoot = pScene->mRootNode;
	return ParseAiSceneNode(pRoot, pScene, pDevice, AiToRtrMeshId);
}

void CRtrModel::CalculateModelProperties()
{
	m_VertexCount = 0;
	m_PrimitiveCount = 0;

	RTR_BOX_F BoundingBox;
	for(const auto Node : m_DrawList)
	{
		for(const auto pMesh : Node.pMeshes)
		{
			const RTR_BOX_F& MeshBox = pMesh->GetBoundingBox();
			RTR_BOX_F TransformedBox = MeshBox.Transform(Node.Transformation);

            BoundingBox.Min = float3::Min(BoundingBox.Min, TransformedBox.Min);
            BoundingBox.Max = float3::Max(BoundingBox.Max, TransformedBox.Max);

			m_VertexCount += pMesh->GetVertexCount();
			m_PrimitiveCount += pMesh->GetPrimiveCount();
		}
	}

	m_Center = (BoundingBox.Max + BoundingBox.Min) * 0.5f;
	float3 distMax = BoundingBox.Max - m_Center;
	m_Radius = distMax.Length();
}

void CRtrModel::Animate()
{
	m_pBones->Animate();
}