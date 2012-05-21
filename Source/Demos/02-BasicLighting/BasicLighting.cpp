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

    Filname: BasicLighting.cpp
---------------------------------------------------------------------------
*/


#include "DXUT.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "d3dx11effect.h"
#include "RtrMesh.h"
#include "DXUTcamera.h"

const UINT ScreenWidth  = 1280;
const UINT ScreenHeight = 1024;

CDXUTDialogResourceManager  gDialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 gUI;                    // User interface
CDXUTTextHelper*            gpTextHelper;

D3DXVECTOR3 gLightColor = D3DXVECTOR3(1, 1, 1);
const float gLightColorScale = 100;
D3DXVECTOR3 gLightDirection = D3DXVECTOR3(0, 0, 1);
D3DXVECTOR3 gDiffuseFactor = D3DXVECTOR3(0.5f, 0.2f, 1);
D3DXVECTOR3 gSpecFactor = D3DXVECTOR3(0.4f, 0.3f, 0.9f);
float gSurfaceSmoothness = 1;

CModelViewerCamera gCamera;

// Mesh stuff
CRtrModel* gpModel = NULL;
ID3D11InputLayout* gpInputLayout = NULL;

// Effect stuff
ID3DX11Effect* gpFX = NULL;
ID3DX11EffectTechnique* gpBasicLightTech = NULL;

ID3DX11EffectMatrixVariable* gpWorldMat = NULL;
ID3DX11EffectMatrixVariable* gpWVPMat = NULL;
ID3DX11EffectVectorVariable* gpLightColor = NULL;
ID3DX11EffectVectorVariable* gpNegLightDirW = NULL;  // Negative light direction in world space
ID3DX11EffectVectorVariable* gpCameraPosW = NULL;	
ID3DX11EffectVectorVariable* gpDiffuseFactor = NULL;
ID3DX11EffectVectorVariable* gpSpecFactor = NULL;
ID3DX11EffectScalarVariable* gpSurfaceSmoothness = NULL;

