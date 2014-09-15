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

    Filname: BrdfShader.hlsl
---------------------------------------------------------------------------
*/
cbuffer cbPeFrame : register(b0)
{
	matrix gVPMat;
	float3 gLightPosW;
    float  gDiffuseEnabled;
    float3 gLightIntensity;
    float  gAmbientEnabled;
    float3 gAmbientIntensity;
    float  gSpecularEnabled;
    float3 gCameraPosW;
    float  gCutoffStart;
    float  gCutoffEnd;
    float3 gSpecularColor;
    float gShininess;
    float3 gDiffuseColor;
}

cbuffer cbPerMesh : register(b1)
{
	matrix gWorld;
}

struct VS_IN
{
	float4 PosL : POSITION;
	float3 NormalL : NORMAL;
};

struct VS_OUT
{
	float4 svPos : SV_POSITION;
    float3 PosW  : POSITION;
	float3 NormalW : NORMAL;
};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;
    vOut.PosW = mul(vIn.PosL, gWorld).xyz;
	vOut.svPos = mul(float4(vOut.PosW, 1), gVPMat);
	vOut.NormalW = mul(float4(vIn.NormalL, 0), gWorld).xyz;
	return vOut;
}

float3 PhongSpec(float3 LightDir, float3 Normal, float3 PosW)
{
    float3 R = reflect(-LightDir, Normal);
    float3 V = normalize(gCameraPosW - PosW);
    float VdotR = saturate(dot(R, V));
    float I = pow(VdotR, gShininess);
    float3 Specular = gSpecularColor * I * gSpecularEnabled;
    return Specular;
}

float3 BlinnPhongSpec(float3 LightDir, float3 Normal, float3 PosW)
{
    // Calculate the half-vector
    float3 V = normalize(gCameraPosW - PosW);
    float3 H = normalize(LightDir + V);
    
    float NdotH = saturate(dot(H, Normal));
    NdotH = pow(NdotH, gShininess);
    float3 Specular = NdotH * gSpecularColor;
    return Specular;
}

float3 CalculateLightIntensity(float3 PosW)
{
    float dist = length(gLightPosW - PosW);
    float cutoff;
    if(dist <= gCutoffStart)
    {
        cutoff = 1;
    }
    else if(dist >= gCutoffEnd)
    {
        cutoff = 0;
    }
    else
    {
        cutoff = (gCutoffEnd - dist) / (gCutoffEnd - gCutoffStart);
    }

    return cutoff * gLightIntensity;
}

#define NO_SPEC             0
#define PHONG_SPEC          1
#define BLINN_PHONG_SPEC    2

float4 PSCommon(VS_OUT vOut, uint mode)
{
    // Calculate diffuse term
    float3 LightDir = normalize(gLightPosW - vOut.PosW);
	float3 n = normalize(vOut.NormalW);
    float NdotL = max(0, dot(n, LightDir));
    float3 Diffuse = NdotL * gDiffuseColor * gDiffuseEnabled;
    
    float3 Specular = float3(0, 0, 0);
    switch(mode)
    {
    case PHONG_SPEC:
        Specular = PhongSpec(LightDir, n, vOut.PosW);
        break;
    case BLINN_PHONG_SPEC:
        Specular = BlinnPhongSpec(LightDir, n, vOut.PosW);
        break;
    }

    float3 Intensity = Diffuse + Specular;
    // Apply attenuation

    float3 c = (Diffuse + Specular) * CalculateLightIntensity(vOut.PosW);
    // Add ambient term
    c += gAmbientIntensity * gAmbientEnabled;

    return float4(c, 1);
}

float4 NoSpecPS(VS_OUT vOut) : SV_TARGET
{
    return PSCommon(vOut, NO_SPEC);
}

float4 PhongPS(VS_OUT vOut) : SV_TARGET
{
    return PSCommon(vOut, PHONG_SPEC);
}

float4 BlinnPhongPS(VS_OUT vOut) : SV_TARGET
{
    return PSCommon(vOut, BLINN_PHONG_SPEC);
}