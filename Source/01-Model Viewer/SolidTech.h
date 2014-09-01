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

Filename: SolidTech.h
---------------------------------------------------------------------------
*/
#pragma once
#include "Common.h"
#include "DxModel.h"
#include "ShaderUtils.h"

class CSolidTech
{
public:
	struct SPerFrameCb
	{
		float4x4 VpMat;
		float3 LightDirW;
		float pad0;
		float3 LightIntensity;
		float pad1;
	};
	static_assert((sizeof(SPerFrameCb) % 16) == 0, "");

	CSolidTech(ID3D11Device* pDevice, const float3& LightDir, const float3& LightIntesity);
    void DrawModel(const CDxModel* pModel, ID3D11DeviceContext* pCtx);
	void PrepareForDraw(ID3D11DeviceContext* pCtx, const SPerFrameCb& PerFrameData);

	void SetLightIntensity(const float3& LightIntensity) { m_LightIntensity = LightIntensity; }
	void SetLightDirection(const float3& LightDirection) { m_LightDir = LightDirection; }

private:
	void DrawMesh(const CDxMesh* pMesh, ID3D11DeviceContext* pCtx);

	SVertexShaderPtr m_VS;
	SPixelShaderPtr m_TexPS;
	SPixelShaderPtr m_ColorPS;
    ID3D11RasterizerStatePtr m_pRastState;
	ID3D11BufferPtr m_PerFrameCb;
	ID3D11BufferPtr m_PerModelCb;
	ID3D11SamplerStatePtr m_pLinearSampler;

	float3 m_LightDir;
	float3 m_LightIntensity;


	struct SPerModelCb
	{
		float4x4 WorldMat;
	};
	static_assert((sizeof(SPerFrameCb) % 16) == 0, "");
};