// UI definitions
enum
{
    IDC_CHOOSE_COLOR_STATIC,
    IDC_CHOOSE_RED_STATIC,
    IDC_CHOOSE_RED_SLIDER,
    IDC_CHOOSE_GREEN_STATIC,
    IDC_CHOOSE_GREEN_SLIDER,
    IDC_CHOOSE_BLUE_STATIC,
    IDC_CHOOSE_BLUE_SLIDER,
    IDC_BLINN_CHECKBOX
};

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch(nControlID)
    {
    case IDC_CHOOSE_RED_SLIDER:
        {
            float f = (float)gUI.GetSlider(IDC_CHOOSE_RED_SLIDER)->GetValue();
            gLightColor.x = f/gLightColorScale;
            WCHAR w[256];
            swprintf_s(w, L"R = %.3f", gLightColor.x);
            gUI.GetStatic(IDC_CHOOSE_RED_STATIC)->SetText(w);
        }
        break;

    case IDC_CHOOSE_GREEN_SLIDER:
        {
            float f = (float)gUI.GetSlider(IDC_CHOOSE_GREEN_SLIDER)->GetValue();
            gLightColor.y = f/gLightColorScale;
            WCHAR w[256];
            swprintf_s(w, L"G = %.3f", gLightColor.y);
            gUI.GetStatic(IDC_CHOOSE_GREEN_STATIC)->SetText(w);
        }
        break;

    case IDC_CHOOSE_BLUE_SLIDER:
        {
            float f = (float)gUI.GetSlider(IDC_CHOOSE_BLUE_SLIDER)->GetValue();
            gLightColor.z = f/gLightColorScale;
            WCHAR w[256];
            swprintf_s(w, L"B = %.3f", gLightColor.z);
            gUI.GetStatic(IDC_CHOOSE_BLUE_STATIC)->SetText(w);
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
    gUI.AddStatic(IDC_CHOOSE_COLOR_STATIC, L"Light color", 10, 0, 50, 24);
    y+= 26;
    WCHAR w[256];
    swprintf_s(w, L"R = %.3f", gLightColor.x);
    gUI.AddStatic(IDC_CHOOSE_RED_STATIC, w, -60, y, 50, 24);
    gUI.AddSlider(IDC_CHOOSE_RED_SLIDER, 20, y, 150, 20, 0, (int)gLightColorScale, (int)(gLightColorScale*gLightColor.x));
    y+=22; 
    swprintf_s(w, L"G = %.3f", gLightColor.y);
    gUI.AddStatic(IDC_CHOOSE_GREEN_STATIC, w, -60, y, 50, 24);
    gUI.AddSlider(IDC_CHOOSE_GREEN_SLIDER, 20, y, 150, 20, 0, (int)gLightColorScale, (int)(gLightColorScale*gLightColor.y));
    y+=22; 
    swprintf_s(w, L"B = %.3f", gLightColor.z);
    gUI.AddStatic(IDC_CHOOSE_BLUE_STATIC, w, -60, y, 50, 24);
    gUI.AddSlider(IDC_CHOOSE_BLUE_SLIDER, 20, y, 150, 20, 0, (int)gLightColorScale, (int)(gLightColorScale*gLightColor.z));
    y+=35; 
    gUI.AddCheckBox(IDC_BLINN_CHECKBOX, L"Use Blinn-Phong equation", -60, y, 150, 24);
    
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

    D3DX11_PASS_DESC passDesc;
    gpBasicLightTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HRESULT hr = pDevice->CreateInputLayout(desc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &gpInputLayout);
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
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr = S_OK;
    InitGUI(pd3dDevice);

    // Create the effect
    V_RETURN(D3DX11CreateEffectFromFile(L"BasicLighting.fx", pd3dDevice, &gpFX));
    gpBasicLightTech = gpFX->GetTechniqueByName("BasicLighting");
    if(gpBasicLightTech->IsValid() == FALSE)
    {
        trace(L"Can't get the technique");
        PostQuitMessage(0);
        return E_FAIL;
    }
    gpWVPMat = gpFX->GetVariableByName("gWVPMat")->AsMatrix();
    gpWorldMat = gpFX->GetVariableByName("gWorldMat")->AsMatrix();
	gpLightColor = gpFX->GetVariableByName("gLightColor")->AsVector();
    gpNegLightDirW = gpFX->GetVariableByName("gNegLightDirW")->AsVector();
    gpCameraPosW = gpFX->GetVariableByName("gCameraPosW")->AsVector();
    gpDiffuseFactor = gpFX->GetVariableByName("gDiffuseFactor")->AsVector();
    gpSpecFactor = gpFX->GetVariableByName("gSpecFactor")->AsVector();
    gpSurfaceSmoothness = gpFX->GetVariableByName("gSurfaceSmoothness")->AsScalar();

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

    gUI.SetLocation( pBackBufferSurfaceDesc->Width - 200, 20 );
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
    // Update the camera's position based on user input 
    gCamera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    // Clear render target and the depth stencil 
    float ClearColor[4] = { 0, 0, 0, 0.0f };

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

    gpWorldMat->SetMatrix((float*)gCamera.GetWorldMatrix());
    D3DXMATRIX wvp = (*gCamera.GetWorldMatrix()) * (*gCamera.GetViewMatrix()) * (*gCamera.GetProjMatrix());
    gpWVPMat->SetMatrix((float*)wvp);

	gpLightColor->SetFloatVector((float*)&(gLightColor));
    D3DXVECTOR3 negLightDir = -gLightDirection;
    gpNegLightDirW->SetFloatVector((float*)&negLightDir);
    gpCameraPosW->SetFloatVector((float*)gCamera.GetEyePt());
    gpDiffuseFactor->SetFloatVector((float*)&gDiffuseFactor);
    gpSpecFactor->SetFloatVector((float*)&gSpecFactor);
    gpSurfaceSmoothness->SetFloat(gSurfaceSmoothness);


    pd3dImmediateContext->IASetInputLayout(gpInputLayout);
    gpBasicLightTech->GetPassByIndex(0)->Apply(0, pd3dImmediateContext);
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
    SAFE_RELEASE(gpFX);
    SAFE_DELETE(gpModel);
    SAFE_RELEASE(gpInputLayout);
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
    DXUTCreateWindow( L"BasicLighting" );

    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, ScreenWidth, ScreenHeight );
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}


