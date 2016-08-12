#pragma once

#define DLL_API __declspec(dllexport)  

#include <functional>
#include <thread>
#include <stdint.h>

class DLL_API CEncodeThread {
public:
	using _SourceDataCallback = std::function<int32_t(unsigned char*, int32_t)>;
	using _SinkDataCallback = std::function<int32_t(unsigned char*, int32_t)>;

	bool init(const char* params);
	void start(_SourceDataCallback ReadNextFrameCallback, _SinkDataCallback WriteFrameCallback);
	void stop();
	void join();

private:
	int32_t run();

	bool				m_exited;
	std::thread			m_impl;
	_SourceDataCallback m_source_callback;
	_SinkDataCallback   m_sink_callback;
	char				m_parameter_buf[1024];//-g 1920x1080 -b 3000 -f 30/1
};


class DLL_API CDecodeThread {
public:
	using _SourceDataCallback = std::function<int32_t(unsigned char*, int32_t)>;
	using _SinkDataCallback = std::function<int32_t(unsigned char*, int32_t)>;

	bool init(const char* params);
	void start(_SourceDataCallback ReadNextFrameCallback, _SinkDataCallback WriteFrameCallback);
	void stop();
	void join();

private:
	int32_t run();

	bool				m_exited;
	std::thread			m_impl;
	_SourceDataCallback m_source_callback;
	_SinkDataCallback   m_sink_callback;
	char				m_parameter_buf[1024];//-g 1920x1080 -b 3000 -f 30/1
};