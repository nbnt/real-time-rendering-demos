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
#include "StringUtils.h"
#include <sstream>
#include <vector>

UINT CGui::m_RefCount = 0;

CGui::CGui(const std::string& Caption, ID3D11Device* pDevice, int Width, int Height, bool bVisible)
{
	if(m_RefCount == 0)
	{ 
		++m_RefCount;
		TwInit(TW_DIRECT3D11, pDevice);
		TwWindowSize(Width, Height);
	}
	m_pTwBar = TwNewBar(Caption.c_str());
	assert(m_pTwBar);
    SetVisibility(bVisible);
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

bool CGui::MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (TwEventWin(hwnd, uMsg, wParam, lParam) != 0);
}

void CGui::DrawAll()
{
	TwDraw();
}

void CGui::SetGlobalHelpMessage(const std::string& Msg)
{
	std::string TwMsg = std::string(" GLOBAL help='") + Msg + "' ";
	TwDefine(TwMsg.c_str());
	TwDefine(" GLOBAL fontsize=3");
}

std::string GetGroupParamString(const std::string& Group)
{
	std::string tmp;
	if(Group.size() != 0)
	{
		tmp = " group=\""+ Group + "\"";
	}
	return tmp;
}
void CGui::AddButton(const std::string& Name, GuiButtonCallback Callback, void* pUserData, const std::string& Group)
{
	int res = TwAddButton(m_pTwBar, Name.c_str(), Callback, pUserData, GetGroupParamString(Group).c_str());
	if(res == 0)
	{
		DisplayTwError(L"Error when creating button \"" + string_2_wstring(Name) + L"\"");
	}
}

void CGui::AddCheckBox(const std::string& Name, bool* pVar, const std::string& Group)
{
	int res = TwAddVarRW(m_pTwBar, Name.c_str(), TW_TYPE_BOOL8, pVar, GetGroupParamString(Group).c_str());
	if(res == 0)
	{
		DisplayTwError(L"Error when creating checkbox \"" + string_2_wstring(Name) + L"\"");
	}
}

void CGui::AddDir3FVar(const std::string& Name, float3* pVar, const std::string& Group)
{
	std::string params = "axisz=-z " + GetGroupParamString(Group);
	int res = TwAddVarRW(m_pTwBar, Name.c_str(), TW_TYPE_DIR3F, pVar, params.c_str());
	if(res == 0)
	{
		DisplayTwError(L"Error when creating Dir3Var \"" + string_2_wstring(Name) + L"\"");
	}
}

void CGui::AddRgbColor(const std::string& Name, float3* pVar, const std::string& Group)
{
	int res = TwAddVarRW(m_pTwBar, Name.c_str(), TW_TYPE_COLOR3F, pVar, GetGroupParamString(Group).c_str());
	if(res == 0)
	{
		DisplayTwError(L"Error when creating Dir3Var \"" + string_2_wstring(Name) + L"\"");
	}
}

void CGui::AddFloatVar(const std::string& Name, float* pVar, const std::string& Group, float Min, float Max, float Step)
{
	std::stringstream ss;
	ss << " min=" << Min << " max=" << Max << " step=" << Step << GetGroupParamString(Group);
	const auto& param = ss.str();
	int res = TwAddVarRW(m_pTwBar, Name.c_str(), TW_TYPE_FLOAT, pVar, param.c_str());
	if(res == 0)
	{
		DisplayTwError(L"Error when creating float var \"" + string_2_wstring(Name) + L"\"");
	}
}

std::vector<TwEnumVal> ConvertDropdownList(const CGui::dropdown_list& GuiList)
{
	std::vector<TwEnumVal> TwList;
	for(const auto& GuiVal : GuiList)
	{
		TwEnumVal TwVal;
		TwVal.Label = GuiVal.Label.c_str();
		TwVal.Value = GuiVal.Value;
		TwList.push_back(TwVal);
	}

	return TwList;
}

void CGui::AddDropdown(const std::string& Name, const dropdown_list& Values, void* pVar, const std::string& Group)
{
	auto TwList = ConvertDropdownList(Values);
	TwType enumType = TwDefineEnum(Name.c_str(), &TwList[0], UINT(TwList.size()));
	int res = TwAddVarRW(m_pTwBar, Name.c_str(), enumType, pVar, GetGroupParamString(Group).c_str());
	if(res == 0)
	{
		DisplayTwError(L"Error when creating dropdown \"" + string_2_wstring(Name) + L"\"");
	}
}

void CGui::AddDropdownWithCallback(const std::string& Name, const dropdown_list& Values, TwSetVarCallback SetCallback, TwGetVarCallback GetCallback, void* pUserData, const std::string& Group)
{
	auto TwList = ConvertDropdownList(Values);
	TwType enumType = TwDefineEnum(Name.c_str(), &TwList[0], UINT(TwList.size()));
	int res = TwAddVarCB(m_pTwBar, Name.c_str(), enumType, SetCallback, GetCallback, pUserData, GetGroupParamString(Group).c_str());
	if(res == 0)
	{
		DisplayTwError(L"Error when creating dropdown with callback\"" + string_2_wstring(Name) + L"\"");
	}
}

void CGui::SetVisibility(bool bVisible)
{
	int v = bVisible ? 1 : 0;
	TwSetParam(m_pTwBar, nullptr, "visible", TW_PARAM_INT32, 1, &v);
}

void CGui::SetVarVisibility(const std::string& Name, bool bVisible)
{
	TwSetParam(m_pTwBar, Name.c_str(), "visible", TW_PARAM_CSTRING, 1, bVisible ? "true" : "false");
}

void CGui::RemoveVar(const std::string& Name)
{
	TwRemoveVar(m_pTwBar, Name.c_str());
}

void CGui::Refresh() const
{
	TwRefreshBar(m_pTwBar);
}
