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

Filename: ShaderUtils.cpp
---------------------------------------------------------------------------*/
#include "ShaderUtils.h"
#include <d3dcompiler.h>
#include <sstream>

static ID3DBlob* CompileShader(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const std::string& Target)
{
	// We don't activley search for the shader file, it's either in the current directory or in the media shader directory
	WCHAR tmp[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, tmp);
	std::wstring pwd(tmp);

	const WCHAR* SearchDirs[] =
	{
		L"\\",									// Shader is in the current directory
		L"\\..\\..\\..\\Media\\Shaders\\",		// App is running from bin directory
	};

	std::wstring FullPath;
	for (int i = 0; i < ARRAYSIZE(SearchDirs); i++)
	{
		if (IsFileExists(pwd + SearchDirs[i] + Filename))
		{
			FullPath = pwd + SearchDirs[i] + Filename;
			break;
		}
	}

	if (FullPath.length() == 0)
	{
		std::wstring msg = L"Can't find shader file " + Filename;
		trace(__WIDEFILE__, __WIDELINE__, E_INVALIDARG, msg.c_str());
		return nullptr;
	}

	ID3DBlob* pCode;
	ID3DBlobPtr pErrors;

	UINT flags = D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = D3DCompileFromFile(FullPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), Target.c_str(), flags, 0, &pCode, &pErrors);
	if (FAILED(hr))
	{
		std::wstring msg = L"Failed to compile shader " + Filename + L".\n\n";
		WCHAR error[2048];
		MultiByteToWideChar(CP_ACP, 0, (LPCCH)pErrors->GetBufferPointer(), pErrors->GetBufferSize(), error, ARRAYSIZE(error));
		msg += error;
		trace(__WIDEFILE__, __WIDELINE__, hr, msg.c_str());
		return nullptr;
	}

	return pCode;
}


SVertexShaderPtr CreateVsFromFile(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const std::string& Target)
{
	SVertexShaderPtr ShaderPtr = std::make_unique<CShader<ID3D11VertexShaderPtr>>();

	ShaderPtr->pCodeBlob = CompileShader(pDevice, Filename, EntryPoint, Target);
	if (ShaderPtr->pCodeBlob.GetInterfacePtr())
	{
		verify(pDevice->CreateVertexShader(ShaderPtr->pCodeBlob->GetBufferPointer(), ShaderPtr->pCodeBlob->GetBufferSize(), nullptr, &ShaderPtr->pShader));
		verify(D3DReflect(ShaderPtr->pCodeBlob->GetBufferPointer(), ShaderPtr->pCodeBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&ShaderPtr->pReflector));
	}

	return ShaderPtr;
}

SPixelShaderPtr CreatePsFromFile(ID3D11Device* pDevice, const std::wstring& Filename, const std::string& EntryPoint, const std::string& Target)
{
	SPixelShaderPtr ShaderPtr = std::make_unique<CShader<ID3D11PixelShaderPtr>>();

	ShaderPtr->pCodeBlob = CompileShader(pDevice, Filename, EntryPoint, Target);
	if (ShaderPtr->pCodeBlob.GetInterfacePtr())
	{
		verify(pDevice->CreatePixelShader(ShaderPtr->pCodeBlob->GetBufferPointer(), ShaderPtr->pCodeBlob->GetBufferSize(), nullptr, &ShaderPtr->pShader));
		verify(D3DReflect(ShaderPtr->pCodeBlob->GetBufferPointer(), ShaderPtr->pCodeBlob->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&ShaderPtr->pReflector));
	}

	return ShaderPtr;
}

bool VerifyConstantLocation(ID3D11ShaderReflection* pReflector, const std::string& VarName, UINT CbIndex, UINT Offset)
{
	ID3D11ShaderReflectionConstantBuffer* pCb = pReflector->GetConstantBufferByIndex(CbIndex);
	ID3D11ShaderReflectionVariable* pVar = pCb->GetVariableByName(VarName.c_str());
	D3D11_SHADER_VARIABLE_DESC Desc;
	HRESULT hr = pVar->GetDesc(&Desc);
	std::stringstream ss;

	if (FAILED(hr))
	{
		ss << "Can't find variable \"" + VarName + "\" in Cb" << CbIndex;
	}
	else if (Desc.StartOffset != Offset)
	{
		ss << "Var \"" << VarName << "\" offset mismatch. Expected " << Offset << " ,Found " << Desc.StartOffset;
	}

	if (ss.str().size())
	{
		trace(ss.str());
		return false;
	}
	return true;
}

bool VerifyResourceLocation(ID3D11ShaderReflection* pReflector, const std::string& VarName, UINT SrvIndex, UINT ArraySize)
{
	D3D11_SHADER_INPUT_BIND_DESC desc;
	HRESULT hr = pReflector->GetResourceBindingDescByName(VarName.c_str(), &desc);
	std::stringstream ss;

	if (FAILED(hr))
	{
		ss << "Can't find SRV \"" + VarName + "\".";
	}
	else if (desc.Type != D3D_SIT_TEXTURE)
	{
		ss << "SRV \"" << VarName << "\" not a texture";
	}
	else if (desc.BindPoint != SrvIndex)
	{
		ss << "SRV \"" << VarName << "\" index mismatch. Expected " << SrvIndex << " ,Found " << desc.BindPoint;
	}
	else if (desc.BindCount != ArraySize)
	{
		ss << "SRV \"" << VarName << "\" array size mismatch. Expected " << SrvIndex << " ,Found " << desc.BindCount;
	}
	
	if (ss.str().size())
	{
		trace(ss.str());
		return false;
	}
	return true;
}