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

Filename: DxState.cpp
---------------------------------------------------------------------------*/
#include "Common.h"
#include "DxState.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "StringUtils.h"

ID3D11ShaderResourceView* CreateShaderResourceViewFromFile(ID3D11Device* pDevice, const std::wstring& Filename, bool bSrgb)
{
	// null-call to get the size
	std::wstring fullpath;
	verify(FindFileInCommonDirs(Filename, fullpath));

	ID3D11DeviceContextPtr pCtx;
	pDevice->GetImmediateContext(&pCtx);

	ID3D11ShaderResourceView* pSrv = nullptr;
	const std::wstring dds(L".dds");

	bool bDDS = HasSuffix(fullpath, dds, false);

	if(bDDS)
	{
		verify(DirectX::CreateDDSTextureFromFileEx(pDevice, pCtx, fullpath.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, bSrgb, nullptr, &pSrv));
	}
	else
	{
		verify(DirectX::CreateWICTextureFromFileEx(pDevice, pCtx, fullpath.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, bSrgb, nullptr, &pSrv));
	}
	return pSrv;
}