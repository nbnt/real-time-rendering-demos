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

Filename: TransparencyTech.h
---------------------------------------------------------------------------*/
#pragma once
#include "ShaderUtils.h"

class CDxModel;
class CDxMesh;

class CTransparencyTech
{
public:
	struct SPerModelCb
	{
		float4x4 Wvp;
		float4x4 World;
		float3 LightDirW;
		float  pad0;
		float3 LightIntensity;
		float AlphaOut;
	};
	verify_cb_size_alignment(SPerModelCb);

	enum class TransparencyMode
	{
		Solid,
		UnorderedBlend,
	};

	CTransparencyTech(ID3D11Device* pDevice);
	void PrepareForDraw(ID3D11DeviceContext* pContext, const SPerModelCb& CbData, TransparencyMode mode);
	void DrawModel(const CDxModel* pModel, ID3D11DeviceContext* pCtx);

private:
	SVertexShaderPtr m_VS;
	SPixelShaderPtr m_SolidPS;
	SPixelShaderPtr m_UnorderedBlendingPS;
	SPixelShaderPtr m_DepthPeelingPS;

	ID3D11BufferPtr m_PerModelCb;
	ID3D11BufferPtr m_PerMeshCb;
	ID3D11DepthStencilStatePtr	m_NoDepthTest;
	ID3D11BlendStatePtr			m_SrcAlphaBlend;
	ID3D11RasterizerStatePtr	m_NoCullRastState;

	struct SPerMeshCb
    {
        float3 MeshColor;
		float pad;
	};
	verify_cb_size_alignment(SPerMeshCb);

	void SetPerMeshData(ID3D11DeviceContext* pContext, const SPerMeshCb& CbData);
	void DrawMesh(ID3D11DeviceContext* pCtx, const CDxMesh* pMesh);
};