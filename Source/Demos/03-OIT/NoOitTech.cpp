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

CNoOitTech* CNoOitTech::Create(ID3D11Device* pDevice)
{
    CNoOitTech* pNoOitTech = new CNoOitTech;
    if(pNoOitTech)
    {
        // Create the effect
        if(D3DX11CreateEffectFromFile(L"03-OIT\\03-NoOit.fx", pDevice, &pNoOitTech->m_pFX) == S_OK)
        {
            pNoOitTech->m_pTechnique = pNoOitTech->m_pFX->GetTechniqueByName("NoOitTech");
            if(pNoOitTech->m_pTechnique->IsValid() == FALSE)
            {
                trace(L"Can't create NoOit technique");
                PostQuitMessage(0);
                return NULL;
            }
            pNoOitTech->m_pWVPMat = pNoOitTech->m_pFX->GetVariableByName("gWVPMat")->AsMatrix();
            pNoOitTech->m_pWorldMat = pNoOitTech->m_pFX->GetVariableByName("gWorldMat")->AsMatrix();
            pNoOitTech->m_pLightIntensity = pNoOitTech->m_pFX->GetVariableByName("gLightIntensity")->AsScalar();
            pNoOitTech->m_pNegLightDirW = pNoOitTech->m_pFX->GetVariableByName("gNegLightDirW")->AsVector();
            pNoOitTech->m_pCameraPosW = pNoOitTech->m_pFX->GetVariableByName("gCameraPosW")->AsVector();
            pNoOitTech->m_pAlphaOut = pNoOitTech->m_pFX->GetVariableByName("gAlphaOut")->AsScalar();
            pNoOitTech->m_pMeshID = pNoOitTech->m_pFX->GetVariableByName("gMeshID")->AsScalar();

            pNoOitTech->m_pTechnique->GetPassByIndex(0)->GetDesc(&pNoOitTech->m_PassDesc);

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

            if(pDevice->CreateBlendState(&BlendDesc, &pNoOitTech->m_pBlendEnabled) != S_OK)
            {
                trace(L"Can't create NoOit blend state");
                SAFE_DELETE(pNoOitTech);
                PostQuitMessage(0);
                return NULL;
            }
        }
        else
        {
            SAFE_DELETE(pNoOitTech);
        }
    }
    
    return pNoOitTech;
}
