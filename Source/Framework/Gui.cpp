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

Filename: Gui.cpp
---------------------------------------------------------------------------*/
#include "Gui.h"

UINT CGui::m_RefCount = 0;

CGui::CGui(const std::string& Caption, ID3D11Device* pDevice, int Width, int Height)
{
	if(m_RefCount == 0)
	{ 
		++m_RefCount;
		TwInit(TW_DIRECT3D11, pDevice);
		TwWindowSize(Width, Height);
	}
	m_pTwBar = TwNewBar(Caption.c_str());
	assert(m_pTwBar);
}

CGui::~CGui()
{
	--m_RefCount;
	if(m_RefCount == 0)
	{
		TwTerminate();
	}
}

void CGui::DisplayTwError(const std::wstring& Prefix)
{
	std::string Error(TwGetLastError());
	trace(std::wstring(Prefix + L"\n" + string_2_wstring(Error)));
}

void CGui::GetSize(INT32 Size[2]) const
{
	TwGetParam(m_pTwBar, nullptr, "size", TW_PARAM_INT32, 2, Size);
}

void CGui::GetPosition(INT32 Position[2]) const
{
	TwGetParam(m_pTwBar, nullptr, "position", TW_PARAM_INT32, 2, Position);
}

void CGui::SetSize(const INT32 Size[2])
{
	TwSetParam(m_pTwBar, nullptr, "size", TW_PARAM_INT32, 2, Size);
}

void CGui::SetPosition(const INT32 Position[2])
{
	TwSetParam(m_pTwBar, nullptr, "position", TW_PARAM_INT32, 2, Position);
}

int CGui::MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return TwEventWin(hwnd, uMsg, wParam, lParam);
}

void CGui::DrawAll()
{
	TwDraw();
}

void CGui::SetGlobalHelpMessage(const std::string& Msg)
{
	std::string TwMsg = std::string(" GLOBAL help='") + Msg + "' ";
	TwDefine(TwMsg.c_str());
	TwDefine(" GLOBAL fontsize=4");
}

void CGui::AddButton(const std::string& Name, GuiButtonCallback Callback, void* pUserData)
{
	int res = TwAddButton(m_pTwBar, Name.c_str(), Callback, pUserData, nullptr);
	if(res == 0)
	{
		DisplayTwError(L"Error when creating button \"" + string_2_wstring(Name) + L"\"");
	}
}