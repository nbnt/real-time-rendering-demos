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

Filename: Device.h
---------------------------------------------------------------------------*/
#pragma once
#include "Window.h"

class CDevice
{
public:
	CDevice(CWindow& Window, UINT SampleCount = 1);

	ID3D11RenderTargetView* GetBackBufferRTV() const { return m_pRtv; }
	ID3D11DepthStencilView* GetBackBufferDSV() const { return m_pDsv; }
	ID3D11Device* GetD3DDevice() const { return m_pDevice; }
	ID3D11DeviceContext* GetImmediateContext() { return m_pContext; }
		
	void ResizeWindow();
	bool IsWindowOccluded();
	void Present(bool bVsync);

	void SetSampleCount(UINT SampleCount, HWND hwnd);

private:
	void CreateResourceViews();

	CWindow& m_Window;
	ID3D11DevicePtr m_pDevice;
	ID3D11DeviceContextPtr m_pContext;
	IDXGISwapChainPtr m_pSwapChain;
	D3D_FEATURE_LEVEL m_FeatureLevel;

	ID3D11RenderTargetViewPtr m_pRtv;
	ID3D11DepthStencilViewPtr m_pDsv;
	bool m_bWindowOccluded;
	UINT m_BackBufferWidth;
	UINT m_BackBufferHeight;
	UINT m_SampleCount;
};