#ifndef _CFSCOUNTER_H_
#define _CFSCOUNTER_H_

#include <windows.h>

class  CFPSCounter {
public:
	CFPSCounter() {
		::QueryPerformanceCounter(&m_nStartCounter);
		::QueryPerformanceFrequency(&m_nFrequency);
		m_old_fps = m_fps = .0f;
	}

	void statistics(const char* function_name = "", BOOL printfFlag = TRUE) {
		++m_fps;
		::QueryPerformanceCounter(&m_nStopCounter);
		double nTime = 1000.0f*(m_nStopCounter.QuadPart - m_nStartCounter.QuadPart) / m_nFrequency.QuadPart;
		if (nTime >= 1000.0f) {
			m_old_fps = m_fps * 1000.0f / nTime;
			if (printfFlag)
				fprintf(stderr, "%s fps:%.2f\n", function_name, m_old_fps);
			::QueryPerformanceCounter(&m_nStartCounter);
			m_fps = 0;
		}
	}

	double GetFPS() {
		return m_old_fps;
	}

private:
	LARGE_INTEGER m_nStartCounter;
	LARGE_INTEGER m_nFrequency;
	LARGE_INTEGER m_nStopCounter;
	double		  m_fps;
	double	      m_old_fps;
};
#endif