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

    Filname: BasicLighting.hlsl
---------------------------------------------------------------------------
*/

cbuffer cbPerFrame
{
	matrix gWVPMat;		// WVP matrix
	matrix gWorldMat;
	float3 gNegLightDirW;  // Negative light direction in world space
	float3 gCameraPosW;	
	float3 gDiffuseFactor;
	float3 gSpecFactor;
	float3 gLightColor;
	float  gSurfaceSmoothness;
}

struct VS_INPUT
{
	float4 vPosition : POSITION;
	float3 NormalL	 : NORMAL;
};

struct VS_OUT
{
	float4 svPos : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
};


VS_OUT VSMain(VS_INPUT vIn)
{
	VS_OUT vOut;
	vOut.svPos = mul(vIn.vPosition, gWVPMat);
	vOut.PosW = mul(vIn.vPosition, gWorldMat).xyz;
	vOut.NormalW = mul(float4(vIn.NormalL,0), gWorldMat).xyz;
	return vOut;
}

float4 PSMain(VS_OUT vOut) : SV_TARGET
{
	// Need to normalize the normal
    float3 normalW = normalize(vOut.NormalW);
	float cosTi = saturate(dot(normalW, gNegLightDirW));

	float3 EyeVec = normalize(gCameraPosW - vOut.PosW);
	float3 h = normalize(EyeVec + gNegLightDirW);
	float cosTh = saturate(dot(normalW, h));

	float3 Color = gDiffuseFactor + (gSpecFactor * pow(cosTh, gSurfaceSmoothness));
	Color = Color * gLightColor * cosTi;
	return float4(Color, 1);
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 BasicLighting
{
    pass
    {          
        SetVertexShader( CompileShader( vs_5_0, VSMain() ) );
        SetGeometryShader( NULL );
        SetHullShader( NULL );
        SetDomainShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSMain() ) );
        SetDepthStencilState(NULL, 0);
        SetRasterizerState(NULL);
    }
}