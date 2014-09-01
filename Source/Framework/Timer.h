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

Filename: Timer.h
---------------------------------------------------------------------------*/
#pragma once
#include <windows.h>
#include <chrono>
#include <vector>

class CTimer
{
public:
	void ResetClock()
	{
		m_ElpasedTime.resize(m_FpsFrameWindow);
		m_FrameCount = 0;
		m_ElpasedTime[m_FrameCount] = std::chrono::duration<double>::zero();
		m_LastFrameTime = std::chrono::system_clock::now();
	}

	void Tick()
	{
		m_FrameCount++;
		auto Now = std::chrono::system_clock::now();
		m_ElpasedTime[m_FrameCount	% m_FpsFrameWindow] = Now - m_LastFrameTime;
		m_LastFrameTime = Now;
	}

	float CalcFps() const
	{
		UINT32 Frames = min(m_FrameCount, m_FpsFrameWindow);
		std::chrono::duration<double> ElapsedTime = std::chrono::duration<double>::zero();
		for(UINT32 i = 0; i < Frames; i++)
		{
			ElapsedTime += m_ElpasedTime[i];
		}

		double fps = double(Frames) / ElapsedTime.count();
		return float(fps);
	}
	
	float GetElapsedTime() const
	{
		return float(m_ElpasedTime[m_FrameCount	% m_FpsFrameWindow].count());
	}
private:

	std::chrono::time_point < std::chrono::system_clock > m_LastFrameTime;
	std::vector<std::chrono::duration<double>> m_ElpasedTime;
	UINT32 m_FrameCount;
	static const int m_FpsFrameWindow = 60;
};