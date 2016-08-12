#include "PlaybackCtrlSdi.h"

#include "DeckLinkIODevice.h"
#include "msdk_codec.h"

#include <functional>

#ifdef _DEBUG         
#pragma comment(lib, "msdk_codecd.lib")
#else 
#pragma comment(lib, "msdk_codec.lib")
#endif

#if 1

int32_t ReadNextFrame(unsigned char* buffer, int32_t len, void* ctx) {

	return 0;
}

int32_t WriteNextFrame(unsigned char* buffer, int32_t len, void* ctx) {

	return 0;
}

CPlaybackCtrlSdi::CPlaybackCtrlSdi() {
	m_decklink_input_obj = new CDeckLinkInputDevice;
	m_encoder_thread = new CEncodeThread;
}
CPlaybackCtrlSdi::~CPlaybackCtrlSdi() {

}

BOOL CPlaybackCtrlSdi::start(int sdi_index) {

	if (FALSE == ((CDeckLinkInputDevice*)m_decklink_input_obj)->CreateObjects(sdi_index)) {
		return FALSE;
	}

	((CEncodeThread*)m_encoder_thread)->start(std::bind(&ReadNextFrame, std::placeholders::_1, std::placeholders::_2, this), std::bind(&WriteNextFrame, std::placeholders::_1, std::placeholders::_2, this));

	   
	return FALSE;
}

BOOL	CPlaybackCtrlSdi::stop() {

	return FALSE;
}
void	CPlaybackCtrlSdi::set_cb_have_data(on_have_data_cb cb, void* context) {

}

int32_t	CPlaybackCtrlSdi::set_play_frame_resolution(const int32_t width, const int32_t height) {
	return 0;
}
int32_t CPlaybackCtrlSdi::set_play_frame_rate(const int32_t play_frame_rate, const int32_t sample_gap) {
	return 0;
}
int32_t CPlaybackCtrlSdi::set_store_file(const int32_t flag) {
	return 0;
}

int32_t CPlaybackCtrlSdi::play_live(const int32_t play_frame_rate, const int32_t sample_gap) {
	return 0;
}
int32_t CPlaybackCtrlSdi::play_forward(const int32_t play_frame_rate, const int32_t sample_gap) {
	return 0;
}
int32_t CPlaybackCtrlSdi::play_backward(const int32_t play_frame_rate, const int32_t sample_gap) {
	return 0;
}
int32_t CPlaybackCtrlSdi::play_pause() {
	return 0;
}
int32_t	CPlaybackCtrlSdi::play_from_a2b(const int64_t a, const int64_t b) {
	return 0;
}

double  CPlaybackCtrlSdi::get_grab_fps() {
	return .0;
}
double  CPlaybackCtrlSdi::get_write_file_fps() {
	return .0;
}

#endif

