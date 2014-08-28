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

Filename: Common.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>
#include <codecvt>

#include "simplemath.h"

#include "D3DSmartInterface.h"
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WIDEFILE__ WIDEN(__FILE__)

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define __WIDELINE__ WIDEN(STRINGIZE(__LINE__))

void trace(const std::string& msg);
void trace(const std::wstring& msg);
void trace(const std::wstring& file, const std::wstring& line, HRESULT hr, const std::wstring& msg);

#ifdef _DEBUG
#define verify(a) {HRESULT __hr = a; if(FAILED(__hr)) { trace( __WIDEFILE__, __WIDELINE__, __hr, L#a); } }
#define verify_return(a) {HRESULT __hr = a; if(FAILED(__hr)) { trace( __WIDEFILE__, __WIDELINE__, __hr, L#a); return __hr;} }
#else
#define verify(a) a
#define verify_return(a) {HRESULT __hr = a ; if(FAILED(__hr)) {return __hr;}}
#endif

#define SAFE_DELETE(a) {if(a) {delete a; a = nullptr;}}
#define SAFE_DELETE_ARRAY(a) {if(a) {delete[] a; a = nullptr;}}

inline bool IsFileExists(const std::wstring& filename)
{
	DWORD Attr = GetFileAttributes(filename.c_str());
	return (Attr != INVALID_FILE_ATTRIBUTES);
}

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;

HRESULT FindFileInCommonDirs(const std::wstring& filename, std::wstring& result);

inline std::string wstring_2_string(const std::wstring& ws)
{
	std::wstring_convert<std::codecvt_utf8<WCHAR>> cvt;
	std::string s = cvt.to_bytes(ws.c_str());
	return s;
}

inline std::wstring string_2_wstring(const std::string& s)
{
	std::wstring_convert<std::codecvt_utf8<WCHAR>> cvt;
	std::wstring ws = cvt.from_bytes(s);
	return ws;
}

