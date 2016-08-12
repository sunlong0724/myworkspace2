#include "PlaybackCtrlSdi.h"

#include "DeckLinkIODevice.h"
#include "msdk_codec.h"
#include "utils.h"

#include <functional>


#pragma comment(lib, "Winmm.lib")
#ifdef _DEBUG         
#pragma comment(lib, "msdk_codecd.lib")
#else 
#pragma comment(lib, "msdk_codec.lib")
#endif

#if 1

int32_t	decklink_device_sink_cb(char* buffer, int32_t len, void* ctx) {
	CPlaybackCtrlSdi* p = (CPlaybackCtrlSdi*)ctx;
	return p->write_yuv_data(buffer, len);
}


int32_t encoder_read_next_frame(unsigned char* buffer, int32_t len, void* ctx) {
	CPlaybackCtrlSdi* p = (CPlaybackCtrlSdi*)ctx;
	return p->read_yuv_data((char*)buffer, len);
}

int32_t encoder_write_next_frame(unsigned char* buffer, int32_t len, void* ctx) {
	CPlaybackCtrlSdi* p = (CPlaybackCtrlSdi*)ctx;
	int32_t nbyte_written = 0;
	if (p->m_store_file_flag && p->m_fp) {
		p->m_frame_offset_map.insert(std::make_pair(p->m_frame_counter, p->m_written_bytes));
		nbyte_written = fwrite(buffer, 1, len, p->m_fp);
	}

	//check the data whether need!!!
	if (p->m_play_frame_gap != 0 && p->m_last_live_play_seq + p->m_play_frame_gap != p->m_frame_counter) {
		return nbyte_written;
	}
	//fprintf(stdout, "####m_last_live_play_seq %d, m_frame_counter %d, m_play_frame_gap %d\n", p->m_last_live_play_seq, p->m_frame_counter, p->m_play_frame_gap);

	if (p->m_live_frame_flag) {
		if (p->m_cb_have_data) {
			p->m_cb_have_data(buffer, len, p->m_cb_have_data_ctx);
		}
	}
	return nbyte_written;
}

CPlaybackCtrlSdi::CPlaybackCtrlSdi() :m_store_file_flag(false), m_status(Pb_STATUS_NONE), m_last_status(Pb_STATUS_NONE), m_image_w(MAX_IMAGE_WIDTH), \
m_image_h(MAX_IMAGE_HEIGHT), m_frame_counter(0), m_fp(NULL), m_playback_frame_no(0), m_cb_have_data(NULL), m_cb_have_data_ctx(NULL), m_last_live_play_seq(0), m_play_frame_gap(0), m_live_frame_flag(false){
	m_decklink_input_obj = new CDeckLinkInputDevice;
	m_encoder_thread = new CEncodeThread;

	//m_ring_buffer_for_yuv = RingBuffer_create(GET_IMAGE_BUFFER_SIZE(m_image_w, m_image_h));

}
CPlaybackCtrlSdi::~CPlaybackCtrlSdi() {
	delete m_encoder_thread;
	delete m_decklink_input_obj;
}

bool CPlaybackCtrlSdi::start(int sdi_index) {

	std::string parameter("-g ");//("-g 1920x1080 -b 30000 -f 30/1 -gop 1")
	parameter.append(std::to_string(m_image_w) + "x" + std::to_string(m_image_h));
	parameter.append(" -b 30000 -f ");
	parameter.append(std::to_string(MAX_FPS) + "/1 -gop 1");

	((CEncodeThread*)m_encoder_thread)->init( parameter.c_str());
	((CEncodeThread*)m_encoder_thread)->start(std::bind(&encoder_read_next_frame, std::placeholders::_1, std::placeholders::_2, this), std::bind(&encoder_write_next_frame, std::placeholders::_1, std::placeholders::_2, this));

	if (false == ((CDeckLinkInputDevice*)m_decklink_input_obj)->CreateObjects(sdi_index)) {
		return false;
	}
	if (false == ((CDeckLinkInputDevice*)m_decklink_input_obj)->start_capture()) {
		return false;
	}
	return true;
}

bool CPlaybackCtrlSdi::stop() {

	if (false == ((CDeckLinkInputDevice*)m_decklink_input_obj)->stop_capture()) {
		return false;
	}

	((CDeckLinkInputDevice*)m_decklink_input_obj)->DestroyObjects();
	((CEncodeThread*)m_encoder_thread)->stop();

	return true;
}

