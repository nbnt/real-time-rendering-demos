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

Filename: ToonShader.h
---------------------------------------------------------------------------
*/
#pragma once
#include "Common.h"
#include "ShaderUtils.h"

class CRtrModel;
class CRtrMesh;

/* Resources:
http://artis.imag.fr/~Cyril.Soler/DEA/NonPhotoRealisticRendering/Papers/p447-gooch.pdf
http://markmark.net/npar/npar2000_lake_et_al.pdf
http://artis.imag.fr/~Cyril.Soler/DEA/NonPhotoRealisticRendering/Papers/col0300.pdf
http://developer.amd.com/wordpress/media/2012/10/ShaderX_NPR.pdf
http://www.cs.ucr.edu/~vbz/cs230papers/x-toon.pdf
http://gfx.cs.princeton.edu/gfx/pubs/Rusinkiewicz_2006_ESF/exaggerated_shading.pdf
*/

class CToonShader
{
public:
    enum SHADING_MODE : UINT
    {
        BASIC_DIFFUSE,
        GOOCH_SHADING,
    };

	struct SPerFrameData
	{
		float4x4 VpMat;
		float3 LightPosW;
		float pad0;
		float3 LightIntensity;
        float pad1;
};
	verify_cb_size_alignment(SPerFrameData);

	CToonShader(ID3D11Device* pDevice);
    void DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel);
	void PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameData& PerFrameData, SHADING_MODE Mode);

private:
    void DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat);

	SVertexShaderPtr m_VS;
    SPixelShaderPtr  m_BasicDiffusePS;
	SPixelShaderPtr  m_GoochPS;

	ID3D11BufferPtr m_PerFrameCb;
	ID3D11BufferPtr m_PerModelCb;
	ID3D11SamplerStatePtr m_pLinearSampler;
    ID3D11RasterizerStatePtr m_CullFrontFaceRS;

	float3 m_LightDir;
	float3 m_LightIntensity;

	struct SPerMeshData
	{
		float4x4 World;
	};
	verify_cb_size_alignment(SPerMeshData);
};