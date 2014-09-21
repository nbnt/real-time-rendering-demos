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

Filename: BrdfShader.h
---------------------------------------------------------------------------
*/
#pragma once
#include "Common.h"
#include "ShaderUtils.h"

class CRtrModel;
class CRtrMesh;

class CBrdfShader
{
public:
    enum BRDF_MODEL : UINT
    {
        NO_BRDF,
        PHONG,
        BLINN_PHONG,

        BRDF_COUNT
    };

    struct SPerFrameData
    {
        float4x4 VPMat;
        float3 LightPosW = float3(-2.5f, 5, -5);;
        float  CutoffScale;
        float3 LightIntensity = float3(1, 1, 1);;
        float  CutoffOffset;
        float3 AmbientIntensity = float3(0.005f, 0.01f, 0.01f);
        float  SpecPower = 15;
        float3 CameraPosW;
        float pad0;
        float3 SpecularColor = float3(1, 1, 1);
        float pad1;
        float3 DiffuseColor = float3(0.3f, 1, 0.5f);
        float pad2;
    };
    verify_cb_size_alignment(SPerFrameData);

    CBrdfShader(ID3D11Device* pDevice);
    void DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel);
	void PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameData& PerFrameData, BRDF_MODEL BrdfMode);

private:
    void DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat);

	CVertexShaderPtr m_VS;
    CPixelShaderPtr  m_NoSpecPS;
	CPixelShaderPtr  m_PhongPS;
    CPixelShaderPtr  m_BlinnPhongPS;

	ID3D11BufferPtr m_PerFrameCb;
	ID3D11BufferPtr m_PerModelCb;

	float3 m_LightDir;
	float3 m_LightIntensity;

	struct SPerMeshData
	{
		float4x4 World;
	};
	verify_cb_size_alignment(SPerMeshData);
};