/*
---------------------------------------------------------------------------
Real Time Rendering Demos
---------------------------------------------------------------------------

Copyright (c) 2011 - Nir Benty

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

    Filename: NoOitTech.h
---------------------------------------------------------------------------*/
#pragma once
#include "DXUT.h"
#include "d3dx11effect.h"

class CNoOitTech
{
public:
    static CNoOitTech* Create(ID3D11Device* pDevice);

    inline void SetWorldMat(const D3DXMATRIX* pWorldMat)
    {
        m_pWorldMat->SetMatrix((float*)pWorldMat);
    }
    inline void SetWVPMat(const D3DXMATRIX* pWvpMat)
    {
        m_pWVPMat->SetMatrix((float*)pWvpMat);
    }
    inline void SetLightIntensity(float Intensity)
    {
        m_pLightIntensity->SetFloat(Intensity);
    }
    inline void SetNegLightDirW(const D3DXVECTOR3* pNegLightDirW)
    {
        m_pNegLightDirW->SetFloatVector((float*)pNegLightDirW);
    }
    inline void SetCameraPosW(const D3DXVECTOR3* pCameraPosW)
    {
        m_pCameraPosW->SetFloatVector((float*)pCameraPosW);
    }

    inline void SetMeshID(UINT MeshID)
    {
        m_pMeshID->SetInt(MeshID);
    }

    inline void SetAlphaOut(float AlphaOut)
    {
        m_pAlphaOut->SetFloat(AlphaOut);
    }

    void Apply(ID3D11DeviceContext* pDeviceContext, bool bBlendEnabled)
    {
        float factor[] = {0, 0, 0, 0};
        pDeviceContext->OMSetBlendState(bBlendEnabled ? m_pBlendEnabled : NULL, factor, 0xff);
        m_pTechnique->GetPassByIndex(0)->Apply(0, pDeviceContext);
    }

    inline const D3DX11_PASS_DESC* GetPassDesc()
    {
        return &m_PassDesc;
    }

    ~CNoOitTech()
    {
        SAFE_RELEASE(m_pFX);
        SAFE_RELEASE(m_pBlendEnabled);
    }
private:
    CNoOitTech() {};
    ID3DX11EffectMatrixVariable* m_pWorldMat;
    ID3DX11EffectMatrixVariable* m_pWVPMat;
    ID3DX11EffectScalarVariable* m_pLightIntensity;
    ID3DX11EffectScalarVariable* m_pAlphaOut;
    ID3DX11EffectVectorVariable* m_pNegLightDirW;
    ID3DX11EffectVectorVariable* m_pCameraPosW;
    ID3DX11EffectScalarVariable* m_pMeshID;

    ID3DX11Effect* m_pFX;
    ID3DX11EffectTechnique* m_pTechnique;

    D3DX11_PASS_DESC m_PassDesc;
    ID3D11BlendState* m_pBlendEnabled;
};
