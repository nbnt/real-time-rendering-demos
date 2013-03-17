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

    Filename: BasicLighting.cpp
---------------------------------------------------------------------------
*/
#include "DXUT.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "d3dx11effect.h"
#include "RtrMesh.h"
#include "DXUTcamera.h"
#include "NoOitTech.h"

const UINT ScreenWidth  = 1280;
const UINT ScreenHeight = 1024;

CDXUTDialogResourceManager  gDialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 gUI;                    // User interface
CDXUTTextHelper*            gpTextHelper;

CRtrModel* gpModel = NULL;
CModelViewerCamera gCamera;
ID3D11InputLayout* gpInputLayout = NULL;
CNoOitTech* gpNoOitTech = NULL;
D3DXVECTOR3 gLightDirection = D3DXVECTOR3(0, 0, 1);
float gLightIntensity = 0.5f;
float gAlphaFactor = 0.5f;

ID3D11RasterizerState* gpNoBfcRastState = NULL;
ID3D11DepthStencilState* gpNoDepthState = NULL;

// UI definitions
enum IDC_DEFINITIONS
{
    IDC_TECH_STATIC,
    IDC_TECH_COMBO_BOX,
    IDC_LIGHT_INTENSITY_STATIC,
    IDC_LIGHT_INTENSITY_SLIDER,
    IDC_BLEND_FACTOR_STATIC,
    IDC_BLEND_FACTOR_SLIDER
};

#define SLIDER_MAX_VALUE 1000

enum OIT_TECH_TYPE
{
    OIT_TECH_NONE,
    OIT_TECH_BLEND,
    OIT_TECH_DEPTH_PEELING,
    OIT_TECH_DUAL_DEPTH,
    OIT_TECH_K_BUFFER,
    OIT_TECH_LINKED_LIST,
    IDC_ENABLE_BFC_CHECKBOX,
    IDC_ENABLE_DEPTH_TEST_CHECKBOX
};

OIT_TECH_TYPE gTechType = OIT_TECH_NONE;

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch(nControlID)
    {
    case IDC_TECH_COMBO_BOX:
        {
            CDXUTComboBox* pBox = (CDXUTComboBox*)pControl;
            gTechType = (OIT_TECH_TYPE)(UINT)pBox->GetSelectedData();
        }
        break;

    case IDC_LIGHT_INTENSITY_SLIDER:
        {
            float f = (float)gUI.GetSlider(IDC_LIGHT_INTENSITY_SLIDER)->GetValue();
            gLightIntensity = f/SLIDER_MAX_VALUE;
            WCHAR w[256];
            swprintf_s(w, L"Light Intensity = %.3f", gLightIntensity);
            gUI.GetStatic(IDC_LIGHT_INTENSITY_STATIC)->SetText(w);
        }
        break;

    case IDC_BLEND_FACTOR_SLIDER:
        {
            float f = (float)gUI.GetSlider(IDC_BLEND_FACTOR_SLIDER)->GetValue();
            gAlphaFactor = f/SLIDER_MAX_VALUE;
            WCHAR w[256];
            swprintf_s(w, L"Blend Factor = %.3f", gAlphaFactor);
            gUI.GetStatic(IDC_BLEND_FACTOR_STATIC)->SetText(w);
        }
        break;

    }
}

