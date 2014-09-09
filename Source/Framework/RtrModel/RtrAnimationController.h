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

Filename: RtrAnimationController.h
---------------------------------------------------------------------------*/
#pragma once
#include "..\Common.h"
#include <map>
#include <vector>
#include "RtrAnimation.h"

struct aiScene;
struct aiNode;
class CRtrModel;
struct SRtrBone;

#define INVALID_BONE_ID UINT(-1)
#define BIND_POSE_ANIMATION_ID UINT(-1)

struct SRtrBone
{
    UINT ParentID;
    UINT BoneID;
    std::string Name;
    float4x4 Offset;
    float4x4 LocalTransform;
    float4x4 OriginalLocalTransform;
    float4x4 GlobalTransform;
};

class CRtrAnimationController
{
public:
	CRtrAnimationController(const aiScene* pScene);
    void Animate(float ElapsedTime);
	void Reset() { m_TotalTime = 0; }

    UINT GetAnimationsCount() const { return m_Animations.size(); }
    const std::string& GetAnimationName(UINT ID) const { return m_Animations[ID]->GetName();}
    void SetActiveAnimation(UINT ID);

    const float4x4* GetBonesMatrices() const { return &m_BoneTransforms[0]; }
    UINT GetBonesCount() const {return m_BonesCount;}

    UINT GetBoneIdFromName(const std::string& Name) const;
	void SetBoneLocalTransform(UINT BoneID, const float4x4& Transform);
private:
    std::map<std::string, UINT> m_BoneNameToIdMap;
    std::vector<SRtrBone> m_Bones;
    std::vector<float4x4> m_BoneTransforms;
	std::vector<std::unique_ptr<CRtrAnimation>> m_Animations;

    UINT m_BonesCount = 0;
	float m_TotalTime = 0;
    UINT m_ActiveAnimation = BIND_POSE_ANIMATION_ID;

    void InitializeBones(const aiScene* pScene);
    UINT InitBone(const aiNode* pNode, UINT ParentID, UINT BoneID);
    void InitializeBonesOffsetMatrices(const aiScene* pScene);

    void CalculateBoneTransforms();
};