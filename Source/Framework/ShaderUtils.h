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

Filename: ShaderUtils.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include "Common.h"

template<typename T>
class CShader
{
public:
	CShader(T ShaderPtr, ID3D11ShaderReflection* pReflector, ID3DBlob* pBlob);
	T GetShader() const { return m_pShader.GetInterfacePtr(); }
	ID3DBlobPtr GetBlob() const {return m_pCodeBlob.GetInterfacePtr();}

	// Functions to verify shader variable positions
	bool VerifyConstantLocation(const std::string& VarName, UINT CbIndex, UINT Offset) const;
	bool VerifyResourceLocation(const std::string& VarName, UINT SrvIndex, UINT ArraySize) const;
	bool VerifySamplerLocation(const std::string& VarName, UINT SamplerIndex) const;

private:
	T m_pShader;
	ID3DBlobPtr m_pCodeBlob;
	ID3D11ShaderReflectionPtr m_pReflector;
};

#define verify_cb_size_alignment(_T)  static_assert((sizeof(_T) % 16) == 0, "Unaligned shader cb");

using CVertexShader = CShader<ID3D11VertexShaderPtr>;
using CPixelShader = CShader<ID3D11PixelShaderPtr>;
using CDomainShader = CShader<ID3D11DomainShaderPtr>;
using CHullShader = CShader<ID3D11HullShaderPtr>;
using CGeometryShader = CShader<ID3D11GeometryShaderPtr>;

using CVertexShaderPtr   = std::unique_ptr<CVertexShader>;
using CPixelShaderPtr    = std::unique_ptr<CPixelShader>;
using CDomainShaderPtr   = std::unique_ptr<CDomainShader>;
using CHullShaderPtr     = std::unique_ptr<CHullShader>;
using CGeometryShaderPtr = std::unique_ptr<CGeometryShader> ;

CVertexShaderPtr CreateVsFromFile(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr, const std::string& Target = "vs_5_0");
CPixelShaderPtr  CreatePsFromFile(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr, const std::string& Target = "ps_5_0");
CDomainShaderPtr CreateDsFromFile(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr, const std::string& Target = "ds_5_0");
CHullShaderPtr	  CreateHsFromFile(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr, const std::string& Target = "hs_5_0");
CGeometryShaderPtr CreateGsFromFile(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const D3D_SHADER_MACRO* Defines = nullptr, const std::string& Target = "gs_5_0");

template<typename T>
void UpdateEntireConstantBuffer(ID3D11DeviceContext* pCtx, ID3D11Buffer* pCb, const T& Data)
{
	D3D11_MAPPED_SUBRESOURCE MapInfo;

	verify(pCtx->Map(pCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &MapInfo));
	T* pCbData = (T*)MapInfo.pData;
	*pCbData = Data;
	pCtx->Unmap(pCb, 0);
}