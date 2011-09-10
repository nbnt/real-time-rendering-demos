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

cbuffer cbPerFrame
{
	matrix gWVPMat;		// WVP matrix
	matrix gWorld;
	float3 gLightDirW;  // Light direction in world space
}

Texture2D	gDiffuse : register( t0 );
SamplerState gSamplerLinear : register( s0 );

struct VS_INPUT
{
	float4 vPosition : POSITION;
	float2 TexC		 : TEXCOORD;
	float3 NormalL	 : NORMAL;
};

struct VS_OUT
{
	float4 svPos : SV_POSITION;
	float2 TexC  : TEXCOORD;
	float3 NormalW : NORMAL;
};

VS_OUT VSMain(VS_INPUT vIn)
{
	VS_OUT vOut;
	vOut.svPos = mul(gWVPMat, vIn.vPosition);
	vOut.TexC = vIn.TexC;
	vOut.NormalW = mul(gWorld, float4(vIn.NormalL,0)).xyz;
	return vOut;
}

float4 PSMain(VS_OUT vOut) : SV_TARGET
{
	float3 n = normalize(vOut.NormalW);
	float cosT = dot(n, gLightDirW);
	cosT = max(cosT, 0.1f);
	return float4(cosT, cosT, cosT, 1);
}