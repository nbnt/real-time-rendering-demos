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

Filename: TGALoader.cpp
---------------------------------------------------------------------------*/
#include "Common.h"
#include "DirectXTex.h"

using namespace DirectX;

HRESULT CreateTgaResourceViewFromFile(ID3D11Device* pDevice,
    const wchar_t* Filename,
    bool bSrgb,
    bool bGenMipMaps,
    ID3D11Resource** ppTexture,
    ID3D11ShaderResourceView** ppSrv
    )
{
    TexMetadata Metadata;
    ScratchImage Scratch;
    LoadFromTGAFile(Filename, &Metadata, Scratch);

    assert(Metadata.dimension == TEX_DIMENSION_TEXTURE2D);
    assert(Metadata.arraySize == 1);
    assert(Metadata.mipLevels == 1);

    D3D11_TEXTURE2D_DESC Desc;
    Desc.ArraySize = 1;
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    Desc.CPUAccessFlags = 0;
    Desc.Format = Metadata.format;
    Desc.Height = UINT(Metadata.height);
    Desc.MipLevels = 1;
    Desc.MiscFlags = 0;
    Desc.SampleDesc.Count = 1;
    Desc.SampleDesc.Quality = 0;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.Width = UINT(Metadata.width);

    D3D11_SUBRESOURCE_DATA InitData;
    const Image* pImage = Scratch.GetImage(0, 0, 0);
    InitData.pSysMem = pImage->pixels;
    InitData.SysMemPitch = UINT(pImage->rowPitch);
    InitData.SysMemSlicePitch = UINT(pImage->slicePitch);

    ID3D11Texture2D* pTex;
    verify_return(pDevice->CreateTexture2D(&Desc, &InitData, &pTex));
    
    if(bGenMipMaps)
    {
        Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        Desc.MipLevels = 0;
        Desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        ID3D11Texture2D* pMippedTex;
        verify_return(pDevice->CreateTexture2D(&Desc, nullptr, &pMippedTex));

        ID3D11DeviceContextPtr pCtx;
        pDevice->GetImmediateContext(&pCtx);
        pCtx->CopySubresourceRegion(pMippedTex, 0, 0, 0, 0, pTex, 0, nullptr);
        std::swap(pMippedTex, pTex);
        pMippedTex->Release();
    }

    if(ppSrv || bGenMipMaps)
    {
        // Create SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
        SrvDesc.Texture2D.MipLevels = UINT(-1);
        SrvDesc.Texture2D.MostDetailedMip = 0;
        SrvDesc.Format = Metadata.format;
        SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        ID3D11ShaderResourceView* pSrv;
        verify_return(pDevice->CreateShaderResourceView(pTex, &SrvDesc, &pSrv));

        if(bGenMipMaps)
        {
            ID3D11DeviceContextPtr pCtx;
            pDevice->GetImmediateContext(&pCtx);
            pCtx->GenerateMips(pSrv);
        }

        if(ppSrv)
        {
            *ppSrv = pSrv;
        }
    }
    
    if(ppTexture)
    {
        ppTexture[0] = pTex;
    }
    else
    {
        pTex->Release();
    }

    return S_OK;
}