#pragma once
#define DLL_API __declspec(dllexport)  
//#define DLL_API 


#include <string>
#include <functional>
#include <stdint.h>
#include <thread>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
}

using _SourceDataCallback = std::function < int32_t(int8_t*, int32_t)>;
using _SinkDataCallback = std::function < int32_t(int8_t*, int32_t)>;

class DLL_API CFFmpegH264Encoder {
public:
	

	const static int32_t ENCODER_FRAME_RATE = 25;
	const static int32_t ENCODER_GOP_SIZE = 1;
	
	bool init(int16_t src_w, int16_t src_h, _SourceDataCallback ReadNextFrameCallback, _SinkDataCallback WriteFrameCallback);
	bool uninit();
	void start();
	void stop();
	void join();

private:
	int32_t run();

	bool				m_exited;
	std::thread			m_impl;
	_SourceDataCallback m_source_callback;
	_SinkDataCallback   m_sink_callback;

	int16_t				m_src_w;
	int16_t				m_src_h;
	int8_t*				m_yuv_buffer;
	int32_t				m_yuv_buffer_len;

	AVCodecContext*		m_pCodecCtx;
	AVCodec*			m_pCodec;
};


class DLL_API CFFmpegH264Decoder {
public:


	const static int32_t DECODER_FRAME_RATE = 25;
	const static int32_t DECODER_GOP_SIZE = 1;

	bool init(int16_t src_w, int16_t src_h, _SourceDataCallback ReadNextFrameCallback, _SinkDataCallback WriteFrameCallback);
	bool uninit();
	void start();
	void stop();
	void join();

private:
	int32_t run();

	bool				m_exited;
	std::thread			m_impl;
	_SourceDataCallback m_source_callback;
	_SinkDataCallback   m_sink_callback;

	int16_t				m_src_w;
	int16_t				m_src_h;
	int8_t*				m_h264_buffer;
	int32_t				m_h264_buffer_len;

	uint8_t*			m_yuv420p_buffer;
	int32_t				m_yuv420p_buffer_len;

	AVCodecContext*		m_pCodecCtx;
	AVCodec*			m_pCodec;
};