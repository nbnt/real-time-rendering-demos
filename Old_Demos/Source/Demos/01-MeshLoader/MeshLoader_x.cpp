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

    Filname: MeshLoader.cpp
---------------------------------------------------------------------------
*/

#include <windows.h>
#include "DXUT.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "RtrMesh.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"

const UINT ScreenWidth  = 1280;
const UINT ScreenHeight = 1024;

CD3DSettingsDlg gSettingsDialog;
CDXUTDialogResourceManager  gDialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog                 gUI;                    // User interface
CDXUTTextHelper*            gpTextHelper = NULL;

CModelViewerCamera gCamera;

bool gbWireframe = false;
UINT gVertexCount;

struct  
{
    ID3DBlob* pVSBlob;
    ID3DBlob* pPSBlob;

    ID3D11VertexShader* pVS;
    ID3D11PixelShader* pPS;
    ID3D11InputLayout* pLayout;
    ID3D11RasterizerState* pRastState;
} gWireframeTech = {0};

struct
{
    struct
    {
        ID3DBlob* pVSBlob;
        ID3DBlob* pPSBlob;
        ID3D11VertexShader* pVS;
        ID3D11PixelShader* pPS;

    } Diffuse, Texture;
    ID3D11InputLayout* pLayout;
    ID3D11SamplerState* pLinearSampler;
} gDiffuseTextureTech = {0};


struct SPerFrameCB
{
    D3DXMATRIX gWVPMat;		// WVP matrix. Common to all the techniques, so comes first
    D3DXMATRIX gWorld;
    D3DXVECTOR3 gLightDirW;
    float pad;              // Each element should be 4 floats wide, that's why we pad
};

ID3D11Buffer* gpCB = NULL;

D3DXVECTOR3 gLightDirW = D3DXVECTOR3(-0.5f, 0, -1); // Light dir in camera space

CRtrModel* gpModel = NULL;
// UI definitions
#define IDC_SETTINGS_DIALOG  1
#define IDC_LOAD_MESH        2
#define IDC_TOGGLE_WIREFRAME 3

HRESULT CreateCB(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
    // Create the constant buffer
    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;    
    Desc.ByteWidth = sizeof( SPerFrameCB );
    V_RETURN( pDevice->CreateBuffer( &Desc, NULL, &gpCB ) );
    DXUT_SetDebugName( gpCB, "CB" );
    return hr;
}

HRESULT CreateDiffuseTextureTech(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
    // Compile the shaders using the lowest possible profile for broadest feature level support
    V_RETURN( DXUTCompileShaderFromFile( L"01-MeshLoader\\01-Texture.hlsl", "VSMain", NULL, "vs_4_0_level_9_1", &gDiffuseTextureTech.Texture.pVSBlob ) );
    V_RETURN( DXUTCompileShaderFromFile( L"01-MeshLoader\\01-Texture.hlsl", "PSMain", NULL, "ps_4_0_level_9_1", &gDiffuseTextureTech.Texture.pPSBlob ) );
    // Create the shaders
    V_RETURN( pDevice->CreateVertexShader( gDiffuseTextureTech.Texture.pVSBlob->GetBufferPointer(), gDiffuseTextureTech.Texture.pVSBlob->GetBufferSize(), NULL, &gDiffuseTextureTech.Texture.pVS ));
    DXUT_SetDebugName( gDiffuseTextureTech.Texture.pVS, "TextureVSMain" );
    V_RETURN( pDevice->CreatePixelShader( gDiffuseTextureTech.Texture.pPSBlob->GetBufferPointer(), gDiffuseTextureTech.Texture.pPSBlob->GetBufferSize(), NULL, &gDiffuseTextureTech.Texture.pPS ));
    DXUT_SetDebugName( gDiffuseTextureTech.Texture.pPS, "TexturePSMain" );

    // Compile the shaders using the lowest possible profile for broadest feature level support
    V_RETURN( DXUTCompileShaderFromFile( L"01-MeshLoader\\01-Diffuse.hlsl", "VSMain", NULL, "vs_4_0_level_9_1", &gDiffuseTextureTech.Diffuse.pVSBlob ) );
    V_RETURN( DXUTCompileShaderFromFile( L"01-MeshLoader\\01-Diffuse.hlsl", "PSMain", NULL, "ps_4_0_level_9_1", &gDiffuseTextureTech.Diffuse.pPSBlob ) );
    // Create the shaders
    V_RETURN( pDevice->CreateVertexShader( gDiffuseTextureTech.Diffuse.pVSBlob->GetBufferPointer(), gDiffuseTextureTech.Diffuse.pVSBlob->GetBufferSize(), NULL, &gDiffuseTextureTech.Diffuse.pVS ));
    DXUT_SetDebugName( gDiffuseTextureTech.Diffuse.pVS, "DiffuseVSMain" );
    V_RETURN( pDevice->CreatePixelShader( gDiffuseTextureTech.Diffuse.pPSBlob->GetBufferPointer(), gDiffuseTextureTech.Diffuse.pPSBlob->GetBufferSize(), NULL, &gDiffuseTextureTech.Diffuse.pPS ));
    DXUT_SetDebugName( gDiffuseTextureTech.Diffuse.pPS, "DiffusePSMain" );

    // Create the sampler state
    D3D11_SAMPLER_DESC sampler;
    sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.MaxAnisotropy = 0;
    sampler.MaxLOD = D3D11_FLOAT32_MAX;
    sampler.MinLOD = 0;
    sampler.MipLODBias = 0;
    V_RETURN(pDevice->CreateSamplerState(&sampler, &gDiffuseTextureTech.pLinearSampler));

    return hr;
}

