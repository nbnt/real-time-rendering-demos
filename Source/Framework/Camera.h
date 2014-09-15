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

Filename: Camera.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include "Common.h"

struct SMouseData;

class CModelViewCamera
{
public:
    CModelViewCamera();
	bool OnMouseEvent(const SMouseData& Data);
    void SetProjectionParams(float FovY, float AspectRatio);
    void SetModelParams(const float3& Center, float Radius);
	float3 Project2DCrdToUnitSphere(float2 xy);
    const float3& GetPosition() const { return m_Position; }
    const float4x4& GetViewMatrix();
    const float4x4& GetProjMatrix();

private:
    float m_FovY;
    float m_AspectRatio;
    float3 m_ModelCenter;
    float m_ModelRadius;
    float3 m_Position;
    float4x4 m_ViewMat;
    float4x4 m_ProjMat;
    float4x4 m_Rotation;

    float m_CameraDistance;

    bool m_bDirty = true;
    float3 m_LastVector;
    bool m_bLeftButtonDown = false;

    void CalculateMatrices();
};