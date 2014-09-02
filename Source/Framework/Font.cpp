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

Filename: Font.cpp
---------------------------------------------------------------------------*/
#include "Font.h"
#include <gdiplus.h>
#include <fstream>
#include "ScreenGrab.h"
using namespace Gdiplus;

#define v_gdi_plus(a) if((a) != Gdiplus::Status::Ok) {trace(__WIDEFILE__, __WIDELINE__, E_FAIL, L"GDIPlus returned an error"); PostQuitMessage(1);return;}

std::wstring GetFontDirectory()
{
    const std::wstring FontDir = L"\\..\\..\\..\\Media\\Fonts\\";
    WCHAR tmp[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, tmp);
    const std::wstring pwd(tmp);

    return pwd + FontDir;
}

std::wstring GetFontFilename(const std::wstring& FontName, float size)
{
    assert(IsDirectoryExists(GetFontDirectory().c_str()));
    std::wstring Filename = GetFontDirectory() + FontName + std::to_wstring(size);
    return Filename;
}

class GdiPlusWrapper
{
public:
    GdiPlusWrapper()
    {
        v_gdi_plus(GdiplusStartup(&m_Token, &m_StartupInput, nullptr));
        assert(m_Token);
    }

    ~GdiPlusWrapper()
    {
        GdiplusShutdown(m_Token);
    }

    ULONG_PTR m_Token;
    GdiplusStartupInput m_StartupInput;
};


CFont::CFont(ID3D11Device* pDevice) : CFont(pDevice, L"Bitstream Vera Sans Mono", 14)
{
}

CFont::CFont(ID3D11Device* pDevice, const std::wstring& FontName, float size)
{
    if(LoadFromFile(pDevice, FontName, size) == false)
    {
       Create(pDevice, FontName, size);
    }
}

void CFont::Create(ID3D11Device* pDevice, const std::wstring& FontName, float size)
{
    TextRenderingHint hint = TextRenderingHintAntiAliasGridFit;

    // Start GDI+
    GdiPlusWrapper GdiWrapper;

    // Create the GDI font
    Gdiplus::Font GdiFont(FontName.c_str(), size, FontStyleBold, UnitPixel);
    v_gdi_plus(GdiFont.GetLastStatus());

    // Calculate the required texture width using a temporary bitmap
    INT TempSize = INT(size)*m_CharCount*2;
    Bitmap TempBmp(TempSize, TempSize, PixelFormat32bppARGB);
    v_gdi_plus(TempBmp.GetLastStatus());
    
    Graphics TempGraphics(&TempBmp);
    v_gdi_plus(TempGraphics.GetLastStatus());
    v_gdi_plus(TempGraphics.SetTextRenderingHint(hint));

    WCHAR AllLetters[m_CharCount + 1];
    for(UINT i = 0 ; i < m_CharCount ; i++)
    {
        AllLetters[i] = m_FirstChar + i;
    }
    AllLetters[m_CharCount] = 0;
    RectF r;
    v_gdi_plus(TempGraphics.MeasureString(AllLetters, m_CharCount, &GdiFont, PointF(0, 0), &r));
    float NumRows = ceil(r.Width / float(m_TexWidth));
    m_FontHeight = GdiFont.GetHeight(&TempGraphics);
    UINT TexHeight = UINT((NumRows * m_FontHeight) + 1);

    // Start to initialize the bitmap
    Bitmap FontBitmap(m_TexWidth, TexHeight, PixelFormat32bppARGB);
    v_gdi_plus(FontBitmap.GetLastStatus());

    Graphics FontGraphics(&FontBitmap);
    v_gdi_plus(FontGraphics.GetLastStatus());
    v_gdi_plus(FontGraphics.SetTextRenderingHint(hint));

    SolidBrush FontBrush(Color(0xFFFFFFFF));
    v_gdi_plus(FontBrush.GetLastStatus());

    // Init tab width
    WCHAR Tab = '\t';
    v_gdi_plus(TempGraphics.MeasureString(&Tab, 1, &GdiFont, PointF(0, 0), &r));
    m_TabWidth = r.Width;

    // Init the space width
    WCHAR Space = ' ';
    v_gdi_plus(TempGraphics.MeasureString(&Space, 1, &GdiFont, PointF(0,0), &r));
    m_SpaceWidth = r.Width;

    // Init letter spacing
    m_LetterSpacing = ceil(m_FontHeight * 0.05f);

    INT DstX = 0;
    INT DstY = 0;
    INT Height = INT(m_FontHeight + 1);

    for(UINT i = 0 ; i < m_CharCount; i++)
    {
        WCHAR c = i + m_FirstChar;
        RectF r;
        SolidBrush Brush(Color(0xFFFFFFFF));

        // Draw the character into the temporary bitmap
		v_gdi_plus(TempGraphics.Clear(Color(0, 0, 0, 0)));
        v_gdi_plus(TempGraphics.DrawString(&c, 1, &GdiFont, PointF(0,0), &Brush));

        // Get minX
        INT MinX = 0;
        for(INT x = 0; x < TempSize; x++)
        {
            for(INT y = 0; y < Height; y++)
            {
                Color color;
                v_gdi_plus(TempBmp.GetPixel(x, y, &color));
                if(color.GetAlpha() > 0)
                {
                    MinX = x;
                    x = TempSize;
                    break;
                }
            }
        }

        // Get maxX
        INT MaxX = TempSize - 1;
        for(INT x = TempSize - 1; x >= 0; x--)
        {
            for(INT y = 0; y < Height; y++)
            {
                Color color;
                v_gdi_plus(TempBmp.GetPixel(x, y, &color));
                if(color.GetAlpha() > 0)
                {
                    MaxX = x;
                    x = 0;
                    break;
                }
            }
        }

        // Check if have enough space left in the current row
        INT Width = MaxX - MinX + 1;
        if(DstX + Width > m_TexWidth)
        {
            DstX = 0;
            DstY += INT(m_FontHeight);
        }

        // Copy the font to the dst texture
        v_gdi_plus(FontGraphics.DrawImage(&TempBmp, DstX, DstY, MinX, 0, Width, Height, UnitPixel));
		m_CharDesc[i].TopLeft = float2(float(DstX), float(DstY));
		m_CharDesc[i].Size = float2(float(Width), float(Height));

        DstX += Width + 1;
    }

    // Create the D3D texture
    BitmapData data;
    FontBitmap.LockBits(&Rect(0, 0, m_TexWidth, TexHeight), ImageLockModeRead, PixelFormat32bppARGB, &data);
    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Height = TexHeight;
    desc.MipLevels = 1;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.Width = m_TexWidth;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = data.Scan0;
    InitData.SysMemPitch = data.Stride;
    InitData.SysMemSlicePitch = 0;

    ID3D11Texture2DPtr Tex2D;
    verify(pDevice->CreateTexture2D(&desc, &InitData, &Tex2D));
    verify(pDevice->CreateShaderResourceView(Tex2D, nullptr, &m_pSrv));

    TempBmp.UnlockBits(&data);

    // Cache the font
    SaveToFile(pDevice, FontName, size);
}


