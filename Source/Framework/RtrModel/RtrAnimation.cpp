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

Filename: RtrAnimation.cpp
---------------------------------------------------------------------------*/
#include "RtrAnimation.h"
#include "RtrAnimationController.h"
#include "anim.h"

CRtrAnimation::CRtrAnimation(const aiAnimation* pAiAnimation, const CRtrAnimationController* pAnimationController)
{
	assert(pAiAnimation->mNumMeshChannels == 0);
	m_Duration = float(pAiAnimation->mDuration);
	m_Fps = pAiAnimation->mTicksPerSecond ? float(pAiAnimation->mTicksPerSecond) : 25;
	
	m_BoneKeys.resize(pAiAnimation->mNumChannels);
	for(UINT i = 0; i < pAiAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* pAiNode = pAiAnimation->mChannels[i];
		m_BoneKeys[i].BoneID = pAnimationController->GetBoneIdFromName(pAiNode->mNodeName.C_Str());
		
		for(UINT j = 0; j < pAiNode->mNumPositionKeys; j++)
		{
			const aiVector3D Key = pAiNode->mPositionKeys[j].mValue;
			m_BoneKeys[i].m_PositionKeys.push_back(float3(Key.x, Key.y, Key.z));
		}

		for(UINT j = 0; j < pAiNode->mNumScalingKeys; j++)
		{
			const aiVector3D Key = pAiNode->mScalingKeys[j].mValue;
			m_BoneKeys[i].m_ScalingKeys.push_back(float3(Key.x, Key.y, Key.z));
		}

		for(UINT j = 0; j < pAiNode->mNumRotationKeys; j++)
		{
			const aiQuaternion Key = pAiNode->mRotationKeys[j].mValue;
			m_BoneKeys[i].m_RotationKey.push_back(quaternion(Key.x, Key.y, Key.z, Key.w));
		}
	}
}

void CRtrAnimation::Animate(float TotalTime, CRtrAnimationController* pAnimationController)
{
	for(const auto& Key : m_BoneKeys)
	{
		float4x4 Translate = float4x4::CreateTranslation(Key.m_PositionKeys[0]);
		float4x4 Rotation = float4x4::CreateFromQuaternion(Key.m_RotationKey[0]);
		float4x4 Scale = float4x4::CreateScale(Key.m_ScalingKeys[0]);

		float4x4 T = Scale*Rotation*Translate;
		pAnimationController->SetBoneLocalTransform(Key.BoneID, T);
	}
}