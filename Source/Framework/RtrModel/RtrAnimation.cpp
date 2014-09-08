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
	m_TicksPerSecond = pAiAnimation->mTicksPerSecond ? float(pAiAnimation->mTicksPerSecond) : 25;
	
	m_BoneKeys.resize(pAiAnimation->mNumChannels);
	for(UINT i = 0; i < pAiAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* pAiNode = pAiAnimation->mChannels[i];
		m_BoneKeys[i].BoneID = pAnimationController->GetBoneIdFromName(pAiNode->mNodeName.C_Str());
		
		for(UINT j = 0; j < pAiNode->mNumPositionKeys; j++)
		{
			const aiVectorKey& Key = pAiNode->mPositionKeys[j];
			SAnimationKey<float3> Position;
			Position.Value = float3(Key.mValue.x, Key.mValue.y, Key.mValue.z);
			Position.Time = float(Key.mTime);
			m_BoneKeys[i].PositionKeys.Keys.push_back(Position);
		}

		for(UINT j = 0; j < pAiNode->mNumScalingKeys; j++)
		{
			const aiVectorKey& Key = pAiNode->mScalingKeys[j];
			SAnimationKey<float3> Scale;
			Scale.Value = float3(Key.mValue.x, Key.mValue.y, Key.mValue.z);
			Scale.Time = float(Key.mTime);
			m_BoneKeys[i].ScalingKeys.Keys.push_back(Scale);
		}

		for(UINT j = 0; j < pAiNode->mNumRotationKeys; j++)
		{
			const aiQuatKey& Key = pAiNode->mRotationKeys[j];
			SAnimationKey<quaternion> Rotation;
			Rotation.Value = quaternion(Key.mValue.x, Key.mValue.y, Key.mValue.z, Key.mValue.w);
			Rotation.Time = float(Key.mTime);
			m_BoneKeys[i].RotationKey.Keys.push_back(Rotation);
		}
	}
}

template<typename T>
UINT FindCurrentFrame(T BoneKey, float Ticks, float LastUpdateTime)
{
	UINT CurFrame = BoneKey.LastKey;
	while(CurFrame < BoneKey.Keys.size() - 1)
	{
		if(BoneKey.Keys[CurFrame + 1].Time > Ticks)
		{
			break;
		}
		CurFrame++;
	}
	return CurFrame;
}

float3 Interpolate(const float3& Start, const float3& End, float Ratio)
{
	return Start + ((End - Start) * Ratio);
}

quaternion Interpolate(const quaternion& Start, const quaternion& End, float Ratio)
{
	return quaternion::Slerp(Start, End, Ratio);
}

template<typename _KeyType>
_KeyType CRtrAnimation::CalcCurrentKey(SKeyData<_KeyType>& BoneKeys, float Ticks, float LastUpdateTime)
{
	_KeyType CurValue;
	if(BoneKeys.Keys.size() > 0)
	{
		if(Ticks < LastUpdateTime)
		{
			BoneKeys.LastKey = 0;
		}
	
		// search for the next keyframe
		UINT CurFrame = FindCurrentFrame(BoneKeys, Ticks, LastUpdateTime);
		UINT NextFrame = (CurFrame + 1) % BoneKeys.Keys.size();
		const SAnimationKey<_KeyType>& CurKey = BoneKeys.Keys[CurFrame];
		const SAnimationKey<_KeyType>& NextKey = BoneKeys.Keys[NextFrame];

		assert(Ticks >= CurKey.Time);
		// Interpolate between them
		float diff = NextKey.Time - CurKey.Time;
		if(diff < 0)
		{
			diff += m_Duration;
		}
		else if(diff == 0)
		{
			CurValue = CurKey.Value;
		}
		else
		{
			float ratio = (Ticks - CurKey.Time) / diff;
			CurValue = Interpolate(CurKey.Value, NextKey.Value, ratio);
		}
		BoneKeys.LastKey = CurFrame;
	}
	return CurValue;
}

void CRtrAnimation::Animate(float TotalTime, CRtrAnimationController* pAnimationController)
{
	// Calculate the relative time
	float Ticks = fmod(TotalTime * m_TicksPerSecond, m_Duration);

	for(auto& Key : m_BoneKeys)
	{
		float4x4 Translation = float4x4::CreateTranslation(CalcCurrentKey(Key.PositionKeys, Ticks, Key.LastUpdateTime));
		float4x4 Scaling = float4x4::CreateScale(CalcCurrentKey(Key.ScalingKeys, Ticks, Key.LastUpdateTime));
 		float4x4 Rotation = float4x4::CreateFromQuaternion(CalcCurrentKey(Key.RotationKey, Ticks, Key.LastUpdateTime));

		Key.LastUpdateTime = Ticks;

		float4x4 T = Scaling * Rotation * Translation;
		pAnimationController->SetBoneLocalTransform(Key.BoneID, T);
	}
}