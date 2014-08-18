/*
---------------------------------------------------------------------------
Real Time Rendering Demos
---------------------------------------------------------------------------

Copyright (c) 2011 - Nir Benty

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

Filename: Common.cpp
---------------------------------------------------------------------------
*/

#include "Common.h"
#include "SDKmisc.h"
#include "DXUTsettingsdlg.h"

CD3DSettingsDlg gSettingsDialog;
CDXUTDialogResourceManager  gDialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 gHud;                    // User interface
CDXUTTextHelper*            gpTextHelper;
CRtrDemo* pDemo = nullptr;

// UI definitions
#define IDC_SETTINGS_DIALOG 1

static void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
    switch(nControlID)
    {
    case IDC_SETTINGS_DIALOG:
        gSettingsDialog.SetActive(!gSettingsDialog.IsActive());
        break;
    }
}

HRESULT InitGUI()
{
    HRESULT hr = S_OK;
    UINT Height = 0;

    gSettingsDialog.Init(&gDialogResourceManager);

    gHud.Init(&gDialogResourceManager);
    gHud.SetCallback(OnGUIEvent);
    gHud.AddButton(IDC_SETTINGS_DIALOG, L"Settings", 0, Height, 170, 30, VK_F2);
    Height += 40;

    return hr;
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    gpTextHelper->Begin();
    gpTextHelper->SetInsertionPos(2, 0);
    gpTextHelper->SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
    gpTextHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
    gpTextHelper->DrawTextLine(DXUTGetDeviceStats());
    gpTextHelper->DrawTextLine(L"Press F2 to change device settings");
    gpTextHelper->End();
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                      DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr = S_OK;
    // Init GUI and resource manager
    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    gpTextHelper = new CDXUTTextHelper(pDevice, pd3dImmediateContext, &gDialogResourceManager, 15);

    // Must come last
    V_RETURN(gDialogResourceManager.OnD3D11CreateDevice(pDevice, pd3dImmediateContext));
    V_RETURN(gSettingsDialog.OnD3D11CreateDevice(pDevice));

    pDemo = CreateRtrDemo();
    return pDemo->OnCreateDevice(pDevice, gDialogResourceManager);
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    V_RETURN(gDialogResourceManager.OnD3D11ResizedSwapChain(pDevice, pBackBufferSurfaceDesc));
    V_RETURN(gSettingsDialog.OnD3D11ResizedSwapChain(pDevice, pBackBufferSurfaceDesc));

    gHud.SetLocation(20, 100);
    gHud.SetSize(170, 170);

    return pDemo->OnResizeSwapChain(pDevice, pBackBufferSurfaceDesc);
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pImmediateContext, double Time, float ElapsedTime, void* pUserContext)
{
    if(gSettingsDialog.IsActive())
    {
        gSettingsDialog.OnRender(ElapsedTime);
        return;
    }

    pDemo->RenderFrame(pDevice, pImmediateContext, ElapsedTime);
    RenderText();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
    gDialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
    gSettingsDialog.OnD3D11DestroyDevice();

    SAFE_DELETE(gpTextHelper);

    gDialogResourceManager.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();

    pDemo->OnDestroyDevice();
    SAFE_DELETE(pDemo);

    // The next 2 lines clears and flush the state
    DXUTGetD3D11DeviceContext()->ClearState();
    DXUTGetD3D11DeviceContext()->Flush();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                         bool* pbNoFurtherProcessing, void* pUserContext)
{
    if(gSettingsDialog.IsActive())
    {
        gSettingsDialog.MsgProc(hWnd, uMsg, wParam, lParam);
        return 0;
    }

    *pbNoFurtherProcessing = gHud.MsgProc(hWnd, uMsg, wParam, lParam);
    if(*pbNoFurtherProcessing)
    {
        return 0;
    }

    if(pDemo)
    {
        pDemo->MsgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved(void* pUserContext)
{
    return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int RtrMain(WCHAR* windowName, UINT height, UINT width, D3D_FEATURE_LEVEL featureLevel)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove(OnFrameMove);
    DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
    DXUTSetCallbackDeviceRemoved(OnDeviceRemoved);

    DXUTSetCallbackMsgProc(MsgProc);
    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
    DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
    DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
    DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
    DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
    DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

    // Perform any application-level initialization here

    DXUTInit(true, true, NULL); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings(true, true); // Show the cursor and clip it when in full screen
    DXUTCreateWindow(windowName);

    InitGUI();

    // Only require 10-level hardware
    DXUTCreateDevice(featureLevel, true, height, width);
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here
    return DXUTGetExitCode();
}


