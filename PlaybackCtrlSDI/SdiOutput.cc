#include "PlaybackCtrlSdi.h"
#include "DeckLinkIODevice.h"

extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
}

FILE*	g_uyvy_fp = NULL;
int on_deck_link_output_need_data(char* buffer, int len, void* ctx) {

	CSdiOutput* p = (CSdiOutput*)ctx;


	if (p->m_cb_need_data) {
		p->m_cb_need_data(p->m_input_rgba_buffer, p->m_input_rgba_buffer_len, p->m_cb_need_data_ctx);
	}
	else {
		return 0;
	}

	av_image_fill_arrays(((AVFrame*)p->m_rgba_frame)->data, ((AVFrame*)p->m_rgba_frame)->linesize, (const uint8_t*)p->m_input_rgba_buffer, AV_PIX_FMT_RGBA, p->m_width, p->m_height, 1);
	av_image_fill_arrays(((AVFrame*)p->m_uyvy422_frame)->data, ((AVFrame*)p->m_uyvy422_frame)->linesize, (const uint8_t*)p->m_output_uyvy422_buffer, AV_PIX_FMT_UYVY422, p->m_width, p->m_height, 16);

	int ret = sws_scale((SwsContext*)p->m_scale_ctx, ((AVFrame*)p->m_rgba_frame)->data, ((AVFrame*)p->m_rgba_frame)->linesize, 0, p->m_height, \
							((AVFrame*)p->m_uyvy422_frame)->data, ((AVFrame*)p->m_uyvy422_frame)->linesize);
	if (ret != p->m_height) {
		fprintf(stderr, " rgba to yuv422 falied ret %d\n", ret);
		return 0;
	}

	memcpy(buffer, p->m_output_uyvy422_buffer, p->m_output_uyvy422_buffer_len);

	if (g_uyvy_fp == NULL) {
		g_uyvy_fp = fopen("1.yuv422", "wb");
	}
	if (g_uyvy_fp) {
		fwrite(p->m_output_uyvy422_buffer, 1, p->m_output_uyvy422_buffer_len, g_uyvy_fp);
	}

	return p->m_output_uyvy422_buffer_len;
}


CSdiOutput::CSdiOutput() :m_deck_link_output_device(NULL), m_scale_ctx(NULL), m_uyvy422_frame(NULL), m_rgba_frame(NULL) {
	m_width = 1920;
	m_height = 1080;
}

CSdiOutput::CSdiOutput(int width, int height, int frame_rate):m_deck_link_output_device(NULL), m_scale_ctx(NULL), m_uyvy422_frame(NULL), m_rgba_frame(NULL) {
	m_width = width;
	m_height = height;
	m_frame_rate = frame_rate;
}

bool CSdiOutput::start(int k) {
	m_deck_link_output_device = new CDeckLinkOutputDevice();
	m_scale_ctx = sws_getContext(m_width, m_height, AV_PIX_FMT_RGBA, m_width, m_height, AV_PIX_FMT_UYVY422, SWS_BILINEAR, 0, 0, 0);

	m_uyvy422_frame = av_frame_alloc();
	m_rgba_frame =  av_frame_alloc();

	m_input_rgba_buffer_len = m_width*m_height * 4;
	m_input_rgba_buffer = new char[m_input_rgba_buffer_len];

	m_output_uyvy422_buffer_len = m_width*m_height * 2;
	m_output_uyvy422_buffer = new char[m_output_uyvy422_buffer_len];


	if (false == ((CDeckLinkOutputDevice*)m_deck_link_output_device)->CreateObjects(k, m_width, m_height, m_frame_rate)) {
		return false;
	}
	((CDeckLinkOutputDevice*)m_deck_link_output_device)->setSourceCallback(on_deck_link_output_need_data, this);
	if (false == ((CDeckLinkOutputDevice*)m_deck_link_output_device)->start()) {
		return false;
	}

	return true;
}
bool CSdiOutput::stop() {
	((CDeckLinkOutputDevice*)m_deck_link_output_device)->stop();

	delete[] m_input_rgba_buffer;
	delete[] m_output_uyvy422_buffer;

	av_frame_free((AVFrame**)&m_rgba_frame);
	av_frame_free((AVFrame**)&m_uyvy422_frame);

	return false;
}

void CSdiOutput::setSourceCallback(SourceDataCallback _cb, void* ctx) {
	m_cb_need_data = _cb;
	m_cb_need_data_ctx = ctx;
}

