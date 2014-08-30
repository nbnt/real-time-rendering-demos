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

Filename: Gui.h
---------------------------------------------------------------------------*/
#pragma once
#include "Common.h"
#include <memory>
#include "AntTweakBar.h"

#define GUI_CALL TW_CALL
using GuiButtonCallback = TwButtonCallback;

class CGui
{
public:
	CGui(const std::string& Caption, ID3D11Device* pDevice, int Width, int Height);
	~CGui();
	static bool MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void DrawAll();
	static void SetGlobalHelpMessage(const std::string& Msg);

	// UI Properties
	void GetSize(INT32 Size[2]) const;
	void GetPosition(INT32 Position[2]) const;

	void SetSize(const INT32 Size[2]);
	void SetPosition(const INT32 Position[2]);

	// UI Elements
	void AddButton(const std::string& Name, GuiButtonCallback Callback, void* pUserData);
	void AddCheckBox(const std::string& Name, bool* pVar);
	void AddDir3FVar(const std::string& Name, float3* pVar);
	void AddRgbColor(const std::string& Name, float3* pVar);

private:
	void DisplayTwError(const std::wstring& Prefix);
	static UINT m_RefCount;
	TwBar* m_pTwBar;
};