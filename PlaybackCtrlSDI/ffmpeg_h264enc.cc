#include "ffmpeg_h264enc.h"

#include <signal.h>
#include <fstream>
#include <iostream>

void CFFmpegH264Encoder::start() {
	m_exited = false;
	m_impl = std::thread(&CFFmpegH264Encoder::run, this);
}

void CFFmpegH264Encoder::stop() {
	m_exited = true;
	m_impl.join();
}

void CFFmpegH264Encoder::join() {
	m_impl.join();
}


int32_t CFFmpegH264Encoder::run() {

	m_yuv_buffer_len = m_src_h*m_src_w * 3 / 2;
	m_yuv_buffer = new int8_t[m_yuv_buffer_len];

	int32_t out_buf_len = 1024 * 1024;
	uint8_t* out_buf = new uint8_t[out_buf_len];

	AVPacket pkt;
	AVFrame* yuv_frame = av_frame_alloc();

	if (!yuv_frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}

	/* find the mpeg1 video encoder */
	m_pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!m_pCodec) {
		fprintf(stderr, "Codec not found\n");
		goto end;
	}

	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	if (!m_pCodecCtx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		goto end;
	}

	/* put sample parameters */
	m_pCodecCtx->bit_rate = 3000000;				//FIXME: want to parameterlize
												/* resolution must be a multiple of two */
	m_pCodecCtx->width = m_src_w;
	m_pCodecCtx->height = m_src_h;
	/* frames per second */
	m_pCodecCtx->time_base = { 1, ENCODER_FRAME_RATE };
	/* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/
	m_pCodecCtx->gop_size = ENCODER_GOP_SIZE;
	m_pCodecCtx->max_b_frames = 0;
	m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	av_opt_set(m_pCodecCtx->priv_data, "preset", "ultrafast", 0);
	av_opt_set(m_pCodecCtx->priv_data, "crf", "25.000", 0);
	av_opt_set(m_pCodecCtx->priv_data, "tune", "zerolatency", 0);

	//can happen an exception!!!!
	//av_opt_set(pCodecCtx->priv_data, "profile", "baseline", 0);

	/* open it */
	if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		goto end;
	}

	yuv_frame->format = m_pCodecCtx->pix_fmt;
	yuv_frame->width = m_pCodecCtx->width;
	yuv_frame->height = m_pCodecCtx->height;

	int ret = 0;
	while (!m_exited) {
		int32_t nByteRead = 0;
		//read yuvs
		if (m_source_callback) {
			nByteRead = m_source_callback(m_yuv_buffer, m_yuv_buffer_len);
		}
		if (0 == nByteRead) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}

		//fprintf(stderr, "2%s nByteRead %d\n", __FUNCTION__, nByteRead);
		ret = av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize, (const uint8_t*)m_yuv_buffer, AV_PIX_FMT_YUV420P, m_src_w, m_src_h, 32);
		//yuv_frame.pts = av_frame_get_best_effort_timestamp(&yuv_frame);

		av_init_packet(&pkt);
		pkt.data = out_buf;    // packet data will be allocated by the encoder
		pkt.size = out_buf_len;

		/* encode the image */
		int got_pkt_ptr = 0;
		//fprintf(stdout, "1%s avcodec_encode_video2\n",__FUNCTION__);
		ret = avcodec_encode_video2(m_pCodecCtx, &pkt, yuv_frame, &got_pkt_ptr);
		//fprintf(stdout, "2%s avcodec_encode_video2\n", __FUNCTION__);

		if (ret < 0) {
			fprintf(stderr, "Error encoding frame\n");
			goto end;
		}
		if (got_pkt_ptr) {
			m_sink_callback((int8_t*)pkt.data, pkt.size);
		}
		av_packet_unref(&pkt);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

end:
	delete[] m_yuv_buffer;
	delete[] out_buf;
	av_frame_free(&yuv_frame);
	avcodec_close(m_pCodecCtx);
	avcodec_free_context(&m_pCodecCtx);
	return 0;
}

bool CFFmpegH264Encoder::init(int16_t src_w, int16_t src_h, _SourceDataCallback ReadNextFrameCallback, _SinkDataCallback WriteFrameCallback) {
	m_source_callback = ReadNextFrameCallback;
	m_sink_callback = WriteFrameCallback;
	m_src_h = src_h;
	m_src_w = src_w;

	av_register_all();

	return true;
}

bool CFFmpegH264Encoder::uninit() {
	return true;
}

void CFFmpegH264Decoder::start() {
	m_exited = false;
	m_impl = std::thread(&CFFmpegH264Decoder::run, this);
}

void CFFmpegH264Decoder::stop() {
	m_exited = true;
	m_impl.join();
}

void CFFmpegH264Decoder::join() {
	m_impl.join();
}


