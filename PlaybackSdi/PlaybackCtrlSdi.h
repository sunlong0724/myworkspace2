#ifndef __PLAYBACK_CTRL_SDI_H__
#define __PLAYBACK_CTRL_SDI_H__

typedef long BOOL;

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#include <string>
#include "MyThread.h"
typedef int(*on_have_data_cb)(unsigned char*, int, void*);

class CPlaybackCtrlSdi :public CMyThread{
public:

	CPlaybackCtrlSdi();
	~CPlaybackCtrlSdi();

	void	set_cb_have_data(on_have_data_cb cb, void* context);
	BOOL    start(int sdi_index);
	BOOL	stop();

	int32_t	set_play_frame_resolution(const int32_t width, const int32_t height);//FIXME:  
	int32_t set_play_frame_rate(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t set_store_file(const int32_t flag);

	int32_t play_live(const int32_t play_frame_rate, const int32_t sample_gap);;
	int32_t play_forward(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t play_backward(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t play_pause();
	int32_t	play_from_a2b(const int64_t a, const int64_t b);

	double  get_grab_fps();
	double  get_write_file_fps();

private:

	on_have_data_cb		m_cb_have_data;
	void*				m_cb_have_data_ctx;

	void*				m_decklink_input_obj; //CDeckLinkInputDevice*
	void*				m_encoder_thread;     //CEncodeThread*  //don't need write thread temporarily
};

#endif // !__PLAYBACK_CTRL_SDI_H__