void CPlaybackCtrlSdi::set_cb_have_data(on_have_data_cb cb, void* context) {
	m_cb_have_data = cb;
	m_cb_have_data_ctx = context;
}

int32_t	CPlaybackCtrlSdi::set_play_frame_resolution(const int32_t width, const int32_t height) {
	if (height == m_play_frame_h&& width == m_play_frame_w) {
		return 0;
	}

	m_play_frame_w = width;
	m_play_frame_h = height;

	return 1;
}
int32_t CPlaybackCtrlSdi::set_play_frame_rate(const int32_t play_frame_rate, const int32_t sample_gap) {
	if (play_frame_rate == m_play_frame_rate && sample_gap == m_sample_gap) {
		return 0;
	}
	m_play_frame_rate = play_frame_rate;
	m_sample_gap = sample_gap;
	return 1;
}
int32_t CPlaybackCtrlSdi::set_store_file(const int32_t flag, char* file_name) {
	if (file_name) {
		strcpy(m_file_name, file_name);
		if (NULL == m_fp) {
			m_fp = fopen(m_file_name, "wb");
		}
		if (NULL == m_fp) {
			fprintf(stderr, "%s fopen failed!!!\n", __FUNCTION__);
			return -1;
		}
	}
	return m_store_file_flag = flag;
}

int CPlaybackCtrlSdi::exchanged_status(CPlaybackCtrlSdi::Playback_Status next) {
	m_last_status = m_status;
	return m_status = next;
}

int32_t CPlaybackCtrlSdi::play_live(const int32_t play_frame_rate, const int32_t sample_gap) {
	return exchanged_status(Pb_STATUS_LIVE);
}
int32_t CPlaybackCtrlSdi::play_forward(const int32_t play_frame_rate, const int32_t sample_gap) {
	return exchanged_status(Pb_STATUS_FORWARD);
}
int32_t CPlaybackCtrlSdi::play_backward(const int32_t play_frame_rate, const int32_t sample_gap) {
	return exchanged_status(Pb_STATUS_BACKWARD);
}
int32_t CPlaybackCtrlSdi::play_pause() {
	return exchanged_status(Pb_STATUS_PAUSE);
}
int32_t	CPlaybackCtrlSdi::play_from_a2b(const int64_t a, const int64_t b) {
	return exchanged_status(Pb_STATUS_FROM_A2B_LOOP);
}

double  CPlaybackCtrlSdi::get_grab_fps() {
	return .0;
}
double  CPlaybackCtrlSdi::get_write_file_fps() {
	return .0;
}


void CPlaybackCtrlSdi::run() {
	timeBeginPeriod(1);

	while (true) {
		int64_t	now = get_current_time_in_ms();
		if (m_exited)
			break;

		if (Pb_STATUS_NONE == m_status || Pb_STATUS_PAUSE == m_status) {
			sleep(1);
			continue;
		}
		else if (Pb_STATUS_FORWARD == m_status || Pb_STATUS_BACKWARD == m_status) {
		
		}
		else if (Pb_STATUS_LIVE == m_status) {

		}
		else if (Pb_STATUS_FROM_A2B_LOOP == m_status) {

		}
	
		int64_t	now2 = get_current_time_in_ms();
		double t = 1000.0f / ((double)m_play_frame_rate + 2.0f);
		if (now2 - now < t) {
			//fprintf(stdout, " sleep:%f", t - (double)(now2 - now));
			sleep(t - (double)(now2 - now));
		}
	}
	timeEndPeriod(1);
}

int32_t	CPlaybackCtrlSdi::write_yuv_data(char* buffer, int len) {
	int ret = RingBuffer_write((RingBuffer*)m_ring_buffer_for_yuv, buffer, len);
	if (-1 == ret) {
		fprintf(stderr, "%s m_ring_buffer_for_yuv is full!!!\n", __FUNCTION__);
	}
	return ret;
}

int32_t	CPlaybackCtrlSdi::read_yuv_data(char* buffer, int len) {
	int ret = RingBuffer_read((RingBuffer*)m_ring_buffer_for_yuv, buffer, len);
	if (-1 == ret) {
		fprintf(stderr, "%s m_ring_buffer_for_yuv is empty!!!\n", __FUNCTION__);
	}
	return ret;
}

#endif

