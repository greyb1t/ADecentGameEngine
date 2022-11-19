#pragma once

#include <windows.h>

namespace Engine
{
	class CWaveBuffer
	{
	public:
		~CWaveBuffer();
		BOOL Init(HWAVEOUT hWave, int Size);
		BOOL Write(PBYTE pData, int nBytes, int& BytesWritten);
		void Flush();
	private:
		WAVEHDR      m_Hdr;
		HWAVEOUT     m_hWave;
		int          m_nBytes;
	};

	class CWaveOut {
	public:
		CWaveOut(LPCWAVEFORMATEX Format, int nBuffers, int BufferSize);
		~CWaveOut();
		void Write(PBYTE Data, int nBytes);
		void Flush();
		void Wait();
		void Reset();
	private:
		const HANDLE   m_hSem;
		const int      m_nBuffers;
		int            m_CurrentBuffer;
		BOOL           m_NoBuffer;
		CWaveBuffer* m_Hdrs;
		HWAVEOUT       m_hWave;
	};
}
