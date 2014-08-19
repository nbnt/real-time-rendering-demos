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

Filename: Common.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WIDEFILE__ WIDEN(__FILE__)

void trace(const WCHAR* file, int line, HRESULT hr, const WCHAR* msg);

#ifdef _DEBUG
#define verify(a) {HRESULT __hr = a; if(FAILED(__hr)) { trace( __WIDEFILE__, __LINE__, __hr, L#a); } }
#define verify_return(a) {HRESULT __hr = a; if(FAILED(__hr)) { trace( __WIDEFILE__, __LINE__, __hr, L#a); return __hr;} }
#define assert(a) {if(!a) {__debugbreak();}}
#else
#define verify(a) a
#define verify_return(a) {HRESULT __hr = a ; if(FAILED(__hr)) {return __hr;}}
#define assert(a)
#endif

#define SAFE_RELEASE(a) {if(a) {a->Release(); a = nullptr;}}
#define SAFE_DELETE(a) {if(a) {delete a; a = nullptr;}}
#define SAFE_DELETE_ARRAY(a) {if(a) {delete[] a; a = nullptr;}}