HRESULT InitGUI(ID3D11Device* pd3dDevice)
{
    HRESULT hr = S_OK;

    gUI.Init(&gDialogResourceManager);
    gUI.SetCallback(OnGUIEvent);
    int y = 0;

    gUI.AddStatic(IDC_TECH_STATIC, L"OIT Technique", 80, y, 50, 24);
    y+= 26;

    gUI.AddComboBox(IDC_TECH_COMBO_BOX, 10, y, 240, 24);
    gUI.GetComboBox(IDC_TECH_COMBO_BOX)->AddItem(TEXT("None"), (void*)OIT_TECH_NONE);
    gUI.GetComboBox(IDC_TECH_COMBO_BOX)->AddItem(TEXT("Normal Blending"), (void*)OIT_TECH_BLEND);
    gUI.GetComboBox(IDC_TECH_COMBO_BOX)->AddItem(TEXT("Depth Peeling"), (void*)OIT_TECH_DEPTH_PEELING);
    gUI.GetComboBox(IDC_TECH_COMBO_BOX)->AddItem(TEXT("Dual Depth Peeling"), (void*)OIT_TECH_DUAL_DEPTH);
    gUI.GetComboBox(IDC_TECH_COMBO_BOX)->AddItem(TEXT("Stencil Routed K-Buffer"), (void*)OIT_TECH_K_BUFFER);
    gUI.GetComboBox(IDC_TECH_COMBO_BOX)->AddItem(TEXT("Per Pixel Linked-List"), (void*)OIT_TECH_LINKED_LIST);

    WCHAR w[1024];
    y += 30;
    swprintf_s(w, L"Light Intensity = %.3f", gLightIntensity);
    gUI.AddStatic(IDC_LIGHT_INTENSITY_STATIC, w, 80, y, 50, 24);
    y+= 26;
    gUI.AddSlider(IDC_LIGHT_INTENSITY_SLIDER, 10, y, 240, 24, 0, SLIDER_MAX_VALUE, SLIDER_MAX_VALUE/2);

    y += 30;
    swprintf_s(w, L"Blend Factor = %.3f", gLightIntensity);
    gUI.AddStatic(IDC_BLEND_FACTOR_STATIC, w, 80, y, 50, 24);
    y+= 26;
    gUI.AddSlider(IDC_BLEND_FACTOR_SLIDER, 10, y, 240, 24, 0, SLIDER_MAX_VALUE, SLIDER_MAX_VALUE/2);
    
    y += 24;
    gUI.AddCheckBox(IDC_ENABLE_BFC_CHECKBOX, L"Cull Backfacing Triangles", 10, y, 200, 24, true);
    y += 30;
    gUI.AddCheckBox(IDC_ENABLE_DEPTH_TEST_CHECKBOX, L"Enable Depth Test", 10, y, 200, 24, true);

    // Resource manager
    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    gpTextHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &gDialogResourceManager, 15 );

    // Must come last
    V_RETURN(gDialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));

    return hr;
}

