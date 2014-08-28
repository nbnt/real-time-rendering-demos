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

Filename: Device.cpp
---------------------------------------------------------------------------*/
#include "Device.h"
#include "Common.h"

CDevice::CDevice(CWindow& Window) : m_Window(Window)
{
	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


	m_BackBufferHeight = Window.GetClientHeight();
	m_BackBufferWidth = Window.GetClientWidth();

	DXGI_SWAP_CHAIN_DESC SwapChainDesc = { 0 };
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	SwapChainDesc.BufferDesc.Height = m_BackBufferHeight;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Width = m_BackBufferWidth;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.Flags = 0;
	SwapChainDesc.OutputWindow = Window.GetWindowHandle();
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Windowed = TRUE;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&m_pSwapChain,
		&m_pDevice,
		&m_FeatureLevel,
		&m_pContext
		);

	verify(hr);

	CreateResourceViews();

}

void CDevice::Present(bool bVsync)
{

	m_bWindowOccluded = (m_pSwapChain->Present(bVsync ? 1 : 0, 0) == DXGI_STATUS_OCCLUDED);
}

bool CDevice::IsWindowOccluded()
{
	if(m_bWindowOccluded)
	{
		m_bWindowOccluded = (m_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED);
	}
	return m_bWindowOccluded;
}

void CDevice::ResizeWindow()
{
	m_Window.Resize();
	if((m_Window.GetClientHeight() != m_BackBufferHeight) || (m_Window.GetClientWidth() != m_BackBufferWidth))
	{
		m_pContext->ClearState();
		m_BackBufferWidth = m_Window.GetClientWidth();
		m_BackBufferHeight = m_Window.GetClientHeight();

		m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
		m_pRtv.Release();
		m_pDsv.Release();
		m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0);
		CreateResourceViews();
	}
}

void CDevice::CreateResourceViews()
{
	// Create the render target view
	ID3D11ResourcePtr pBB;
	verify(m_pSwapChain->GetBuffer(0, __uuidof(pBB), reinterpret_cast<void**>(&pBB)));
	verify(m_pDevice->CreateRenderTargetView(pBB, nullptr, &m_pRtv));

	// Create the depth stencil resource and view
	D3D11_TEXTURE2D_DESC DepthDesc;
	DepthDesc.ArraySize = 1;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthDesc.CPUAccessFlags = 0;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.Height = m_BackBufferHeight;
	DepthDesc.Width = m_BackBufferWidth;
	DepthDesc.MipLevels = 1;
	DepthDesc.MiscFlags = 0;
	DepthDesc.SampleDesc.Count = 1;
	DepthDesc.SampleDesc.Quality = 0;
	DepthDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2DPtr pDepthResource;
	verify(m_pDevice->CreateTexture2D(&DepthDesc, nullptr, &pDepthResource));
	verify(m_pDevice->CreateDepthStencilView(pDepthResource, nullptr, &m_pDsv));

	// Set the viewport
	D3D11_VIEWPORT vp;
	vp.Height = (float)m_BackBufferHeight;
	vp.Width = (float)m_BackBufferWidth;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pContext->RSSetViewports(1, &vp);
}