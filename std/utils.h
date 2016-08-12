#ifndef __MY_UTILS_H__
#define __MY_UTILS_H__

#include "defs.h"
extern "C" {
	#include "lcthw\ringbuffer.h"
}

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>

#include <Windows.h>

inline int64_t get_current_time_in_ms() {
	//DWORD d = timeGetTime();
	//return d;
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline void sleep(int32_t milliseconds) {
	//std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	::Sleep(milliseconds);
}


#endif // !__MY_UTILS_H__
