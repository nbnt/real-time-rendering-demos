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

    Filename: Common.h
---------------------------------------------------------------------------
*/
#pragma once
#include "Common.h"
#include "DXUTcamera.h"

class CWireframeTech;

class CMeshLoader : public CRtrDemo
{
public:
    CMeshLoader();
    ~CMeshLoader();

    HRESULT OnCreateDevice(ID3D11Device* pDevice, CDXUTDialogResourceManager& DialogResourceManager);
    HRESULT OnResizeSwapChain(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void RenderFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, float ElapsedTime);
    void OnDestroyDevice();
    LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void* __cdecl operator new(size_t count){ return _aligned_malloc(count, 16); }
    void __cdecl operator delete(void * object) { return _aligned_free(object); }
private:
    CDXUTDialog m_UI;
    HRESULT InitGui(CDXUTDialogResourceManager& DialogResourceManager);
    void LoadModel();

    CModelViewerCamera m_Camera;
    bool m_bWireframe;
    UINT m_VertexCount;

    CWireframeTech* m_pWireframeTech;
    CDxModel* m_pModel;
};