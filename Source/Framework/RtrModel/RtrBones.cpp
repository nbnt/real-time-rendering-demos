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

Filename: RtrBones.cpp
---------------------------------------------------------------------------*/
#include "RtrBones.h"
#include "..\RtrModel.h"
#include "scene.h"
#include <fstream>
#include <algorithm>

void DumpBonesHeirarchy(const std::string& filename, SBoneDesc* pBone, UINT count)
{
	std::ofstream dotfile;
	dotfile.open(filename.c_str(), 'w');

	// Header
	dotfile << "digraph BonesGraph {" << std::endl;

	for(UINT i = 0; i < count; i++)
	{
		const SBoneDesc& bone = pBone[i];
		if(bone.ParentID != INVALID_BONE_ID)
		{
			std::string Parent = pBone[bone.ParentID].Name;
			std::string Me = bone.Name;
			std::replace(Parent.begin(), Parent.end(), '.', '_');
			std::replace(Me.begin(), Me.end(), '.', '_');

			dotfile << Parent << " -> " << Me << std::endl;
		}
	}

	// Close the file
	dotfile << "}" << std::endl; // closing graph scope
	dotfile.close();
}

bool VerifyBoneNameUniquness(const aiNode* pNode, CRtrBones::string_2_uint_map& Names)
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
		b &= VerifyBoneNameUniquness(pNode->mChildren[i], Names);
	}
	return b;
}


CRtrBones::CRtrBones(const aiScene* pScene)
{
	// First, go over the meshes and file all the bones that are used
	for(UINT i = 0; i < pScene->mNumMeshes; i++)
	{
		const aiMesh* pMesh = pScene->mMeshes[i];
		if(pMesh->HasBones())
		{
			for(UINT j = 0; j < pMesh->mNumBones; j++)
			{
				m_BoneNameToID[pMesh->mBones[j]->mName.C_Str()] = INVALID_BONE_ID;
			}
		}
	}
	m_BonesCount = 0;
	if(m_BoneNameToID.size() != 0)
	{
		// Make sure we have unique names in the scene. The bones algorithm relies on unique names
		string_2_uint_map tempmap;
		assert(VerifyBoneNameUniquness(pScene->mRootNode, tempmap));

		// For every bone used, all its ancestors are bones too. Mark them
		string_2_uint_map::iterator it = m_BoneNameToID.begin();
		while(it != m_BoneNameToID.end())
		{
			aiNode* pCurNode = pScene->mRootNode->FindNode(it->first.c_str());
			while(pCurNode)
			{
				m_BoneNameToID[pCurNode->mName.C_Str()] = INVALID_BONE_ID;
				pCurNode = pCurNode->mParent;
			}
			it++;
		}
		m_BonesCount = m_BoneNameToID.size();
		m_Bones.resize(m_BonesCount);
		m_TempMatrices.resize(m_BonesCount);
		m_BonesTransform.resize(m_BonesCount);

		UINT bonesCount = InitBone(pScene->mRootNode, INVALID_BONE_ID, 0);
		_Unreferenced_parameter_(bonesCount);
		assert(m_BonesCount == bonesCount);
		//		DumpBonesHeirarchy("bones.dot", m_Bones.get(), m_BonesCount);
	}
}

UINT CRtrBones::InitBone(const aiNode* pCurNode, UINT ParentID, UINT BoneID)
{
	assert(m_BoneNameToID.find(pCurNode->mName.C_Str()) != m_BoneNameToID.end());
	assert(pCurNode->mNumMeshes == 0);
	m_BoneNameToID[pCurNode->mName.C_Str()] = BoneID;

	assert(BoneID < m_BonesCount);
	SBoneDesc& Bone = m_Bones[BoneID];
	Bone.Name = pCurNode->mName.C_Str();
	float4x4 Matrix = aiMatToD3D(pCurNode->mTransformation);
	Matrix.Transpose(Bone.Matrix);
	Bone.Matrix.Invert(Bone.InvMatrix);

	Bone.ParentID = ParentID;
	Bone.BoneID = BoneID;

	BoneID++;

	for(UINT i = 0; i < pCurNode->mNumChildren; i++)
	{
		// Check that the child is actually used
		if(m_BoneNameToID.find(pCurNode->mChildren[i]->mName.C_Str()) != m_BoneNameToID.end())
		{
			BoneID = InitBone(pCurNode->mChildren[i], Bone.BoneID, BoneID);
		}
	}

	return BoneID;
}

void CRtrBones::Animate()
{
	for(UINT i = 0; i < m_BonesCount; i++)
	{
		m_BonesTransform[i] = float4x4::Identity();
	}

	for(UINT i = 0; i < m_BonesCount; i++)
	{
		float4x4 B = m_Bones[i].Matrix;

		if(m_Bones[i].ParentID != INVALID_BONE_ID)
		{
			B = B * m_BonesTransform[m_Bones[i].ParentID];
		}
		m_BonesTransform[i] = B;
	}
}