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

Filename: WireframeTech.cpp
---------------------------------------------------------------------------
*/

#include "WireframeTech.h"

CWireframeTech::CWireframeTech(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
    // Compile the shaders using the lowest possible profile for broadest feature level support
    V(DXUTCompileShaderFromFile(L"01-MeshLoader\\01-Wireframe.hlsl", "VSMain", NULL, "vs_4_0_level_9_1", &m_pVSBlob));
    V(DXUTCompileShaderFromFile(L"01-MeshLoader\\01-Wireframe.hlsl", "PSMain", NULL, "ps_4_0_level_9_1", &m_pPSBlob));

    // Create the shaders
    V(pDevice->CreateVertexShader(m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize(), NULL, &m_pVS));
    DXUT_SetDebugName(m_pVS, "WireframeVSMain");
    V(pDevice->CreatePixelShader(m_pPSBlob->GetBufferPointer(), m_pPSBlob->GetBufferSize(), NULL, &m_pPS));
    DXUT_SetDebugName(m_pPS, "WireframePSMain");

    D3D11_RASTERIZER_DESC rast;
    rast.AntialiasedLineEnable = TRUE;
    rast.FillMode = D3D11_FILL_WIREFRAME;
    rast.CullMode = D3D11_CULL_NONE;
    rast.DepthBias = 0;
    rast.DepthBiasClamp = 0;
    rast.DepthClipEnable = FALSE;
    rast.FrontCounterClockwise = FALSE;
    rast.MultisampleEnable = FALSE;
    rast.ScissorEnable = FALSE;
    rast.SlopeScaledDepthBias = 0;
    V(pDevice->CreateRasterizerState(&rast, &m_pRastState));
}

CWireframeTech::~CWireframeTech()
{
    SAFE_RELEASE(m_pVSBlob);
    SAFE_RELEASE(m_pPSBlob);
    SAFE_RELEASE(m_pVS);
    SAFE_RELEASE(m_pPS);
    SAFE_RELEASE(m_pLayout);
    SAFE_RELEASE(m_pRastState);
}