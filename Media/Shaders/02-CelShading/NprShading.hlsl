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

    Filname: NprShading.hlsl
---------------------------------------------------------------------------
*/

cbuffer cbCommonPerFrame : register(b0)
{
	matrix gVPMat;
	float3 gLightPosW;
	float3 gLightIntensity;
}

cbuffer cbPerMesh : register(b1)
{
	matrix gWorld;
}

cbuffer cbGooch : register(b2)
{
	float3 gColdColor;
	float  gColdDiffuseFactor;
	float3 gWarmColor;
	float  gWarmDiffuseFactor;
}

cbuffer cbTwoTone : register(b2)
{
	float gShadowThreshold;
	float gShadowFactor;
	float gLightFactor;
}

cbuffer cbPencil : register(b2)
{
	int bVisualizeLayers;
}

Texture2D gAlbedo : register (t0);
SamplerState gLinearSampler : register(s0);
Texture2D gBackground : register(t0);
Texture2D gPencilStrokes[4] : register(t1);

struct VS_IN
{
	float4 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD;
};

struct VS_OUT
{
	float4 svPos : SV_POSITION;
    float3 PosW : POSITION;
	float2 TexC : TEXCOORD;
	float3 NormalW : NORMAL;
};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;
    float4 PosW = mul(vIn.PosL, gWorld);
    vOut.PosW = PosW.xyz;
	vOut.svPos = mul(PosW, gVPMat);
	vOut.TexC = vIn.TexC;
	vOut.NormalW = mul(float4(vIn.NormalL, 0), gWorld).xyz;
	return vOut;
}

float CalculateSpecIntensity(float3 PosW, float3 NormalW)
{
    float3 LightDirW = normalize(gLightPosW - PosW);
    float3 H = normalize(LightDirW + NormalW);
    float NdotH = max(0, dot(NormalW, H));
    float Intensity = pow(NdotH, 1000);
    return Intensity;
}

float4 BasicDiffusePS(VS_OUT vOut) : SV_TARGET
{
	float3 n = normalize(vOut.NormalW);
    // Light dir
    float3 LightDir = gLightPosW - vOut.PosW;
    LightDir = normalize(LightDir);

    //Diffuse term
    float NdotL = max(0, dot(n, LightDir));
    float3 IntensityD = NdotL * gLightIntensity;

    float3 c = gAlbedo.Sample(gLinearSampler, vOut.TexC).xyz;
    c = c * IntensityD + CalculateSpecIntensity(vOut.PosW, n);
	return float4(c, 1);
}

float4 GoochShadingPS(VS_OUT vOut) : SV_TARGET
{
    float3 LightDir = normalize(gLightPosW - vOut.PosW);

    float3 diffuse = gAlbedo.Sample(gLinearSampler, vOut.TexC).xyz;

	float3 Cool = gColdDiffuseFactor * diffuse + gColdColor;
	float3 Warm = gWarmDiffuseFactor * diffuse + gWarmColor;

    float3 N = normalize(vOut.NormalW);
    float NdotL = dot(N, LightDir);
    float factor = (1 + NdotL) * 0.5;
    float3 c = factor*Warm + (1 - factor)*Cool;
    c = c + CalculateSpecIntensity(vOut.PosW, N).xxx;
    return float4(c, 1);
}

float4 TwoTonePS(VS_OUT vOut) : SV_TARGET
{
	float3 LightDir = normalize(gLightPosW - vOut.PosW);
	float3 diffuse = gAlbedo.Sample(gLinearSampler, vOut.TexC).xyz;
	float3 N = normalize(vOut.NormalW);
	float NdotL = saturate(dot(N, LightDir));
	NdotL = NdotL < gShadowThreshold ? gShadowFactor : gLightFactor;

	float3 c = NdotL * gLightIntensity * diffuse;

	return float4(c, 1);
}

float4 BackgroundPS(float2 TexC : TEXCOORD) : SV_TARGET
{
	return gBackground.Sample(gLinearSampler, TexC);
}

float4 PencilPS(VS_OUT vOut) : SV_TARGET
{
	float3 LightDir = normalize(gLightPosW - vOut.PosW);
	float3 N = normalize(vOut.NormalW);
	float NdotL = 1 - saturate(dot(N, LightDir));
	float2 TexC = vOut.TexC * float2(10, 15);

	if(NdotL > 0.7)
	{
		return gPencilStrokes[0].Sample(gLinearSampler, TexC) * (bVisualizeLayers ? float4(1, 0, 0, 1) : float(1).xxxx);
	}
	else if(NdotL > 0.4)
	{
		return gPencilStrokes[1].Sample(gLinearSampler, TexC)* (bVisualizeLayers ? float4(0, 1, 0, 1) : float(1).xxxx);
	}
	else if(NdotL > 0.05)
	{
		return gPencilStrokes[2].Sample(gLinearSampler, TexC)* (bVisualizeLayers ? float4(0, 0, 1, 1) : float(1).xxxx);
	}
	else
	{
		return gPencilStrokes[3].Sample(gLinearSampler, TexC);
	}
}
