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

Filename: Common.cpp
---------------------------------------------------------------------------*/
#include "Common.h"
#include "Shlwapi.h"

void trace(const std::string& msg)
{
	MessageBoxA(nullptr, msg.c_str(), "Error!", MB_ICONEXCLAMATION);
}

void trace(const std::wstring& msg)
{
	MessageBox(nullptr, msg.c_str(), L"Error!", MB_ICONEXCLAMATION);
}

void trace(const std::wstring& file, const std::wstring& line, HRESULT hr, const std::wstring& msg)
{
	WCHAR hr_msg[512];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr);

	std::wstring error_msg;
	error_msg = hr_msg + std::wstring(L"in file ") + file + std::wstring(L" line ") + line + std::wstring(L".\n") + msg;
	trace(error_msg);
}

HRESULT FindFileInCommonDirs(const std::wstring& filename, std::wstring& result)
{
	// We don't actively search for the shader file, it's either in the current directory or in the media shader directory
	WCHAR tmp[MAX_PATH];
	GetModuleFileName(nullptr, tmp, MAX_PATH);
	PathRemoveFileSpec(tmp);
	const std::wstring pwd(tmp);

	HRESULT hr = E_FAIL;
	// Search current directory or absolute path
	if(IsFileExists(filename))
	{
		hr = S_OK;
		result = filename;
	}
	if(FAILED(hr))
	{
		const WCHAR* SearchDirs[] =
		{
			L"\\..\\..\\..\\Media\\Shaders\\",
			L"\\..\\..\\..\\Media\\Textures\\",
			L"\\..\\..\\..\\Media\\Models\\",
            L"\\..\\..\\..\\Media\\Fonts\\",
		};

		for(int i = 0; i < ARRAYSIZE(SearchDirs); i++)
		{
			result = pwd + SearchDirs[i] + filename;
			if(IsFileExists(result))
			{
				hr = S_OK;
				break;
			}
		}
	}

	return hr;
}