void LoadMesh(ID3D11Device* pDevice)
{
    WCHAR f[1024];
    if(FAILED(DXUTFindDXSDKMediaFileCch(f, 1024, L"Stanford Models\\dragon.obj")))
    {
        trace(L"Can't find model file");
        return;
    }

    gpModel = CRtrModel::LoadModelFromFile(f, pDevice);
    if(gpModel == NULL)
    {
        return;
    }

    float fRadius = gpModel->GetRadius();
    gCamera.SetRadius( fRadius * 2, fRadius * 0.25f);

    D3DXVECTOR3 modelCenter = gpModel->GetCenter();    
    gCamera.SetModelCenter(modelCenter);

    // Create the input layout
    D3D11_INPUT_ELEMENT_DESC desc[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    desc[0].AlignedByteOffset = gpModel->GetVertexElementOffset(RTR_MESH_ELEMENT_POSITION);
    desc[1].AlignedByteOffset = gpModel->GetVertexElementOffset(RTR_MESH_ELEMENT_NORMAL);

    const D3DX11_PASS_DESC* pPassDesc = gpNoOitTech->GetPassDesc();
    HRESULT hr = pDevice->CreateInputLayout(desc, 2, pPassDesc->pIAInputSignature, pPassDesc->IAInputSignatureSize, &gpInputLayout);
    if(FAILED(hr))
    {
        trace(L"Could not create input layout");
        PostQuitMessage(0);
    }

}

//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
    gpTextHelper->Begin();
    gpTextHelper->SetInsertionPos( 2, 0 );
    gpTextHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    gpTextHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    gpTextHelper->DrawTextLine( DXUTGetDeviceStats() );
    gpTextHelper->End();
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr = S_OK;
    InitGUI(pd3dDevice);

    gpNoOitTech = CNoOitTech::Create(pd3dDevice);
    if(gpNoOitTech == NULL)
    {
        return E_INVALIDARG;
    }

    // Create rasterizer state
    D3D11_RASTERIZER_DESC RastDesc;
    ZeroMemory(&RastDesc, sizeof(RastDesc));
    RastDesc.AntialiasedLineEnable = FALSE;
    RastDesc.CullMode = D3D11_CULL_NONE;
    RastDesc.FillMode = D3D11_FILL_SOLID;

    V_RETURN(pd3dDevice->CreateRasterizerState(&RastDesc, &gpNoBfcRastState));

    // Create the depth state
    D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
    ZeroMemory(&DepthStencilDesc, sizeof(DepthStencilDesc));
    V_RETURN(pd3dDevice->CreateDepthStencilState(&DepthStencilDesc, &gpNoDepthState));

    // Load the mesh
    LoadMesh(pd3dDevice);
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( gDialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    gUI.SetLocation( pBackBufferSurfaceDesc->Width - 300, 20 );
    gUI.SetSize( 170, 170 );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    gCamera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    gCamera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    gCamera.FrameMove(fElapsedTime);
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    // Clear render target and the depth stencil 
    float ClearColor[4] = { 0.2f, 0.3f, 0.9f, 1.0f };

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

    gpNoOitTech->SetWorldMat(gCamera.GetWorldMatrix());
    D3DXMATRIX wvp = (*gCamera.GetWorldMatrix()) * (*gCamera.GetViewMatrix()) * (*gCamera.GetProjMatrix());
    gpNoOitTech->SetWVPMat(&wvp);

    gpNoOitTech->SetLightIntensity(gLightIntensity);
    D3DXVECTOR3 negLightDir = -gLightDirection;
    gpNoOitTech->SetNegLightDirW(&negLightDir);
    gpNoOitTech->SetCameraPosW(gCamera.GetEyePt());
    gpNoOitTech->SetAlphaOut((gTechType == OIT_TECH_BLEND) ? gAlphaFactor : 1.0f);

    pd3dImmediateContext->IASetInputLayout(gpInputLayout);
    bool bDepthEnabled = gUI.GetCheckBox(IDC_ENABLE_DEPTH_TEST_CHECKBOX)->GetChecked();
    bool bBFCEnabled = gUI.GetCheckBox(IDC_ENABLE_BFC_CHECKBOX)->GetChecked();

    pd3dImmediateContext->RSSetState(bBFCEnabled ? NULL : gpNoBfcRastState);
    pd3dImmediateContext->OMSetDepthStencilState(bDepthEnabled ? NULL : gpNoDepthState, 0);

    gpNoOitTech->Apply(pd3dImmediateContext, (gTechType == OIT_TECH_BLEND));
    gpModel->Draw(pd3dImmediateContext);

    gUI.OnRender(fElapsedTime);
    RenderText();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    gDialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    gDialogResourceManager.OnD3D11DestroyDevice();
    SAFE_DELETE(gpTextHelper);
    SAFE_DELETE(gpModel);
    SAFE_RELEASE(gpInputLayout);
    SAFE_DELETE(gpNoOitTech);
    SAFE_RELEASE(gpNoBfcRastState);
    SAFE_RELEASE(gpNoDepthState);
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    *pbNoFurtherProcessing = gUI.MsgProc(hWnd, uMsg, wParam, lParam);
    if( *pbNoFurtherProcessing )
    {
        return 0;
    }

    gCamera.HandleMessages(hWnd, uMsg, wParam, lParam);

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackMsgProc( MsgProc );
    
    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Perform any application-level initialization here
    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Order Independence Transparency" );

    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, ScreenWidth, ScreenHeight );
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}


