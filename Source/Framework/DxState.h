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

Filename: DxState.h
---------------------------------------------------------------------------*/
#include <d3d11.h>
#include <d3dcompiler.inl>
#include <comdef.h>
#include <string>

#define MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))

// Device
MAKE_SMART_COM_PTR(ID3D11Device);
MAKE_SMART_COM_PTR(ID3D11DeviceContext);
MAKE_SMART_COM_PTR(ID3D11InputLayout);

// DXGI
MAKE_SMART_COM_PTR(IDXGISwapChain);

// Resource
MAKE_SMART_COM_PTR(ID3D11RenderTargetView);
MAKE_SMART_COM_PTR(ID3D11DepthStencilView);
MAKE_SMART_COM_PTR(ID3D11ShaderResourceView);
MAKE_SMART_COM_PTR(ID3D11Buffer);
MAKE_SMART_COM_PTR(ID3D11Resource);
MAKE_SMART_COM_PTR(ID3D11Texture1D);
MAKE_SMART_COM_PTR(ID3D11Texture2D);
MAKE_SMART_COM_PTR(ID3D11Texture3D);

// Shaders
MAKE_SMART_COM_PTR(ID3D11VertexShader);
MAKE_SMART_COM_PTR(ID3D11PixelShader);
MAKE_SMART_COM_PTR(ID3D11DomainShader);
MAKE_SMART_COM_PTR(ID3D11HullShader);
MAKE_SMART_COM_PTR(ID3D11GeometryShader);
MAKE_SMART_COM_PTR(ID3D11ComputeShader);
MAKE_SMART_COM_PTR(ID3DBlob);

// Reflection
MAKE_SMART_COM_PTR(ID3D11ShaderReflection);
MAKE_SMART_COM_PTR(ID3D11ShaderReflectionVariable);

// State
MAKE_SMART_COM_PTR(ID3D11DepthStencilState);
MAKE_SMART_COM_PTR(ID3D11RasterizerState);
MAKE_SMART_COM_PTR(ID3D11BlendState);
MAKE_SMART_COM_PTR(ID3D11SamplerState);

ID3D11ShaderResourceView* CreateShaderResourceViewFromFile(ID3D11Device* pDevice, const std::wstring& Filename, bool bSrgb);
