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

Filename: Sample.cpp
---------------------------------------------------------------------------*/
#include <sstream>
#include "Sample.h"
#include "Font.h"
#include <Windowsx.h>

const float2 CSample::m_CoordsOffset = float2(-1, 1);

void CSample::Run(HICON hIcon)
{
	// Create the window
	if (m_Window.Create(hIcon, this) != S_OK)
	{
		PostQuitMessage(0);
		return;
	}

	// Create the device
	m_pDevice = std::make_unique<CDevice>(m_Window);
	assert(m_pDevice);

	// Create UI
	InitUI();	

    // Create font and text helper
    std::unique_ptr<CFont> pFont = std::make_unique<CFont>(m_pDevice->GetD3DDevice());
	m_pTextRenderer = std::make_unique<CTextRenderer>(m_pDevice->GetD3DDevice());
	m_pTextRenderer->SetFont(pFont);

	// Call resize window
	ResizeWindow();	
	m_Timer.ResetClock();

	OnCreateDevice(m_pDevice->GetD3DDevice());

	// Enter the message loop
	MessageLoop();

	// Shutdown
	m_pDevice->GetImmediateContext()->ClearState();
	OnDestroyDevice();
}

void CSample::InitUI()
{
	m_pGui = std::make_unique<CGui>("Sample UI", m_pDevice->GetD3DDevice(), m_Window.GetClientWidth(), m_Window.GetClientHeight());
	OnInitUI();
}

void CSample::SetWindowParams(const WCHAR* Title, int Width, int Height)
{
	m_Window.SetParams(Title, &CSample::MsgProc, Width, Height);
}

LRESULT CALLBACK CSample::MsgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CSample* pSample;
	// If this is a create message, store the pointer to the sample object in the user-data window space
	if (Msg == WM_CREATE)
	{
		CREATESTRUCT* pCreateStruct = (CREATESTRUCT*)lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCreateStruct->lpCreateParams);
		return DefWindowProc(hwnd, Msg, wParam, lParam);
	}
	else
	{
		pSample = (CSample*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if(pSample)
	{
		return pSample->HandleWindowsEvent(hwnd, Msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hwnd, Msg, wParam, lParam);
	}
}

void CSample::MessageLoop()
{
	MSG Msg;
	while (1)
	{
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
			if (Msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			RenderFrame();
		}
	}
}

void CSample::RenderFrame()
{
	m_Timer.Tick();
	if(m_pDevice->IsWindowOccluded() == false)
	{
		ID3D11DeviceContext* pCtx = m_pDevice->GetImmediateContext();
		ID3D11RenderTargetView* pRTV = m_pDevice->GetBackBufferRTV();

		// Bind RTV and DSV
		pCtx->OMSetRenderTargets(1, &pRTV, m_pDevice->GetBackBufferDSV());

		OnFrameRender(m_pDevice->GetD3DDevice(), m_pDevice->GetImmediateContext());

		CGui::DrawAll();

		m_pDevice->Present(m_bVsync);
	}
}

void CSample::ResizeWindow()
{
	m_pDevice->ResizeWindow();
	SetUiPos();
	m_Timer.ResetClock();
	m_CoordsScale = float2(2 / float(m_Window.GetClientWidth()), -2 / float(m_Window.GetClientHeight()));
	OnResizeWindow();
}

void CSample::SetUiPos()
{
	int BarSize[2];
	m_pGui->GetSize(BarSize);
	int BarPosition[2] = { m_Window.GetClientWidth() - 10 - BarSize[0], 10 };
	m_pGui->SetPosition(BarPosition);
}

const std::wstring CSample::GetFPSString()
{
	std::wstringstream ss;
	ss << INT(ceil(m_Timer.CalcFps())) << " FPS. ";
	ss << "VSYNC " << (m_bVsync ? "ON" : "OFF" ) << ", Press 'V' to toggle.";
	return ss.str();
}

LPARAM CSample::HandleWindowsEvent(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// Let the UI handle events
	if(CGui::MsgProc(hwnd, Msg, wParam, lParam))
	{
		return 0;
	}

	// Handle rest of event
	switch(Msg)
	{
	case WM_SIZE:
		if(wParam != SIZE_MINIMIZED)
		{
			ResizeWindow();
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		HandleKeyPress(wParam);
		break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEWHEEL:
		HandleMouse(Msg, wParam, lParam);
		break;
	default:
		return DefWindowProc(hwnd, Msg, wParam, lParam);
	}
	return 0;
}

void CSample::HandleMouse(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	SMouseData data;
	data.Event = Msg;
	data.WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

	data.Crd = float2(float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam)));
	data.Crd *= m_CoordsScale;
	data.Crd += m_CoordsOffset;

	OnMouseEvent(data);
}

void CSample::HandleKeyPress(WPARAM KeyCode)
{
	if (OnKeyPress(KeyCode))
	{
		return;
	}

	switch(KeyCode)
	{
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	case 'V':
		m_bVsync = !m_bVsync;
		m_Timer.ResetClock();
		break;
	}
}

bool CSample::OnKeyPress(WPARAM KeyCode)
{
	return false;
}

bool CSample::OnMouseEvent(const SMouseData& Data)
{
	return false;
}

void CSample::OnInitUI()
{

}