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

Filename: Font.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include "Common.h"

class CFont
{	
public:
    CFont(ID3D11Device* pDevice);
    CFont(ID3D11Device* pDevice, const std::wstring& FontName, float size);
    CFont(const CFont&) = delete;
    CFont& operator=(const CFont&) = delete;

    struct SCharDesc
    {
		float2 TopLeft;
		float2 Size;
    };

    ID3D11ShaderResourceView* GetSrv() const {return m_pSrv;}
	const SCharDesc& GetCharDesc(WCHAR Char) const
	{
		assert(Char >= m_FirstChar && Char <= m_LastChar);
		return m_CharDesc[Char - m_FirstChar];
	}

    float GetFontHeight() const {return m_FontHeight;}
    float GetTabWidth() const {return m_TabWidth;}
    float GetLettersSpacing() const {return m_LetterSpacing;}

private:
    void Create(ID3D11Device* pDevice, const std::wstring& FontName, float size);
    void SaveToFile(ID3D11Device* pDevice, const std::wstring& FontName, float size);
    bool LoadFromFile(ID3D11Device* pDevice, const std::wstring& FontName, float size);

    static const WCHAR m_FirstChar = '!';
    static const WCHAR m_LastChar = '~';
    static const UINT m_CharCount = m_LastChar - m_FirstChar + 1;
    static const UINT m_TexWidth = 1024;

    ID3D11ShaderResourceViewPtr m_pSrv;
    SCharDesc m_CharDesc[m_CharCount];
    float m_FontHeight;
    float m_TabWidth;
    float m_LetterSpacing;
};