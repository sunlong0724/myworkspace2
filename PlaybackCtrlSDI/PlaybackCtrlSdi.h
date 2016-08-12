#ifndef __PLAYBACK_CTRL_SDI_H__
#define __PLAYBACK_CTRL_SDI_H__


#include <string>
#include "MyThread.h"
typedef int(*on_have_data_cb)(unsigned char*, int, void*);

class CPlaybackCtrlSdi :public CMyThread{
public:
	CPlaybackCtrlSdi();
	~CPlaybackCtrlSdi();

	void	set_cb_have_data(on_have_data_cb cb, void* context);
	bool    start(int sdi_index);
	bool	stop();

	int32_t	set_play_frame_resolution(const int32_t width, const int32_t height);//FIXME:  
	int32_t set_play_frame_rate(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t set_store_file(const int32_t flag,char* file_name = NULL);

	int32_t play_live(const int32_t play_frame_rate, const int32_t sample_gap);;
	int32_t play_forward(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t play_backward(const int32_t play_frame_rate, const int32_t sample_gap);
	int32_t play_pause();
	int32_t	play_from_a2b(const int64_t a, const int64_t b);

	double  get_grab_fps();
	double  get_write_file_fps();

public:
	enum Playback_Status {
		Pb_STATUS_NONE,
		Pb_STATUS_PAUSE,
		Pb_STATUS_LIVE,
		Pb_STATUS_BACKWARD,
		Pb_STATUS_FORWARD,
		Pb_STATUS_FROM_A2B_LOOP,
	};
	int exchanged_status(Playback_Status next);

	int32_t	write_yuv_data(char* buffer, int len);
	int32_t	read_yuv_data(char* buffer, int len);
protected:
	void run();

public:
	std::map<int64_t, int64_t>  m_frame_offset_map;
	int64_t				m_frame_counter;
	int64_t				m_written_bytes;
	bool				m_store_file_flag;
	FILE*				m_fp;
	int64_t				m_playback_frame_no;
	int64_t				m_play_frame_gap;
	int64_t				m_last_live_play_seq;
	on_have_data_cb		m_cb_have_data;
	void*				m_cb_have_data_ctx;
	bool				m_live_frame_flag;

private:
	void*				m_decklink_input_obj; //CDeckLinkInputDevice*
	void*				m_encoder_thread;     //CEncodeThread*  //don't need write thread temporarily
	//void*				m_file_writer_thread;



	int32_t				m_image_w;
	int32_t				m_image_h;

	int32_t				m_play_frame_w;
	int32_t				m_play_frame_h;

	int32_t				m_play_frame_rate;
	int32_t				m_sample_gap;



	Playback_Status		m_status;
	Playback_Status		m_last_status;

	void*				m_ring_buffer_for_yuv;
	char				m_file_name[1024];
};

#endif // !__PLAYBACK_CTRL_SDI_H__
