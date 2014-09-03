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

Filename: Camera.cpp
---------------------------------------------------------------------------*/
#include "Camera.h"
#include <sstream>
#include "Sample.h"

using namespace DirectX;

static const float defaultCameraDistance = 4.0f;

void CModelViewCamera::SetProjectionParams(float FovY, float AspectRatio)
{
    m_AspectRatio = AspectRatio;
    m_FovY = FovY;
    m_bDirty = true;
}

void CModelViewCamera::SetModelParams(const float3& Center, float Radius)
{
    m_ModelCenter = Center;
    m_ModelRadius = Radius;
    m_CameraDistance = defaultCameraDistance;
    m_Rotation = float4x4::Identity();
    m_bDirty = true;
}

const float4x4& CModelViewCamera::GetViewMatrix()
{
    if(m_bDirty)
	{
        // Prepare translation matrix so that the model is centered around the origin
        float4x4 Translation = float4x4::CreateTranslation(-m_ModelCenter);

        const float3 Up(0, 1, 0);
        const float3 CameraPosition(0, 0, -m_ModelRadius * m_CameraDistance);

        m_ViewMat = Translation * m_Rotation * XMMatrixLookAtLH(CameraPosition, float3(0, 0, 0), Up);
        float NearZ = m_ModelRadius * (m_CameraDistance - 1);
        float FarZ = m_ModelRadius * (m_CameraDistance  + 1);
        m_ProjMat = XMMatrixPerspectiveFovLH(m_FovY, m_AspectRatio, NearZ, FarZ);
        m_bDirty = false;
	}
	return m_ViewMat;
}

CModelViewCamera::CModelViewCamera()
{
}

float3 CModelViewCamera::Project2DCrdToUnitSphere(float2 xy)
{
    float xyLengthSquared = xy.LengthSquared();

    float z = 0;
    if(xyLengthSquared < 1)
    {
        z = -sqrt(1 - xyLengthSquared);
    }
    else
    {
        xy.Normalize();
    }
    return float3(xy.x, xy.y, z);
}

bool CModelViewCamera::OnMouseEvent(const SMouseData& Data)
{
	bool bHandled = false;
	switch(Data.Event)
	{
	case WM_MOUSEWHEEL:
		m_CameraDistance -= (float(Data.WheelDelta) * 0.3f);
		m_bDirty = true;
		bHandled = true;
		break;
	case WM_LBUTTONDOWN:
		m_LastVector = Project2DCrdToUnitSphere(Data.Crd);
		m_bLeftButtonDown = true;
		bHandled = true;
		break;
	case WM_LBUTTONUP:
		bHandled = m_bLeftButtonDown;
		m_bLeftButtonDown = false;
		break;
	case WM_MOUSEMOVE:
		if(m_bLeftButtonDown)
		{
			float3 CurVec = Project2DCrdToUnitSphere(Data.Crd);
			quaternion q = CreateQuaternionFromVectors(m_LastVector, CurVec);
			float4x4 rot = float4x4::CreateFromQuaternion(q);
			m_Rotation *= rot;
            m_bDirty = true;
			m_LastVector = CurVec;
			bHandled = true;
		}
		break;
	default:
		break;
	}

	return bHandled;
}