HRESULT CreateWireframeTech(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;
    // Compile the shaders using the lowest possible profile for broadest feature level support
    V_RETURN( DXUTCompileShaderFromFile( L"01-MeshLoader\\01-Wireframe.hlsl", "VSMain", NULL, "vs_4_0_level_9_1", &gWireframeTech.pVSBlob ) );
    V_RETURN( DXUTCompileShaderFromFile( L"01-MeshLoader\\01-Wireframe.hlsl", "PSMain", NULL, "ps_4_0_level_9_1", &gWireframeTech.pPSBlob ) );

    // Create the shaders
    V_RETURN( pDevice->CreateVertexShader( gWireframeTech.pVSBlob->GetBufferPointer(), gWireframeTech.pVSBlob->GetBufferSize(), NULL, &gWireframeTech.pVS ));
    DXUT_SetDebugName( gWireframeTech.pVS, "WireframeVSMain" );
    V_RETURN( pDevice->CreatePixelShader( gWireframeTech.pPSBlob->GetBufferPointer(), gWireframeTech.pPSBlob->GetBufferSize(), NULL, &gWireframeTech.pPS ));
    DXUT_SetDebugName( gWireframeTech.pPS, "WireframePSMain" );

    D3D11_RASTERIZER_DESC rast;
    rast.AntialiasedLineEnable = TRUE;
    rast.FillMode = D3D11_FILL_WIREFRAME;
    rast.CullMode = D3D11_CULL_NONE;
    rast.DepthBias = 0;
    rast.DepthBiasClamp = 0;
    rast.DepthClipEnable = FALSE;
    rast.FrontCounterClockwise = FALSE;
    rast.MultisampleEnable = FALSE;
    rast.ScissorEnable = FALSE;
    rast.SlopeScaledDepthBias = 0;
    V_RETURN(pDevice->CreateRasterizerState(&rast, &gWireframeTech.pRastState));
    return hr;
}