static const DWORD FontMagicNumber = 0xDEAD0001;

#pragma pack(1)
struct SFontFileHeader
{
    UINT32 StructSize;
    UINT32 CharDataSize;
    DWORD MagicNumber;  
    UINT32 CharCount;
    float SpaceWidth;
    float FontHeight;
    float TabWidth;
    float LetterSpacing;
};

#pragma pack(1)
struct SFontCharData
{
    char Char;
    float TopLeftX;
    float TopLeftY;
    float Width;
    float Height;
};

void CFont::SaveToFile(ID3D11Device* pDevice, const std::wstring& FontName, float size)
{
    std::wstring Filename = GetFontFilename(FontName, size);
    std::wstring TextureFilename = Filename + L".dds";
    std::wstring DataFilename = Filename + L".bin";

    // Save the texture
    ID3D11DeviceContextPtr pCtx;
    ID3D11ResourcePtr pResource;
    pDevice->GetImmediateContext(&pCtx);
    m_pSrv->GetResource(&pResource);

    
    verify(DirectX::SaveDDSTextureToFile(pCtx.GetInterfacePtr(), pResource.GetInterfacePtr(), TextureFilename.c_str()));

    // Store the data
    std::ofstream Data(DataFilename, std::ios::binary);
    
    // Store the header
    SFontFileHeader Header;
    Header.StructSize = sizeof(Header);
    Header.CharDataSize = sizeof(SFontCharData);
    Header.CharCount = m_CharCount;
    Header.MagicNumber = FontMagicNumber;
    Header.SpaceWidth = m_SpaceWidth;
    Header.FontHeight = m_FontHeight;
    Header.TabWidth = m_TabWidth;
    Header.LetterSpacing = m_LetterSpacing;
    Data.write((char*)&Header, sizeof(Header));

    // Store the char data
    for(auto i = 0; i < m_CharCount; i++)
    {
        SFontCharData CharData;
        CharData.Char = i + m_FirstChar;
        CharData.TopLeftX = m_CharDesc[i].TopLeft.x;
        CharData.TopLeftY = m_CharDesc[i].TopLeft.y;
        CharData.Width = m_CharDesc[i].Size.x;
        CharData.Height = m_CharDesc[i].Size.y;

        Data.write((char*)&CharData, sizeof(CharData));
    }
    Data.close();
}

bool CFont::LoadFromFile(ID3D11Device* pDevice, const std::wstring& FontName, float size)
{
    std::wstring Filename = GetFontFilename(FontName, size);
    std::wstring TextureFilename = Filename + L".dds";
    std::wstring DataFilename = Filename + L".bin";
    if((IsFileExists(TextureFilename) == false) || (IsFileExists(DataFilename) == false))
    {
        return false;
    }

    // Load the data
    std::ifstream Data(DataFilename, std::ios::binary);
    SFontFileHeader Header;
    // Read the header
    Data.read((char*)&Header, sizeof(Header));
    bool bValid = (Header.StructSize == sizeof(Header));
    bValid = bValid && (Header.MagicNumber == FontMagicNumber);
    bValid = bValid && (Header.CharDataSize == sizeof(SFontCharData));
    bValid = bValid && (Header.CharCount == m_CharCount);

    if(bValid == false)
    {
        Data.close();
        return false;
    }

    m_LetterSpacing = Header.LetterSpacing;
    m_TabWidth = Header.TabWidth;
    m_FontHeight = Header.FontHeight;
    m_SpaceWidth = Header.SpaceWidth;

    // Load the char data
    for(auto i = 0; i < m_CharCount; i++)
    {
        SFontCharData CharData;
        Data.read((char*)&CharData, sizeof(SFontCharData));
        char Char = i + m_FirstChar;
        if(CharData.Char != i + m_FirstChar)
        {
            Data.close();
            return false;
        }
        
        m_CharDesc[i].TopLeft.x = CharData.TopLeftX;
        m_CharDesc[i].TopLeft.y = CharData.TopLeftY;
        m_CharDesc[i].Size.x = CharData.Width;
        m_CharDesc[i].Size.y = CharData.Height;
    }

    // Load the texture
    m_pSrv = CreateShaderResourceViewFromFile(pDevice, TextureFilename, false);
    return true;
}