int32_t CFFmpegH264Decoder::run() {
	/* find the mpeg1 video encoder */
	m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!m_pCodec) {
		fprintf(stderr, "Codec not found\n");
		goto end;
	}

	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	if (!m_pCodecCtx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		goto end;
	}

	//初始化参数，下面的参数应该由具体的业务决定  
	m_pCodecCtx->time_base.num = 1;
	m_pCodecCtx->frame_number = 1; //每包一个视频帧  
	m_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	m_pCodecCtx->bit_rate = 0;
	m_pCodecCtx->time_base.den = 30;//帧率  
	m_pCodecCtx->width = m_src_w;//视频宽  
	m_pCodecCtx->height = m_src_h;//视频高  

	/* open it */
	if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		goto end;
	}

	m_h264_buffer_len = 1024*1024*1;
	m_h264_buffer = new int8_t[m_h264_buffer_len];

	m_yuv420p_buffer_len = m_src_h* m_src_w * 3 / 2;
	m_yuv420p_buffer = new uint8_t[m_yuv420p_buffer_len];

	AVFrame* pFrame_ = av_frame_alloc();// Allocate video frame  
	pFrame_->width = m_src_w;
	pFrame_->height = m_src_h;

	while (!m_exited) {
		AVPacket pkt;
		int32_t nByteRead = 0;

		//read yuvs
		if (m_source_callback) {
			nByteRead = m_source_callback(m_h264_buffer, m_h264_buffer_len);
		}
		if (0 == nByteRead) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}

		av_init_packet(&pkt);
		pkt.data = (uint8_t*)m_h264_buffer;
		pkt.size = nByteRead;

		/* encode the image */
		int got_pkt_ptr = 0;
		int ret = avcodec_decode_video2(m_pCodecCtx, pFrame_, &got_pkt_ptr, &pkt);
		if (ret < 0) {
			fprintf(stderr, "Error encoding frame\n");
			goto end;
		}
		if (got_pkt_ptr) {

			int height = m_pCodecCtx->height;
			int width = m_pCodecCtx->width;

			//写入数据  
			int a = 0, i;
			for (i = 0; i<height; i++)
			{
				memcpy(m_yuv420p_buffer + a, pFrame_->data[0] + i * pFrame_->linesize[0], width);
				a += width;
			}
			for (i = 0; i<height / 2; i++)
			{
				memcpy(m_yuv420p_buffer + a, pFrame_->data[1] + i * pFrame_->linesize[1], width / 2);
				a += width / 2;
			}
			for (i = 0; i<height / 2; i++)
			{
				memcpy(m_yuv420p_buffer + a, pFrame_->data[2] + i * pFrame_->linesize[2], width / 2);
				a += width / 2;
			}
			if (m_sink_callback) {
				m_sink_callback((int8_t*)m_yuv420p_buffer, m_yuv420p_buffer_len);
			}
		}
		av_packet_unref(&pkt);
	}

end:
	delete[] m_yuv420p_buffer;
	delete[] m_h264_buffer;
	av_frame_free(&pFrame_);
	avcodec_close(m_pCodecCtx);
	avcodec_free_context(&m_pCodecCtx);
	return 0;
}

bool CFFmpegH264Decoder::init(int16_t src_w, int16_t src_h, _SourceDataCallback ReadNextFrameCallback, _SinkDataCallback WriteFrameCallback) {
	m_source_callback = ReadNextFrameCallback;
	m_sink_callback = WriteFrameCallback;
	m_src_h = src_h;
	m_src_w = src_w;

	av_register_all();

	return true;
}

bool CFFmpegH264Decoder::uninit() {
	return true;
}

#if 0

static int i = 0;
static int j = 0;
int32_t ReadNextFrame(int8_t* buffer, int32_t buffer_len, void* ctx) {
	size_t nBytesRead = 0;
	std::ifstream* ifsp = (std::ifstream*)ctx;

	if (!ifsp || !buffer) return 0;

	ifsp->read((char*)buffer, buffer_len);
	fprintf(stderr, "%s, %d\n", __FUNCTION__, ++i);
	if (ifsp->fail()) {
		//ifsp->clear();
		//ifsp->seekg(ifsp->beg);
		return 0;
	}
	nBytesRead = buffer_len;

	//FILE* fp = (FILE*)ctx;
	//nBytesRead = fread(buffer, 1, buffer_len, fp);
	//if (nBytesRead == 0) {
	//	fprintf(stderr, "%s, %d\n", __FUNCTION__, ++i);
	//}

	return nBytesRead;
}

static int index = 0;

int32_t WriteNextFrame(int8_t* buffer, int32_t buffer_len, void*  ctx) {
	std::ofstream* ofsp = (std::ofstream*)ctx;
	ofsp->write((char*)buffer, buffer_len);
	fprintf(stderr, "%s, %d\n", __FUNCTION__, ++j);

	char file[100];
	sprintf(file, "%d.264", ++index);
	std::ofstream ofs(file, std::ofstream::out | std::ofstream::binary);
	if (ofs.fail()) {
		fprintf(stderr, __FUNCTION__);
		return 0;
	}
	ofs.write((const char*)buffer, buffer_len);
	ofs.close();

	return buffer_len;
}


bool g_running_flag = true;
void sig_cb(int sig)
{
	if (sig == SIGINT) {
		std::cout << __FUNCTION__ << std::endl;
		g_running_flag = false;
	}
}




int main(int argc, char** argv) {

	int32_t src_w = 1920;
	int32_t src_h = 1080;

	//FILE* fp = fopen("1.h264", "rb");
	//std::ofstream ofs("trailer_1080p_out.yuv420p", std::ofstream::out | std::ofstream::binary);

	//CFFmpegH264Decoder decoder;

	//decoder.init(src_w, src_h, std::bind(ReadNextFrame, std::placeholders::_1, std::placeholders::_2, fp), std::bind(WriteNextFrame, std::placeholders::_1, std::placeholders::_2, &ofs));
	//decoder.start();

	//while (g_running_flag)
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	//decoder.stop();
	//ofs.close();
	//fclose(fp);



	std::ifstream ifs("trailer_1080p.yuv420p", std::ifstream::in | std::ifstream::binary);
	std::ofstream ofs("trailer_1080p.264", std::ofstream::out | std::ofstream::binary);

	CFFmpegH264Encoder encoder;

	encoder.init(src_w, src_h, std::bind(ReadNextFrame, std::placeholders::_1, std::placeholders::_2, &ifs), std::bind(WriteNextFrame, std::placeholders::_1, std::placeholders::_2, &ofs));
	encoder.start();

	while (g_running_flag)
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

	encoder.stop();
	ifs.close();
	ofs.close();

	return 0;
}

#endif