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

CWindow::CWindow() : m_hWnd(nullptr), m_Title(nullptr), m_WndProc(nullptr), m_ClientWidth(0), m_ClientHeight(0)
{
}

CWindow::~CWindow()
{
	if(m_hWnd)
	{
		DestroyWindow(m_hWnd);
	}
}

HRESULT CWindow::Create(HICON hIcon, void* pUserData)
{
	const WCHAR* ClassName = L"RtrSampleWindow";
	// Register the window class
	WNDCLASS wc = {};
	wc.lpfnWndProc = m_WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = ClassName;
	wc.hIcon = hIcon;

	if (RegisterClass(&wc) == 0)
	{
		verify_return(GetLastError());
	}

	// Window size we have is for client area, calculate actual window size
	RECT r{ 0, 0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&r, m_WindowStyle, false);

	int Width = r.right - r.left;
	int Height = r.bottom - r.top;

	// Create the window
	m_hWnd = CreateWindowEx(0, ClassName, m_Title, m_WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, nullptr, nullptr, wc.hInstance, pUserData);
	if (m_hWnd == nullptr)
	{
		verify_return(GetLastError());
	}

	// Show the window
	ShowWindow(m_hWnd, SW_SHOW);

	return S_OK;
}

void CWindow::SetParams(const WCHAR* Title, WNDPROC WndProc, int Width, int Height)
{
	m_Title = Title;
	m_WndProc = WndProc;
	m_ClientWidth = Width;
	m_ClientHeight = Height;
}

void CWindow::Resize()
{
	RECT r;
	GetClientRect(m_hWnd, &r);
	m_ClientWidth = r.right - r.left;
	m_ClientHeight = r.bottom - r.top;
}