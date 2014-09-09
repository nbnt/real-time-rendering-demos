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
#include "SimpleMath.h"

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;
using quaternion = DirectX::SimpleMath::Quaternion;

inline quaternion CreateQuaternionFromVectors(const float3& from, const float3& to)
{
	quaternion quat;
	float3 nFrom;
	float3 nTo;
	from.Normalize(nFrom);
	to.Normalize(nTo);

	float dot = nFrom.Dot(nTo);
	dot = max(min(dot, 1), -1);
	if(dot != 1)
	{
		float angle = acosf(dot);

		float3 cross = nFrom.Cross(nTo);
		float3 axis;
		cross.Normalize(axis);

		quat = quaternion::CreateFromAxisAngle(axis, angle);
	}

	return quat;
}

inline float FovFromFocalLength(int FocalLength)
{
	const float x = 43.266f; // Diagonal length of 24*36mm image
	float fov = float(FocalLength);
	fov = 2 * atan(x / (2 * fov));
	return fov;
}

using RTR_BOX = D3D11_BOX;
struct RTR_BOX_F
{
	float3 Min;
	float3 Max;

	RTR_BOX_F() : Min(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX), Max(-D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX)
	{
	}

	RTR_BOX_F Transform(const float4x4& Transform) const
	{
		RTR_BOX_F f;

		float3 xa = Transform.Right() * Min.x;
		float3 xb = Transform.Right() * Max.x;
		float3 xMin = float3::Min(xa, xb);
		float3 xMax = float3::Max(xa, xb);

		float3 ya = Transform.Up() * Min.y;
		float3 yb = Transform.Up() * Max.y;
		float3 yMin = float3::Min(ya, yb);
		float3 yMax = float3::Max(ya, yb);

		float3 za = Transform.Backward() * Min.z;
		float3 zb = Transform.Backward() * Max.z;
		float3 zMin = float3::Min(za, zb);
		float3 zMax = float3::Max(za, zb);

		RTR_BOX_F Box;
		Box.Min = xMin + yMin + zMin + Transform.Translation();
		Box.Max = xMax + yMax + zMax + Transform.Translation();

		return Box;
	}
};
