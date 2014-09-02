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

Filename: Transparency.hlsl
---------------------------------------------------------------------------*/

cbuffer cbPerModel : register(b0)
{
	matrix gWVP;
	matrix gWorld;
	float3 gLightDirW;
	float3 gLightIntensity;
	float gAlphaOut;
};

cbuffer cbPerMesh : register(b1)
{
    float3 gMeshColor;
}

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormalL	 : NORMAL;
};

struct VS_OUT
{
	float4 svPos : SV_POSITION;
	float3 NormalW : NORMAL;
};

VS_OUT VS(VS_INPUT vIn)
{
	VS_OUT vOut;
	vOut.svPos = mul(vIn.PosL, gWVP);
	vOut.NormalW = mul(float4(vIn.NormalL, 0), gWorld).xyz;
	return vOut;
}

float3 PSCalcColor(VS_OUT vOut, const bool bForTransparency) : SV_TARGET
{
	// Need to normalize the normal
	float3 NormalW = normalize(vOut.NormalW);
	float NdotL = dot(NormalW, gLightDirW);
	if(bForTransparency)
	{
		NdotL = abs(NdotL);
	}
	else
	{
		NdotL = saturate(NdotL);
	}


	float3 Color = gMeshColor * (NdotL + 0.01f) * gLightIntensity;
	return Color;
}

float4 SolidPS(VS_OUT vOut) : SV_TARGET
{
	return float4(PSCalcColor(vOut, false), 1);
}

float4 UnorderedBlendPS(VS_OUT vOut) : SV_TARGET
{
	return float4(PSCalcColor(vOut, true), gAlphaOut);
}