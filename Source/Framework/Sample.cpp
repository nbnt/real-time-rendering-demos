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

const float2 CSample::SMouseTranslation::Offset = float2(-1, 1);

void TW_CALL GetSampleCountCallback(void *value, void *pUserData)
{
	CSample* pSample = (CSample*)(pUserData);
	*(UINT*)value = pSample->GetDevice()->GetSampleCount();
}

void TW_CALL SetSampleCountCallback(const void *value, void *pUserData)
{
	CSample* pSample = (CSample*)(pUserData);
	UINT SampleCount = *(UINT*)value;
	pSample->GetDevice()->SetSampleCount(SampleCount);
}

void CSample::Run(const std::wstring& Title, int Width, int Height, UINT SampleCount, HICON hIcon)
{
	// Create the window
	if (m_Window.Create(Title, CSample::MsgProc, Width, Height, hIcon, this)!= S_OK)
	{
		PostQuitMessage(0);
		return;
	}

	// Create the device
	m_pDevice = std::make_unique<CDevice>(m_Window, SampleCount);
	assert(m_pDevice);

    // Create UI
    CreateSettingsDialog();
    m_pAppGui = std::make_unique<CGui>("Sample UI", m_pDevice->GetD3DDevice(), m_Window.GetClientWidth(), m_Window.GetClientHeight());

    // Create font and text helper
    std::unique_ptr<CFont> pFont = std::make_unique<CFont>(m_pDevice->GetD3DDevice());
	m_pTextRenderer = std::make_unique<CTextRenderer>(m_pDevice->GetD3DDevice());
	m_pTextRenderer->SetFont(pFont);

	// Call resize window
    m_Window.Show();
	OnCreateDevice(m_pDevice->GetD3DDevice());

	// Enter the message loop
	MessageLoop();

	// Shutdown
	m_pDevice->GetImmediateContext()->ClearState();
	OnDestroyDevice();
}

void CSample::CreateSettingsDialog()
{
    m_SettingsDialog.bVisible = false;
    m_SettingsDialog.pGui = std::make_unique<CGui>("Device Settings", m_pDevice->GetD3DDevice(), m_Window.GetClientWidth(), m_Window.GetClientHeight(), m_SettingsDialog.bVisible);

	INT32 Size[2] = { 200, 100 };
    m_SettingsDialog.pGui->SetSize(Size);
	INT32 Pos[2];
    m_SettingsDialog.pGui->GetPosition(Pos);
	Pos[1] += 100;
    m_SettingsDialog.pGui->SetPosition(Pos);

	auto SupportedSamples = m_pDevice->GetSupportedSampleCount();
	CGui::dropdown_list SampleList;

	for(auto SampleCount : SupportedSamples)
	{
		CGui::SDropdownValue val;
		val.Value = SampleCount;
		val.Label = std::to_string(SampleCount);
		SampleList.push_back(val);
	}

    m_SettingsDialog.pGui->AddDropdownWithCallback("Sample Count", SampleList, SetSampleCountCallback, GetSampleCountCallback, this);
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
	m_MouseTranslation.Scale = float2(2 / float(m_Window.GetClientWidth()), -2 / float(m_Window.GetClientHeight()));
	OnResizeWindow();
}

void CSample::SetUiPos()
{
	int BarSize[2];
	m_pAppGui->GetSize(BarSize);
    BarSize[0] += 30;
    m_pAppGui->SetSize(BarSize);
	int BarPosition[2] = { m_Window.GetClientWidth() - 10 - BarSize[0], 10 };
	m_pAppGui->SetPosition(BarPosition);
}

const std::wstring CSample::GetGlobalSampleMessage()
{
    WCHAR fpsStr[1024];
    float fps = m_Timer.CalcFps();
    float spf = 1/fps;
    swprintf_s(fpsStr, ARRAYSIZE(fpsStr), L"%.2f FPS(%.3fms)", fps, spf);
    std::wstring str(fpsStr);
    str += std::wstring(L"VSYNC ") + (m_bVsync ? L"ON" : L"OFF") + L", Press 'V' to toggle\n";
	str += L"Press F2 for device settings dialog";
	return str;
}

LPARAM CSample::HandleWindowsEvent(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(CGui::MsgProc(hwnd, Msg, wParam, lParam))
	{
		return 0;
	}

	bool bHandled = false;
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
		bHandled = true;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		bHandled = true;
		break;
	case WM_KEYDOWN:
		bHandled = HandleKeyPress(wParam);
		break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEWHEEL:
		bHandled = HandleMouse(Msg, wParam, lParam);
		break;

	}
	if(bHandled == false)
	{
		return DefWindowProc(hwnd, Msg, wParam, lParam);
	}
	return bHandled ? 0 : 1;
}

bool CSample::HandleMouse(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	SMouseData data;
	data.Event = Msg;
	data.WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

	data.Crd = float2(float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam)));
	data.Crd *= m_MouseTranslation.Scale;
	data.Crd += m_MouseTranslation.Offset;

	return OnMouseEvent(data);
}

bool CSample::HandleKeyPress(WPARAM KeyCode)
{
	if (OnKeyPress(KeyCode))
	{
		return true;
	}

	bool bHandled = true;
	switch(KeyCode)
	{
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	case 'V':
		m_bVsync = !m_bVsync;
		m_Timer.ResetClock();
		break;
	case VK_F2:
        m_SettingsDialog.bVisible = !m_SettingsDialog.bVisible;
        m_SettingsDialog.pGui->SetVisibility(m_SettingsDialog.bVisible);
		break;
	default:
		bHandled = false;
	}

	return bHandled;
}

bool CSample::OnKeyPress(WPARAM KeyCode)
{
	return false;
}

bool CSample::OnMouseEvent(const SMouseData& Data)
{
	return false;
}

const CFullScreenPass* CSample::GetFullScreenPass()
{
	if(m_pFullScreenPass.get() == nullptr)
	{
		m_pFullScreenPass = std::make_unique<CFullScreenPass>(m_pDevice->GetD3DDevice());
	}
	return m_pFullScreenPass.get();
}