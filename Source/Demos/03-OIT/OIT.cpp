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
#include "OitTech.h"
#include "FullScreenPass.h"

const float gClearColor[4] = { 0.2f, 0.3f, 0.9f, 1.0f };

#define MESH_FILE L"orbiter bugship\\orbiter bugship.obj"
const UINT ScreenWidth  = 1280;
const UINT ScreenHeight = 1024;

CDXUTDialogResourceManager  gDialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 gUI;                    // User interface
CDXUTTextHelper*            gpTextHelper;

CRtrModel* gpModel = NULL;
CModelViewerCamera gCamera;
ID3D11InputLayout* gpInputLayout = NULL;
COitTech* gpOitTech = NULL;
D3DXVECTOR3 gLightDirection = D3DXVECTOR3(0, 0, 1);
float gLightIntensity = 0.5f;
float gAlphaFactor = 0.5f;

ID3D11RasterizerState* gpNoBfcRastState = NULL;
ID3D11DepthStencilState* gpNoDepthState = NULL;

struct  
{
    ID3D11Texture2D* pResource;
    ID3D11DepthStencilView* pDSV;
    ID3D11ShaderResourceView* pSRV;
} gDepthResources[2];

struct
{
    ID3D11Texture2D* pResource;
    ID3D11RenderTargetView* pRTV;
    ID3D11ShaderResourceView* pSRV;
} gDepthPeelRenderTarget;
ID3D11DepthStencilState* gpDepthTestGreaterState;
ID3D11Buffer* gpQuadVB;
CFullScreenPass* gpFullScreenPass;

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
    if(FAILED(DXUTFindDXSDKMediaFileCch(f, 1024, MESH_FILE)))
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

    const D3DX11_PASS_DESC* pPassDesc = gpOitTech->GetPassDesc();
    HRESULT hr = pDevice->CreateInputLayout(desc, 2, pPassDesc->pIAInputSignature, pPassDesc->IAInputSignatureSize, &gpInputLayout);
    if(FAILED(hr))
    {
        trace(L"Could not create input layout");
        PostQuitMessage(0);
    }

}

void ReleaseDepthPeelResources()
{
    for(int i = 0 ; i < 2 ; i++)
    {
        SAFE_RELEASE(gDepthResources[i].pDSV);
        SAFE_RELEASE(gDepthResources[i].pSRV);
        SAFE_RELEASE(gDepthResources[i].pResource);
    }

    SAFE_RELEASE(gDepthPeelRenderTarget.pRTV);
    SAFE_RELEASE(gDepthPeelRenderTarget.pSRV);
    SAFE_RELEASE(gDepthPeelRenderTarget.pResource);
}

