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

Filename: Sample.h
---------------------------------------------------------------------------*/
#pragma once
#include "Common.h"
#include <chrono>
#include <vector>
#include "Window.h"
#include "Device.h"
#include "TextRenderer.h"
#include "Gui.h"
#include "Timer.h"
#include "FullScreenPass.h"

struct SMouseData
{
	UINT Event;
	float2 Crd;	// Normalized coordinates x,y in range [-1, 1]
	INT WheelDelta;
};

class CSample
{
public:
	CSample() = default;
    CSample(const CSample&) = delete;
    CSample& operator=(const CSample&) = delete;

	void Run(const std::wstring& Title, int Width, int Height, UINT SampleCount, HICON hIcon);
	void MessageLoop();

	// Mandatory callbacks
	virtual HRESULT OnCreateDevice(ID3D11Device* pDevice) = 0;
	virtual void OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) = 0;
	virtual void OnDestroyDevice() = 0;
	virtual void OnResizeWindow() = 0;

	// Optional callbacks
	virtual bool OnKeyPress(WPARAM KeyCode);
	virtual bool OnMouseEvent(const SMouseData& Data);
	virtual void OnInitUI();

	// Some Getters
	CDevice* GetDevice() const { return m_pDevice.get(); }

protected:
    std::unique_ptr<CDevice> m_pDevice;
	std::unique_ptr<CTextRenderer> m_pTextRenderer;
	std::unique_ptr<CGui> m_pAppGui;
	const std::wstring GetGlobalSampleMessage();
	const CFullScreenPass* GetFullScreenPass();

	CWindow m_Window;
	CTimer m_Timer;
private:
    static LRESULT CALLBACK MsgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LPARAM HandleWindowsEvent(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	bool HandleKeyPress(WPARAM KeyCode);
	bool HandleMouse(UINT Msg, WPARAM wParam, LPARAM lParam);

	void RenderFrame();
	void InitUI();
	void ResizeWindow();
	void SetUiPos();

	void CreateSettingsDialog();

	std::unique_ptr<CFullScreenPass> m_pFullScreenPass;

	bool m_bVsync = false;

    struct SMouseTranslation
	{
		float2 Scale;
		static const float2 Offset;
	} m_MouseTranslation;

    struct 
    {
        std::unique_ptr<CGui> pGui;
        bool bVisible;
    } m_SettingsDialog;
};