void SetConstantBuffer(ID3D11DeviceContext* pCtx)
{
    HRESULT hr;
    // Update the CB
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pCtx->Map(gpCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

    SPerFrameCB* pCB = (SPerFrameCB*)MappedResource.pData;
    D3DXMATRIX m;
    // Transpose the matrices, since we need it in row major
    // Once we start using effects, we don't need to transpose, since the effect system does that for us.
    D3DXMatrixTranspose(&m, gCamera.GetWorldMatrix());
    pCB->gWorld = m;
    D3DXMATRIX wvp = (*gCamera.GetWorldMatrix()) * (*gCamera.GetViewMatrix()) * (*gCamera.GetProjMatrix());
    D3DXMatrixTranspose(&m, &wvp);
    pCB->gWVPMat = m;
    pCB->gLightDirW = gLightDirW;
    pCtx->Unmap( gpCB, 0 );
    pCtx->VSSetConstantBuffers(0, 1, &gpCB);    
    pCtx->PSSetConstantBuffers(0, 1, &gpCB);
}

void SetWireframeState(ID3D11DeviceContext* pCtx)
{
    SetConstantBuffer(pCtx);

    // Set the shaders
    pCtx->VSSetShader(gWireframeTech.pVS, NULL, 0);
    pCtx->PSSetShader(gWireframeTech.pPS, NULL, 0);
    pCtx->IASetInputLayout(gWireframeTech.pLayout);
    pCtx->RSSetState(gWireframeTech.pRastState);
}

void SetTextureState(ID3D11DeviceContext* pCtx)
{
    SetConstantBuffer(pCtx);
    pCtx->VSSetShader(gDiffuseTextureTech.Texture.pVS, NULL, 0);
    pCtx->PSSetShader(gDiffuseTextureTech.Texture.pPS, NULL, 0);
    pCtx->IASetInputLayout(gDiffuseTextureTech.pLayout);
    pCtx->RSSetState(NULL);
    pCtx->PSSetSamplers(0, 1, &gDiffuseTextureTech.pLinearSampler);
}

void SetDiffuseState(ID3D11DeviceContext* pCtx)
{
    SetConstantBuffer(pCtx);
    pCtx->VSSetShader(gDiffuseTextureTech.Diffuse.pVS, NULL, 0);
    pCtx->PSSetShader(gDiffuseTextureTech.Diffuse.pPS, NULL, 0);
    pCtx->IASetInputLayout(gDiffuseTextureTech.pLayout);
    pCtx->RSSetState(NULL);
}

void CreateInputLayouts()
{
    ID3D11Device* pDevice = DXUTGetD3D11Device();

    SAFE_RELEASE(gWireframeTech.pLayout);
    SAFE_RELEASE(gDiffuseTextureTech.pLayout);

    D3D11_INPUT_ELEMENT_DESC desc[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    desc[0].AlignedByteOffset = gpModel->GetVertexElementOffset(RTR_MESH_ELEMENT_POSITION);
    desc[1].AlignedByteOffset = gpModel->GetVertexElementOffset(RTR_MESH_ELEMENT_TEXCOORD0);
    desc[2].AlignedByteOffset = gpModel->GetVertexElementOffset(RTR_MESH_ELEMENT_NORMAL);

    HRESULT hr = pDevice->CreateInputLayout(desc, 1, gWireframeTech.pVSBlob->GetBufferPointer(), gWireframeTech.pVSBlob->GetBufferSize(), &gWireframeTech.pLayout);
    if(FAILED(hr))
    {
        trace(L"Could not create wireframe input layout");
    }

    hr = pDevice->CreateInputLayout(desc, 3, gDiffuseTextureTech.Diffuse.pVSBlob->GetBufferPointer(), gDiffuseTextureTech.Diffuse.pVSBlob->GetBufferSize(), &gDiffuseTextureTech.pLayout);
    if(FAILED(hr))
    {
        trace(L"Could not create texture input layout");
    }
}

void LoadMesh()
{
    OPENFILENAME ofn;
    WCHAR filename[MAX_PATH] = L"";
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"";

    ID3D11Device* pDevice = DXUTGetD3D11Device();

    if (GetOpenFileName(&ofn))
    {
        SAFE_DELETE(gpModel);

        gpModel = gpModel->LoadModelFromFile(filename, pDevice);

        if(gpModel == NULL)
        {
            trace(L"Could not load mesh");            
            return;
        }
        CreateInputLayouts();

        // update the camera position        
        float fRadius = gpModel->GetRadius();
        gCamera.SetRadius( fRadius * 2, fRadius * 0.25f);

        D3DXVECTOR3 modelCenter = gpModel->GetCenter();
        gCamera.SetModelCenter(modelCenter);

        gVertexCount = 0;
        for(UINT i = 0 ; i < gpModel->GetMeshesCount() ; i++)
        {
            gVertexCount += gpModel->GetMeshVertexCount(i);
        }
    }
}

void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch(nControlID)
    {
    case IDC_SETTINGS_DIALOG:
        gSettingsDialog.SetActive(!gSettingsDialog.IsActive());
        break;
    case IDC_LOAD_MESH:
        LoadMesh();
        break;
    case IDC_TOGGLE_WIREFRAME:
        gbWireframe = !gbWireframe;
        break;
    }
}

void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    gCamera.FrameMove( fElapsedTime );
}

