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

Filename: TextRenderer.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include <memory>
#include "Common.h"
#include "Font.h"
#include "ShaderUtils.h"

class CTextRenderer
{	
public:
	CTextRenderer(ID3D11Device* pDevice);
	CTextRenderer(const CTextRenderer&) = delete;
	CTextRenderer& operator=(const CTextRenderer&) = delete;

	void SetFont(std::unique_ptr<CFont>& pFont);
	void Begin(ID3D11DeviceContext* pCtx, const float2& StartPos);
	void End();
	void RenderLine(const std::wstring& line);

	struct SVertex
	{
		float2 ScreenPos;
		float2 TexCoord;
	};
private:
    ID3D11DeviceContext* m_pContext = nullptr;
	float2 m_CurPos = { 0, 0 };
	float2 m_StartPos = { 0, 0 };

	std::unique_ptr<CFont> m_pFont;
	struct SPerBatchCB
	{
		float4x4 vpTransform;
	};

	SVertexShaderPtr m_VS;
	SPixelShaderPtr  m_PS;
	ID3D11InputLayoutPtr  m_InputLayout;
	ID3D11BufferPtr		m_VertexBuffer;
	ID3D11DepthStencilStatePtr m_DepthStencilState;
	ID3D11RasterizerStatePtr   m_RasterizerState;
	ID3D11BufferPtr m_PerBatchCB;
	ID3D11BlendStatePtr m_BlendState;

	void CreateVertexShader(ID3D11Device* pDevice);
	void CreatePixelShader(ID3D11Device* pDevice);
	void CreateInputLayout(ID3D11Device* pDevice);
	void CreateVertexBuffer(ID3D11Device* pDevice);
	void CreateDepthStencilState(ID3D11Device* pDevice);
	void CreateRasterizerState(ID3D11Device* pDevice);
	void CreateConstantBuffer(ID3D11Device* pDevice);
	void CreateBlendState(ID3D11Device* pDevice);

	static const auto MaxBatchSize = 1000;
};