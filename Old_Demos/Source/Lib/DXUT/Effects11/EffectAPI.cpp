//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//  File:       EffectAPI.cpp
//  Content:    D3DX11 Effect DLL entry points
//
//////////////////////////////////////////////////////////////////////////////

#include "pchfx.h"
#include "DXUT.h"
#include "SDKmisc.h"

using namespace D3DX11Effects;

HRESULT WINAPI D3DX11CreateEffectFromMemory(CONST void *pData, SIZE_T DataLength, UINT FXFlags, ID3D11Device *pDevice, ID3DX11Effect **ppEffect)
{
    HRESULT hr = S_OK;

    // Note that pData must point to a compiled effect, not HLSL
    VN( *ppEffect = NEW CEffect( FXFlags & D3DX11_EFFECT_RUNTIME_VALID_FLAGS) );
    VH( ((CEffect*)(*ppEffect))->LoadEffect(pData, static_cast<UINT>(DataLength)) );
    VH( ((CEffect*)(*ppEffect))->BindToDevice(pDevice) );

lExit:
    if (FAILED(hr))
    {
        SAFE_RELEASE(*ppEffect);
    }
    return hr;
}

HRESULT WINAPI D3DX11CreateEffectFromFile(WCHAR* filename, ID3D11Device *pDevice, ID3DX11Effect **ppEffect)
{
    HRESULT hr;

    ID3D10Blob* pShader;
    ID3D10Blob* pErrors;
    WCHAR str[1024];
    hr = DXUTFindDXSDKMediaFileCch(str, 1024, filename);
    if(FAILED(hr))
    {
        trace(L"Can't find shader file");
        return E_FAIL;
    }
    DWORD ShaderFlags = 0;
#ifdef _DEBUG
    ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif
    hr = D3DX11CompileFromFile(str, NULL, NULL, NULL, "fx_5_0", ShaderFlags, 0, NULL, &pShader, &pErrors, NULL);
    if (FAILED(hr))
    {
        MessageBoxA(NULL, (char *)pErrors->GetBufferPointer(), "Error", MB_OK);
        return E_FAIL;
    }
    V_RETURN(D3DX11CreateEffectFromMemory(pShader->GetBufferPointer(), pShader->GetBufferSize(), ShaderFlags, pDevice, ppEffect));

    SAFE_RELEASE(pErrors);
    SAFE_RELEASE(pShader);

    return S_OK;
}