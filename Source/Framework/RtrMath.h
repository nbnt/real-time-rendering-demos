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

Filename: RtrMath.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include "Common.h"
#include <DirectXMath.h>

class float2
{
public:
	// Constructors
	float2() {};
	float2(float _x, float _y) : x(_x), y(_y) {}

	// Operators
	float2 operator+(const float2& f) const
	{
		return float2(x + f.x, y + f.y);
	}

	float2& operator+=(const float2& f)
	{
		*this = *this + f;
		return *this;
	}

	float2 operator*(const float2& f) const
	{
		return float2(x * f.x, y * f.y);
	}

	float2& operator*=(const float2& f)
	{
		*this = *this * f;
		return *this;
	}

	float2 operator*(float f)
	{
		return float2(x*f, y*f);
	}

	float2& operator*=(float f)
	{
		*this = *this * f;
		return *this;
	}

	float x = 0;
	float y = 0;
};

class float4x4 : public DirectX::XMFLOAT4X4
{
public:
	// Constructors
	float4x4() : float4x4(DirectX::XMMatrixIdentity()) {};
	float4x4(const DirectX::XMMATRIX& mat) { XMStoreFloat4x4(this, mat); }

	// Assignment
	float4x4& operator=(const DirectX::XMMATRIX& mat){ XMStoreFloat4x4(this, mat); return *this; }
};