HRESULT InitGUI()
{
    gSettingsDialog.Init(&gDialogResourceManager);

    HRESULT hr = S_OK;
    UINT Height = 0;
    gUI.Init(&gDialogResourceManager);
    gUI.SetCallback(OnGUIEvent);
    gUI.AddButton(IDC_SETTINGS_DIALOG, L"Settings", 0, Height, 170, 30, VK_F2);
    Height += 40;
    gUI.AddButton(IDC_LOAD_MESH, L"Load Mesh", 0, Height, 170, 30);
    Height += 45;
    gUI.AddCheckBox(IDC_TOGGLE_WIREFRAME, L"Toggle wireframe", 0, Height, 170, 20, gbWireframe);
    
    return hr;
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
    if(gpModel)
    {
        WCHAR w[256];
        swprintf_s(w, L"%d meshes, %d vertices, %d primitives" , (UINT)gpModel->GetMeshesCount(), gVertexCount, (gVertexCount / 3));
        gpTextHelper->DrawTextLine(w);

    }
    WCHAR w[] = L"Left click rotates model, Right click rotates camera";
    gpTextHelper->DrawTextLine(w);
    gpTextHelper->End();
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
    const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( gDialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN(gSettingsDialog.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

    gUI.SetLocation( pBackBufferSurfaceDesc->Width - 200, 20 );
    gUI.SetSize( 170, 170 );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    gCamera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    gCamera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;
    // Init GUI and resource manager
    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    gpTextHelper = new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &gDialogResourceManager, 15);
    V_RETURN(gDialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
    V_RETURN(gSettingsDialog.OnD3D11CreateDevice(pd3dDevice));

    V_RETURN(CreateDiffuseTextureTech(pd3dDevice));
    V_RETURN(CreateWireframeTech(pd3dDevice));
    V_RETURN(CreateCB(pd3dDevice));
    D3DXVec3Normalize(&gLightDirW, &gLightDirW);
    return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    if(gSettingsDialog.IsActive())
    {
        const float ClearColor[4] = { 0, 0, 0, 0 };
        pd3dImmediateContext->ClearRenderTargetView(DXUTGetD3D11RenderTargetView(), ClearColor);
        gSettingsDialog.OnRender(fElapsedTime);
        return;
    }

    // Clear render target and the depth stencil 
    float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };

    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

    if(gpModel)
    {
        if(gbWireframe)
        {
            SetWireframeState(pd3dImmediateContext);
        }
        else
        {
            if(gpModel->HasTextures())
            {
                SetTextureState(pd3dImmediateContext);
            }
            else
            {
                SetDiffuseState(pd3dImmediateContext);
            }
        }

        gpModel->Draw(pd3dImmediateContext, 0);
    }

    gUI.OnRender(fElapsedTime);
    RenderText();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    gDialogResourceManager.OnD3D11DestroyDevice();
    gSettingsDialog.OnD3D11DestroyDevice();

    DXUTGetGlobalResourceCache().OnDestroyDevice();

    SAFE_DELETE(gpTextHelper);
    SAFE_DELETE(gpModel);

    SAFE_RELEASE(gWireframeTech.pVSBlob);
    SAFE_RELEASE(gWireframeTech.pPSBlob);
    SAFE_RELEASE(gWireframeTech.pVS);
    SAFE_RELEASE(gWireframeTech.pPS);
    SAFE_RELEASE(gWireframeTech.pLayout);
    SAFE_RELEASE(gWireframeTech.pRastState);

    SAFE_RELEASE(gDiffuseTextureTech.Texture.pVSBlob);
    SAFE_RELEASE(gDiffuseTextureTech.Texture.pPSBlob);
    SAFE_RELEASE(gDiffuseTextureTech.Texture.pVS);
    SAFE_RELEASE(gDiffuseTextureTech.Texture.pPS);
    SAFE_RELEASE(gDiffuseTextureTech.Diffuse.pVSBlob);
    SAFE_RELEASE(gDiffuseTextureTech.Diffuse.pPSBlob);
    SAFE_RELEASE(gDiffuseTextureTech.Diffuse.pVS);
    SAFE_RELEASE(gDiffuseTextureTech.Diffuse.pPS);
    SAFE_RELEASE(gDiffuseTextureTech.pLayout);
    SAFE_RELEASE(gDiffuseTextureTech.pLinearSampler);

    SAFE_RELEASE(gpCB);
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    gDialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    if(gSettingsDialog.IsActive())
    {
        gSettingsDialog.MsgProc(hWnd, uMsg, wParam, lParam);
        return 0;
    }

    *pbNoFurtherProcessing = gUI.MsgProc(hWnd, uMsg, wParam, lParam);
    if( *pbNoFurtherProcessing )
    {
        return 0;
    }

    gCamera.HandleMessages(hWnd, uMsg, wParam, lParam);
    return 0;
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
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);

    // Perform any application-level initialization here

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"MeshLoader" );

    InitGUI();

    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, ScreenWidth, ScreenHeight);
    DXUTMainLoop(); // Enter into the DXUT ren  der loop

    // Perform any application-level cleanup here

    return DXUTGetExitCode();
}


