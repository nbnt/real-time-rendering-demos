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

    Filename: Common.cpp
---------------------------------------------------------------------------
*/
#include "EmptyProject.h"

enum
{
    IDC_LOAD_MESH,
};

static void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
    switch(nControlID)
    {
    case IDC_LOAD_MESH:
        MessageBox(nullptr, L"Button pressed", L"Load Mesh", MB_OK);
        break;
    }
}

HRESULT CEmptyProjectDemo::InitGui(CDXUTDialogResourceManager& DialogResourceManager)
{
    UINT Height = 0;
    m_UI.Init(&DialogResourceManager);
    m_UI.SetCallback(OnGUIEvent, this);
    m_UI.AddButton(IDC_LOAD_MESH, L"Load Mesh", 0, Height, 170, 30, VK_F2);
    Height += 40;

    return S_OK;
}

HRESULT CEmptyProjectDemo::OnCreateDevice(ID3D11Device* pDevice, CDXUTDialogResourceManager& DialogResourceManager)
{
    InitGui(DialogResourceManager);
    return S_OK;
}

HRESULT CEmptyProjectDemo::OnResizeSwapChain(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    UINT x = pBackBufferSurfaceDesc->Width - 250;
    UINT y = 20;
    m_UI.SetLocation(x, y);
    m_UI.SetSize(100, 250);

    return S_OK;
}

void CEmptyProjectDemo::RenderFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, float ElapsedTime)
{
    // Clear render target and the depth stencil 
    float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pContext->ClearRenderTargetView(pRTV, ClearColor);
    pContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

    m_UI.OnRender(ElapsedTime);
}

LRESULT CEmptyProjectDemo::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return m_UI.MsgProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    return RtrMain(L"Empty Project", 1280, 1024, D3D_FEATURE_LEVEL_10_0);
}

CRtrDemo* CreateRtrDemo()
{
    return new CEmptyProjectDemo;
}