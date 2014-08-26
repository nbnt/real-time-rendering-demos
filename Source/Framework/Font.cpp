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
using namespace Gdiplus;

#define v_gdi_plus(a) if((a) != Gdiplus::Status::Ok) {trace(__WIDEFILE__, __LINE__, E_FAIL, L"GDIPlus returned an error"); PostQuitMessage(1);return;}

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


CFont::CFont(ID3D11Device* pDevice) : CFont(pDevice, L"Arial", 18, true)
{
}

CFont::~CFont()
{
    SAFE_RELEASE(m_pSrv);
}

CFont::CFont(ID3D11Device* pDevice, const std::wstring& FontName, float size, bool bAntiAliased) : m_pSrv(nullptr)
{
    TextRenderingHint hint = bAntiAliased ? TextRenderingHintAntiAliasGridFit : TextRenderingHintSingleBitPerPixelGridFit;

    // Start GDI+
    GdiPlusWrapper GdiWrapper;

    // Create the GDI font
    Gdiplus::Font GdiFont(FontName.c_str(), size, FontStyleRegular, UnitPixel);
    v_gdi_plus(GdiFont.GetLastStatus());

    // Calculate the required texture width using a temporary bitmap
    INT TempSize = INT(size)*m_NumChars*2;
    Bitmap TempBmp(TempSize, TempSize, PixelFormat32bppARGB);
    v_gdi_plus(TempBmp.GetLastStatus());
    
    Graphics TempGraphics(&TempBmp);
    v_gdi_plus(TempGraphics.GetLastStatus());
    v_gdi_plus(TempGraphics.SetTextRenderingHint(hint));

    WCHAR AllLetters[m_NumChars + 1];
    for(UINT i = 0 ; i < m_NumChars ; i++)
    {
        AllLetters[i] = m_FirstChar + i;
    }
    AllLetters[m_NumChars] = 0;
    RectF r;
    v_gdi_plus(TempGraphics.MeasureString(AllLetters, m_NumChars, &GdiFont, PointF(0, 0), &r));
    float NumRows = ceil(r.Width / float(m_TexWidth));
    float FontHeight = GdiFont.GetHeight(&TempGraphics);
    m_TexHeight = UINT((NumRows * FontHeight) + 1);

    // Start to initialize the bitmap
    Bitmap FontBitmap(m_TexWidth, m_TexHeight, PixelFormat32bppARGB);
    v_gdi_plus(FontBitmap.GetLastStatus());

    Graphics FontGraphics(&FontBitmap);
    v_gdi_plus(FontGraphics.GetLastStatus());
    v_gdi_plus(FontGraphics.SetTextRenderingHint(hint));

    SolidBrush FontBrush(Color(0xFFFFFFFF));
    v_gdi_plus(FontBrush.GetLastStatus());

    // Init the space width
    WCHAR Space = ' ';
    v_gdi_plus(TempGraphics.MeasureString(&Space, 1, &GdiFont, PointF(0,0), &r));
    m_SpaceWidth = r.Width;

    INT DstX = 0;
    INT DstY = 0;
    INT Height = INT(FontHeight + 1);

    for(UINT i = 0 ; i < m_NumChars; i++)
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
                Gdiplus::Color color;
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
                Gdiplus::Color color;
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
        INT Width = MaxX - MinX;
        if(DstX + Width > m_TexWidth)
        {
            DstX = 0;
            DstY += INT(FontHeight);
        }

        // Copy the font to the dst texture
        v_gdi_plus(FontGraphics.DrawImage(&TempBmp, DstX, DstY, MinX, 0, Width, Height, UnitPixel));
        m_CharDesc[i].TopLeftX = float(DstX);
        m_CharDesc[i].TopLeftY = float(DstY);
        m_CharDesc[i].Width = float(Width);
        m_CharDesc[i].Height = float(Height);

        DstX += Width + 1;
    }

    // Create the D3D texture
    BitmapData data;
    FontBitmap.LockBits(&Rect(0, 0, m_TexWidth, m_TexHeight), ImageLockModeRead, PixelFormat32bppARGB, &data);
    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Height = m_TexHeight;
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

    ID3D11Texture2D* Tex2D;
    verify(pDevice->CreateTexture2D(&desc, &InitData, &Tex2D));
    verify(pDevice->CreateShaderResourceView(Tex2D, nullptr, &m_pSrv));
    SAFE_RELEASE(Tex2D);

    TempBmp.UnlockBits(&data);
}