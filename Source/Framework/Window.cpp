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

Filename: CWindow.cpp
---------------------------------------------------------------------------*/
#include "Window.h"

CWindow::CWindow() : m_hWnd(nullptr), m_ClientWidth(0), m_ClientHeight(0)
{
}

CWindow::~CWindow()
{
	if(m_hWnd)
	{
		DestroyWindow(m_hWnd);
	}
}

HRESULT CWindow::Create(const std::wstring& Title, WNDPROC WndProc, int ClientRectWidth, int ClientRectHeight, HICON hIcon, void* pUserData)
{
	const std::wstring ClassName = L"RtrSampleWindow";
	// Register the window class
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = ClassName.c_str();
	wc.hIcon = hIcon;

	if (RegisterClass(&wc) == 0)
	{
		verify_return(GetLastError());
	}

	// Window size we have is for client area, calculate actual window size
	m_ClientWidth = ClientRectWidth;
	m_ClientHeight = ClientRectHeight;
	RECT r{ 0, 0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&r, m_WindowStyle, false);

	int WindowWidth = r.right - r.left;
	int WindowHeight = r.bottom - r.top;

	// Create the window
	m_hWnd = CreateWindowEx(0, ClassName.c_str(), Title.c_str(), m_WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight, nullptr, nullptr, wc.hInstance, pUserData);
	if (m_hWnd == nullptr)
	{
		verify_return(GetLastError());
	}

    // It might be tempting to call ShowWindow() here, but this fires a WM_SIZE message, which if you look at our CSample::MsgProc()
    // calls some device functions. That's a race condition, since the device isn't necessrily initialized yet. 
	return S_OK;
}

void CWindow::Show()
{
    ShowWindow(m_hWnd, SW_SHOW);
}

void CWindow::Resize()
{
	RECT r;
	GetClientRect(m_hWnd, &r);
	m_ClientWidth = r.right - r.left;
	m_ClientHeight = r.bottom - r.top;
}