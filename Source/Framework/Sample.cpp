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
#include "Sample.h"

CSample::CSample()
{
}

CSample::~CSample()
{
	// Destroy sample related resources

	// Destroy the device
	SAFE_DELETE(m_pDevice);

	// Destroy the window
}

void CSample::Run()
{
	// Create the window
	if (m_Window.Create() != S_OK)
	{
		PostQuitMessage(0);
		return;
	}

	// Create the device
	m_pDevice = new CDevice(m_Window.GetHeight(), m_Window.GetWidth(), m_Window.GetWindowHandle());
	assert(m_pDevice);

	// Create sample related resources

	// Enter the message loop
	MessageLoop();
}

void CSample::SetWindowParams(const WCHAR* Title, int Width, int Height)
{
	m_Window.SetParams(Title, &CSample::WindowProc, Width, Height);
}

LRESULT CALLBACK CSample::WindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Msg, wParam, lParam);
	}
	return 0;
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
	OnFrameRender(m_pDevice->GetDevice(), m_pDevice->GetImmediateContext());
	m_pDevice->Present();
}