void InitDepthResources(ID3D11Device* pDevice, UINT Height, UINT Width)
{
    ReleaseDepthPeelResources();
    for(int i = 0 ; i < 2 ; i++)
    {
        // Create the resource
        D3D11_TEXTURE2D_DESC TexDesc;
        TexDesc.ArraySize = 1;
        TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        TexDesc.CPUAccessFlags = 0;
        TexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        TexDesc.Height = Height;
        TexDesc.MipLevels = 1;
        TexDesc.MiscFlags = 0;
        TexDesc.SampleDesc.Count = 1;
        TexDesc.SampleDesc.Quality = 0;
        TexDesc.Usage = D3D11_USAGE_DEFAULT;
        TexDesc.Width = Width;

        if(FAILED(pDevice->CreateTexture2D(&TexDesc, NULL, &gDepthResources[i].pResource)))
        {
            trace(L"Could not create depth resource");
            PostQuitMessage(0);
        }

        // Create the resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
        SrvDesc.Texture2D.MipLevels = 1;
        SrvDesc.Texture2D.MostDetailedMip = 0;
        SrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        if(FAILED(pDevice->CreateShaderResourceView(gDepthResources[i].pResource, &SrvDesc, &gDepthResources[i].pSRV)))
        {
            trace(L"Could not create depth SRV");
            PostQuitMessage(0);
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC DsvDesc;
        DsvDesc.Flags = 0;
        DsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        DsvDesc.Texture2D.MipSlice = 0;
        DsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        if(FAILED(pDevice->CreateDepthStencilView(gDepthResources[i].pResource, &DsvDesc, &gDepthResources[i].pDSV)))
        {
            trace(L"Could not create depth DSV");
            PostQuitMessage(0);
        }
    }

    // Create the render target resource
    ID3D11RenderTargetView* pOrigRTV = DXUTGetD3D11RenderTargetView();
    D3D11_RENDER_TARGET_VIEW_DESC RtvDesc;
    pOrigRTV->GetDesc(&RtvDesc);

    D3D11_TEXTURE2D_DESC TexDesc;
    TexDesc.ArraySize = 1;
    TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    TexDesc.CPUAccessFlags = 0;
    TexDesc.Format = RtvDesc.Format;
    TexDesc.Height = Height;
    TexDesc.MipLevels = 1;
    TexDesc.MiscFlags = 0;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
    TexDesc.Width = Width;

    if(FAILED(pDevice->CreateTexture2D(&TexDesc, NULL, &gDepthPeelRenderTarget.pResource)))
    {
        trace(L"Could not create depth peel render target resource");
        PostQuitMessage(0);
    }

    // Create the resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
    SrvDesc.Texture2D.MipLevels = 1;
    SrvDesc.Texture2D.MostDetailedMip = 0;
    SrvDesc.Format = RtvDesc.Format;
    SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    if(FAILED(pDevice->CreateShaderResourceView(gDepthPeelRenderTarget.pResource, &SrvDesc, &gDepthPeelRenderTarget.pSRV)))
    {
        trace(L"Could not create depth peel render target SRV");
        PostQuitMessage(0);
    }

    // Create the render target view
    if(FAILED(pDevice->CreateRenderTargetView(gDepthPeelRenderTarget.pResource, &RtvDesc, &gDepthPeelRenderTarget.pRTV)))
    {
        trace(L"Could not create depth peel render target view");
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

    gpOitTech = COitTech::Create(pd3dDevice);
    if(gpOitTech == NULL)
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

    DepthStencilDesc.DepthEnable = TRUE;
    DepthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
    DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    V_RETURN(pd3dDevice->CreateDepthStencilState(&DepthStencilDesc, &gpDepthTestGreaterState));

    // Load the mesh
    LoadMesh(pd3dDevice);

    // Create the full screen effect
    gpFullScreenPass = CFullScreenPass::Create(pd3dDevice);

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

    // Create the depth resources
    InitDepthResources(pd3dDevice, pBackBufferSurfaceDesc->Height, pBackBufferSurfaceDesc->Width);

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
    static bool b = true;
    if(b)
    {
        gCamera.FrameMove(fElapsedTime);
//        b = false;
    }
}

void InitDrawCommon(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Clear render target and the depth stencil 
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, gClearColor );

    // Set common shader data
    gpOitTech->SetWorldMat(gCamera.GetWorldMatrix());
    D3DXMATRIX wvp = (*gCamera.GetWorldMatrix()) * (*gCamera.GetViewMatrix()) * (*gCamera.GetProjMatrix());
    gpOitTech->SetWVPMat(&wvp);

    gpOitTech->SetLightIntensity(gLightIntensity);
    D3DXVECTOR3 negLightDir = -gLightDirection;
    gpOitTech->SetNegLightDirW(&negLightDir);
    gpOitTech->SetCameraPosW(gCamera.GetEyePt());
    gpOitTech->SetAlphaOut((gTechType != OIT_TECH_NONE) ? gAlphaFactor : 1.0f);

    // Set the input layout
    pd3dImmediateContext->IASetInputLayout(gpInputLayout);
}

void NormalDraw(ID3D11DeviceContext* pd3dImmediateContext, bool bBlendEnabled)
{
    // Clear the DSV
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

    // Set rasterizer/depthstencil state
    pd3dImmediateContext->RSSetState(bBlendEnabled ? gpNoBfcRastState : NULL);
    pd3dImmediateContext->OMSetDepthStencilState(bBlendEnabled ? gpNoDepthState : NULL, 0);

    // Draw
    for(UINT i = 0 ; i < gpModel->GetMeshesCount() ; i++)
    {
        gpOitTech->SetMeshID(i);
        gpOitTech->ApplyNoOitTech(pd3dImmediateContext, (gTechType == OIT_TECH_BLEND));
        if(gpModel->SetMeshData(i, pd3dImmediateContext))
        {
            pd3dImmediateContext->DrawIndexed(gpModel->GetMeshIndexCount(i), 0, 0);
        }
    }
}

void DrawDepthPeeling(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Backup the original buffers
    ID3D11RenderTargetView* pOrigRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pOrigDSV = DXUTGetD3D11DepthStencilView();

    pd3dImmediateContext->ClearDepthStencilView(gDepthResources[1].pDSV, D3D11_CLEAR_DEPTH, 1, 0);


    for(int Layer = 0 ; Layer < 3 ; Layer++)
    {
        // First, peel (going back to front)
        int ActiveDSV = Layer % 2;

        // Setup the depth state
        pd3dImmediateContext->ClearDepthStencilView(gDepthResources[ActiveDSV].pDSV, D3D11_CLEAR_DEPTH, 0, 0);
        pd3dImmediateContext->ClearRenderTargetView(gDepthPeelRenderTarget.pRTV, gClearColor);
        pd3dImmediateContext->OMSetRenderTargets(1, &gDepthPeelRenderTarget.pRTV, gDepthResources[ActiveDSV].pDSV);

        // Set DS/RS state
        pd3dImmediateContext->OMSetDepthStencilState(gpDepthTestGreaterState, 0);
        pd3dImmediateContext->RSSetState(gpNoBfcRastState);

        // Setup the render target view
        gpOitTech->SetDepthTexture(gDepthResources[1 - ActiveDSV].pSRV);

        // Set the input layout
        pd3dImmediateContext->IASetInputLayout(gpInputLayout);

        // Draw
        for(UINT MeshID = 0 ; MeshID < gpModel->GetMeshesCount() ; MeshID++)
        {
            gpOitTech->SetMeshID(MeshID);
            gpOitTech->ApplyDepthPeelTech(pd3dImmediateContext);
            if(gpModel->SetMeshData(MeshID, pd3dImmediateContext))
            {
                pd3dImmediateContext->DrawIndexed(gpModel->GetMeshIndexCount(MeshID), 0, 0);
            }
        }

        // Now blend with the background
        pd3dImmediateContext->OMSetRenderTargets(1, &pOrigRTV, pOrigDSV);
        gpFullScreenPass->DrawBackToFrontBlend(pd3dImmediateContext, gDepthPeelRenderTarget.pSRV);
    }

}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    InitDrawCommon(pd3dImmediateContext);

    switch(gTechType)
    {
    case OIT_TECH_NONE:
    case OIT_TECH_BLEND:
        NormalDraw(pd3dImmediateContext, gTechType == OIT_TECH_BLEND);
        break;
    case OIT_TECH_DEPTH_PEELING:
        DrawDepthPeeling(pd3dImmediateContext);
        break;
    case OIT_TECH_DUAL_DEPTH:
        break;
    case OIT_TECH_K_BUFFER:
        break;
    case OIT_TECH_LINKED_LIST:
        break;
    default:
        break;
    }

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
    ReleaseDepthPeelResources();
    gDialogResourceManager.OnD3D11DestroyDevice();
    SAFE_DELETE(gpTextHelper);
    SAFE_DELETE(gpModel);
    SAFE_RELEASE(gpInputLayout);
    SAFE_DELETE(gpOitTech);
    SAFE_RELEASE(gpNoBfcRastState);
    SAFE_RELEASE(gpNoDepthState);
    SAFE_RELEASE(gpDepthTestGreaterState);
    SAFE_DELETE(gpFullScreenPass);
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


