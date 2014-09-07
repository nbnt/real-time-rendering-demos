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

    Filname: Wireframe.hlsl
---------------------------------------------------------------------------
*/
cbuffer cbPeFrame : register(b0)
{
	matrix gVPMat;
	float3 gLightDirW;
	float3 gLightIntensity;
}

cbuffer cbPerMesh : register(b1)
{
	int gbDoubleSided;
	matrix gBones[256];
}

Texture2D gAlbedo : register (t0);
SamplerState gLinearSampler : register(s0);

struct VS_IN
{
	float4 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD;
#ifdef _USE_BONES
	float4 BonesWeights[2] : BONE_WEIGHTS;
	uint4  BonesIDs[2]    : BONE_IDS;
#endif
};

struct VS_OUT
{
	float4 svPos : SV_POSITION;
	float2 TexC : TEXCOORD;
	float3 NormalW : NORMAL;
};

float4x4 CalculateWorldMatrixFromBones(float4 BonesWeights[2], uint4  BonesIDs[2], float4x4 Bones[256])
{
	float4x4 WorldMat = { float4(0, 0, 0, 0), float4(0, 0, 0, 0), float4(0, 0, 0, 0), float4(0, 0, 0, 0) };

		for(int i = 0; i < 2; i++)
		{
			WorldMat += Bones[BonesIDs[i].x] * BonesWeights[i].x;
			WorldMat += Bones[BonesIDs[i].y] * BonesWeights[i].y;
			WorldMat += Bones[BonesIDs[i].z] * BonesWeights[i].z;
			WorldMat += Bones[BonesIDs[i].w] * BonesWeights[i].w;
		}

	return WorldMat;
}

VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;
	float4x4 World;
#ifdef _USE_BONES
	World = CalculateWorldMatrixFromBones(vIn.BonesWeights, vIn.BonesIDs, gBones);
#else
	World = gBones[0];
#endif

	vOut.svPos = mul(mul(vIn.PosL, World), gVPMat);
	vOut.TexC = vIn.TexC;
	vOut.NormalW = mul(float4(vIn.NormalL, 0), World).xyz;
	return vOut;
}

float4 PS(VS_OUT vOut) : SV_TARGET
{
	float3 n = normalize(vOut.NormalW);
    float NdotL = dot(n, -gLightDirW);
    if(gbDoubleSided)
    {
        NdotL = abs(NdotL);
    }
    else
    {
        NdotL = saturate(NdotL);
    }

    float3 Light = NdotL * gLightIntensity;
	float4 c = float4(Light, 1);

#ifdef _USE_TEXTURE
	c *= gAlbedo.Sample(gLinearSampler, vOut.TexC);
#endif
	return c;
}