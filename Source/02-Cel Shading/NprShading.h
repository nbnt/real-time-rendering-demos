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

Filename: NprShading.h
---------------------------------------------------------------------------
*/
#pragma once
#include "Common.h"
#include "ShaderUtils.h"

class CRtrModel;
class CRtrMesh;
class CFullScreenPass;

/* Resources:
http://artis.imag.fr/~Cyril.Soler/DEA/NonPhotoRealisticRendering/Papers/col0300.pdf
http://developer.amd.com/wordpress/media/2012/10/ShaderX_NPR.pdf
http://www.cs.ucr.edu/~vbz/cs230papers/x-toon.pdf
http://gfx.cs.princeton.edu/gfx/pubs/Rusinkiewicz_2006_ESF/exaggerated_shading.pdf
*/

class CNprShading
{
public:
    enum SHADING_MODE : UINT
    {
		BLINN_PHONG,
        GOOCH_SHADING,
		TWO_TONE_SHADING,
		NDOTL_PENCIL_SHADING,
		LUMINANCE_PENCIL_SHADING,
    };

	struct SCommonSettings
	{
		float4x4 VpMat;
		float3 LightPosW = float3(1, -1, 1);
		float Pad0;
		float3 LightIntensity = float3(1, 1, 1);
		float Pad1;
	};
	verify_cb_size_alignment(SCommonSettings);

	struct SGoochSettings
	{
		float3 ColdColor = float3(0.33f, 0, 0.4f);
		float ColdDiffuseFactor = 0.1f;
		float3 WarmColor = float3(0.2f, 0.15f, 0);
		float WarmDiffuseFactor = 0.7f;
	};
	verify_cb_size_alignment(SGoochSettings);

	struct STwoToneSettings
	{
		float ShadowThreshold  = 0.5f;
		float ShadowFactor		= 0.1f;
		float LightFactor		= 0.75f;
		float pad0;
	};
	verify_cb_size_alignment(SGoochSettings);

	struct SPencilSettings
	{
		bool bVisualizeLayers = 0;
		int pad[3];
	};
	verify_cb_size_alignment(SPencilSettings);

	struct SDrawSettings
	{
		SHADING_MODE Mode = BLINN_PHONG;
		SCommonSettings Common;
		SGoochSettings Gooch;
		STwoToneSettings HardShading;
		SPencilSettings Pencil;
	};


	CNprShading(ID3D11Device* pDevice, const CFullScreenPass* pFullScreenPass);

    void DrawModel(ID3D11DeviceContext* pCtx, const CRtrModel* pModel);
	void PrepareForDraw(ID3D11DeviceContext* pCtx, const SDrawSettings& DrawSettings);

private:
    void DrawMesh(const CRtrMesh* pMesh, ID3D11DeviceContext* pCtx, const float4x4& WorldMat);
	void DrawPencilBackground(ID3D11DeviceContext* pCtx);

	// Common
	CVertexShaderPtr m_VS;
    CPixelShaderPtr  m_BasicDiffusePS;

	ID3D11BufferPtr m_PerFrameCB;
	ID3D11BufferPtr m_PerMeshCB;
	ID3D11SamplerStatePtr m_pLinearSampler;

	// Gooch - http://artis.imag.fr/~Cyril.Soler/DEA/NonPhotoRealisticRendering/Papers/p447-gooch.pdf
	CPixelShaderPtr  m_GoochPS;
	ID3D11BufferPtr m_GoochCB;

	// Two Tone (Hard Shading) - http://markmark.net/npar/npar2000_lake_et_al.pdf
	CPixelShaderPtr m_TwoTonePS;
	ID3D11BufferPtr m_TwoToneCB;
	SHADING_MODE m_Mode;


	// Pencil shader
	const CFullScreenPass* m_pFullScreenPass;
	ID3D11BufferPtr m_PencilCb;
	CPixelShaderPtr m_BackgroundPS;
	CPixelShaderPtr m_LuminancePencilPS;
	CPixelShaderPtr m_NdotLPencilPS;
	ID3D11ShaderResourceViewPtr m_BackgroundSRV;
	ID3D11ShaderResourceViewPtr m_PencilSRV[4];
	

	struct SPerMeshData
	{
		float4x4 World;
	};
	verify_cb_size_alignment(SPerMeshData);
};