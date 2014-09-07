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

Filename: RtrBones.h
---------------------------------------------------------------------------*/
#pragma once
#include "..\Common.h"
#include <map>
#include <vector>

struct aiScene;
struct aiNode;

#define INVALID_BONE_ID UINT(-1)

struct SBoneDesc
{
	SBoneDesc() { ZeroMemory(this, sizeof(this)); }
	std::string Name;
	UINT BoneID;
	UINT ParentID;
	float4x4 Matrix;
	float4x4 InvMatrix;
};

class CRtrBones
{
public:
	CRtrBones(const aiScene* pScene);
	using string_2_uint_map = std::map < std::string, UINT > ;

	UINT GetCount() const { return m_BonesCount; }
	void Animate();
	const SBoneDesc& GetBoneDesc(UINT BoneID) const { return m_Bones[BoneID]; }
	const float4x4* GetModelTransform() const { return &m_ModelTransform[0]; }
	const float4x4* GetSkeletonTransform() const { return &m_SkeletonTransform[0]; }
	UINT GetIdFromName(const std::string& Name) const;
private:
	UINT InitBone(const aiNode* pCurNode, UINT ParentID, UINT BoneID);

	UINT m_BonesCount;
	std::vector<SBoneDesc> m_Bones;
	std::vector<float4x4> m_SkeletonTransform;
	std::vector<float4x4> m_ModelTransform;
	struct STempBonesData
	{
		float4x4 BindPose;
		float4x4 Transform;
	};

	std::vector<STempBonesData> m_TempMatrices;
	string_2_uint_map m_BoneNameToID;
};