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

Filename: RtrAnimationController.cpp
---------------------------------------------------------------------------*/
#include "RtrAnimationController.h"
#include "scene.h"
#include "..\RtrModel.h"
#include <fstream>

void DumpBonesHeirarchy(const std::string& filename, SRtrBone* pBone, UINT count)
{
    std::ofstream dotfile;
    dotfile.open(filename.c_str(), 'w');

    // Header
    dotfile << "digraph BonesGraph {" << std::endl;

    for(UINT i = 0; i < count; i++)
    {
        const SRtrBone& bone = pBone[i];
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

CRtrAnimationController::CRtrAnimationController(const aiScene* pScene)
{
    if(pScene->HasAnimations())
	{
		InitializeBones(pScene);
		m_Animations.resize(pScene->mNumAnimations);
		for(UINT i = 0; i < pScene->mNumAnimations; i++)
		{
			m_Animations[i] = std::make_unique<CRtrAnimation>(pScene->mAnimations[i], this);
		}
	}
}

void CRtrAnimationController::InitializeBones(const aiScene* pScene)
{
    // Go over all the meshes, and find the bones that are being used
    for(UINT i = 0; i < pScene->mNumMeshes; i++)
    {
        const aiMesh* pMesh = pScene->mMeshes[i];
        if(pMesh->HasBones())
        {
            for(UINT j = 0; j < pMesh->mNumBones; j++)
            {
                m_BoneNameToIdMap[pMesh->mBones[j]->mName.C_Str()] = INVALID_BONE_ID;
            }
        }
    }

    if(m_BoneNameToIdMap.size() != 0)
    {
        // For every bone used, all its ancestors are bones too. Mark them
        auto it = m_BoneNameToIdMap.begin();
        while(it != m_BoneNameToIdMap.end())
        {
            aiNode* pCurNode = pScene->mRootNode->FindNode(it->first.c_str());
            while(pCurNode)
            {
                m_BoneNameToIdMap[pCurNode->mName.C_Str()] = INVALID_BONE_ID;
                pCurNode = pCurNode->mParent;
            }
            it++;
        }

        // Now create the hierarchy
        m_BonesCount = UINT(m_BoneNameToIdMap.size());
        m_Bones.resize(m_BonesCount);
        UINT bonesCount = InitBone(pScene->mRootNode, INVALID_BONE_ID, 0);
        _Unreferenced_parameter_(bonesCount);
        assert(m_BonesCount == bonesCount);
//        DumpBonesHeirarchy("bones.dot", &m_Bones[0], m_BonesCount);

        InitializeBonesOffsetMatrices(pScene);

        m_BoneTransforms.resize(m_BonesCount);
    }
}

UINT CRtrAnimationController::InitBone(const aiNode* pCurNode, UINT ParentID, UINT BoneID)
{
    assert(m_BoneNameToIdMap.find(pCurNode->mName.C_Str()) != m_BoneNameToIdMap.end());
    assert(pCurNode->mNumMeshes == 0);
    m_BoneNameToIdMap[pCurNode->mName.C_Str()] = BoneID;

    assert(BoneID < m_BonesCount);
    SRtrBone& Bone = m_Bones[BoneID];
    Bone.Name = pCurNode->mName.C_Str();
    Bone.ParentID = ParentID;
    Bone.BoneID = BoneID;
    Bone.LocalTransform = aiMatToD3D(pCurNode->mTransformation);
    Bone.OriginalLocalTransform = Bone.LocalTransform;
    Bone.GlobalTransform = Bone.LocalTransform;

    if(ParentID != INVALID_BONE_ID)
    {
        Bone.GlobalTransform *= m_Bones[ParentID].GlobalTransform;
    }

    BoneID++;

    for(UINT i = 0; i < pCurNode->mNumChildren; i++)
    {
        // Check that the child is actually used
        if(m_BoneNameToIdMap.find(pCurNode->mChildren[i]->mName.C_Str()) != m_BoneNameToIdMap.end())
        {
            BoneID = InitBone(pCurNode->mChildren[i], Bone.BoneID, BoneID);
        }
    }
    return BoneID;
}

void CRtrAnimationController::InitializeBonesOffsetMatrices(const aiScene* pScene)
{
    for(UINT MeshID = 0; MeshID < pScene->mNumMeshes; MeshID++)
    {
        const aiMesh* pAiMesh = pScene->mMeshes[MeshID];

        for(UINT BoneID = 0; BoneID < pAiMesh->mNumBones; BoneID++)
        {
            const aiBone* pAiBone = pAiMesh->mBones[BoneID];
            auto RtrBoneIt = m_BoneNameToIdMap.find(pAiBone->mName.C_Str());
            assert(RtrBoneIt != m_BoneNameToIdMap.end());
            SRtrBone& RtrBone = m_Bones[RtrBoneIt->second];
            RtrBone.Offset = aiMatToD3D(pAiBone->mOffsetMatrix);
        }
    }
}

void CRtrAnimationController::SetBoneLocalTransform(UINT BoneID, const float4x4& Transform)
{
	assert(BoneID < m_BonesCount);
	m_Bones[BoneID].LocalTransform = Transform;
}

void CRtrAnimationController::Animate(float ElapsedTime)
{
    if(m_ActiveAnimation != BIND_POSE_ANIMATION_ID)
    {
        m_TotalTime += ElapsedTime;
        m_Animations[m_ActiveAnimation]->Animate(m_TotalTime, this);
    }

    for(UINT i = 0; i < m_BonesCount; i++)
    {
        m_Bones[i].GlobalTransform = m_Bones[i].LocalTransform;
        if(m_Bones[i].ParentID != INVALID_BONE_ID)
        {
            m_Bones[i].GlobalTransform *= m_Bones[m_Bones[i].ParentID].GlobalTransform;
        }
        m_BoneTransforms[i] = m_Bones[i].Offset * m_Bones[i].GlobalTransform;
    }
}

UINT CRtrAnimationController::GetBoneIdFromName(const std::string& Name) const
{
    const auto a = m_BoneNameToIdMap.find(Name);
    assert(a != m_BoneNameToIdMap.end());
    return a->second;
}

void CRtrAnimationController::SetActiveAnimation(UINT ID)
{
    assert(ID == BIND_POSE_ANIMATION_ID || ID < m_Animations.size());
    m_ActiveAnimation = ID;
    if(ID == BIND_POSE_ANIMATION_ID)
    {
        for(auto& Bone : m_Bones)
        {
            Bone.LocalTransform = Bone.OriginalLocalTransform;
        }
    }
    m_TotalTime = 0;
}