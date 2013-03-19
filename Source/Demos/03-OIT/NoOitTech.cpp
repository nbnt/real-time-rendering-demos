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

    Filename: NoOitTech.cpp
---------------------------------------------------------------------------*/
#include "NoOitTech.h"

COitTech* COitTech::Create(ID3D11Device* pDevice)
{
    COitTech* pOitTech = new COitTech;
    if(pOitTech)
    {
        // Create the effect
        if(D3DX11CreateEffectFromFile(L"03-OIT\\03-NoOit.fx", pDevice, &pOitTech->m_pFX) == S_OK)
        {
            pOitTech->m_pNoOitTech = pOitTech->m_pFX->GetTechniqueByName("NoOitTech");
            if(pOitTech->m_pNoOitTech->IsValid() == FALSE)
            {
                trace(L"Can't create NoOit technique");
                PostQuitMessage(0);
                return NULL;
            }

            pOitTech->m_pDepthPeelTech = pOitTech->m_pFX->GetTechniqueByName("DepthPeelTech");
            if(pOitTech->m_pDepthPeelTech->IsValid() == FALSE)
            {
                trace(L"Can't create DepthPeel technique");
                PostQuitMessage(0);
                return NULL;
            }

            pOitTech->m_pWVPMat = pOitTech->m_pFX->GetVariableByName("gWVPMat")->AsMatrix();
            pOitTech->m_pWorldMat = pOitTech->m_pFX->GetVariableByName("gWorldMat")->AsMatrix();
            pOitTech->m_pLightIntensity = pOitTech->m_pFX->GetVariableByName("gLightIntensity")->AsScalar();
            pOitTech->m_pNegLightDirW = pOitTech->m_pFX->GetVariableByName("gNegLightDirW")->AsVector();
            pOitTech->m_pCameraPosW = pOitTech->m_pFX->GetVariableByName("gCameraPosW")->AsVector();
            pOitTech->m_pAlphaOut = pOitTech->m_pFX->GetVariableByName("gAlphaOut")->AsScalar();
            pOitTech->m_pMeshID = pOitTech->m_pFX->GetVariableByName("gMeshID")->AsScalar();

            pOitTech->m_pDepthTex = pOitTech->m_pFX->GetVariableByName("gDepthTex")->AsShaderResource();

            pOitTech->m_pNoOitTech->GetPassByIndex(0)->GetDesc(&pOitTech->m_PassDesc);

            // Create the blend state
            D3D11_BLEND_DESC BlendDesc;
            BlendDesc.AlphaToCoverageEnable = FALSE;
            BlendDesc.IndependentBlendEnable = FALSE;
            BlendDesc.RenderTarget[0].BlendEnable = TRUE;
            BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;

            if(pDevice->CreateBlendState(&BlendDesc, &pOitTech->m_pBlendEnabled) != S_OK)
            {
                trace(L"Can't create NoOit blend state");
                SAFE_DELETE(pOitTech);
                PostQuitMessage(0);
                return NULL;
            }
        }
        else
        {
            SAFE_DELETE(pOitTech);
        }
    }
    
    return pOitTech;
}
