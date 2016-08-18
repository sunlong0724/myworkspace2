#ifndef __PLAYBACK_CTRL_SDI_H__
#define __PLAYBACK_CTRL_SDI_H__
#define DLL_API __declspec(dllexport)  

#include <string>
#include "MyThread.h"
#include <map>

typedef int(*on_have_data_cb)(unsigned char*, int, void*);
typedef int(*on_format_changed_cb)(int,int,double,void*);//width,height,frame_rate


class DLL_API CPlaybackCtrlSdi :public CMyThread{
public:
	CPlaybackCtrlSdi();
	~CPlaybackCtrlSdi();

	void	set_format_changed(on_format_changed_cb, void* context);
	void	set_cb_have_data(on_have_data_cb cb, void* context);
	bool    start(int type, int sdi_index);
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
	double  get_full_frame_rate();

	////The src_fmt/dst_fmt value is 1 when the format is AV_PIX_FMT_YUYV422 
	int32_t	scale_image(char* src_yuv, int src_len, int src_w, int src_h,int src_fmt, char* dst_yuv, int dst_len, int dst_w, int dst_h, int dst_fmt);

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
	void calc_play_frame_no(int gap);
protected:
	void run();

public:

	int64_t				m_bytes_written;
	static __int64				m_max_file_size;

	bool				m_has_reach_max_file_size;

	double				m_full_frame_rate;

	int64_t				m_frame_counter;
	bool				m_store_file_flag;
	int64_t				m_play_frame_gap;
	int64_t				m_last_live_play_seq;

	int64_t				m_last_decode_seq;

	on_have_data_cb		m_cb_have_data;
	void*				m_cb_have_data_ctx;

	on_format_changed_cb	m_cb_format_changed;
	void*					m_cb_format_changed_ctx;
	bool				m_live_frame_flag;


	void*				m_decklink_input_obj; //CDeckLinkInputDevice*
	void*				m_ffmpeg_encoder;     //CEncodeThread*  //don't need write thread temporarily
	void*				m_ffmpeg_decoder;

	Playback_Status		m_status;
	Playback_Status		m_last_status;

	FILE*				m_fp_writter;
	FILE*				m_fp_reader;
	std::map<int64_t, int64_t> m_frame_offset_map;


	int64_t				m_playback_frame_seq_find;

	int64_t				m_from_a2b_from;
	int64_t				m_from_a2b_to;
	int64_t				m_from_a2b_index;
	bool				m_toward_2b;

	char*				m_io_buffer;
	int64_t				m_io_buffer_len;

	char*				m_read_buffer_for_ctrl;
	int64_t				m_read_buffer_for_ctrl_len;
	void*				m_ring_buffer_for_ctrl;


	char*				m_read_buffer_for_decoder;
	int64_t				m_read_buffer_for_decoder_len;
	void*				m_ring_buffer_for_decoder;


	void*				m_grab_fps_counter;
	void*				m_write_file_fps_counter;

	bool				m_resized_flag;
	void*				m_scale_ctx_for_resized;
	char*				m_resized_yuy2_buffer;
	int64_t				m_resized_yuy2_buffer_len;

	void*				m_scale_ctx_uyvy422_to_yuv420p;
	char*				m_yuv420p_buffer;
	int64_t				m_yuv420p_buffer_len;

	void*				m_scale_ctx_yuv420p_to_yuy2;
	void*				m_scale_ctx_uyvy422_to_yuy2;

	char*				m_out_yuy2_buffer;
	int64_t				m_out_yuy2_buffer_len;

	char*				m_uyvy422_buffer;
	int64_t				m_uyvy422_buffer_len;

	void*				m_yuv422_frame;
	void*				m_yuv420_frame;

	void*					m_output_yuy2_frame;
	char*					m_output_uyvy422_buffer;
	int32_t					m_output_uyvy422_buffer_len;

	void*				m_yuv_frame_in;
	void*				m_resized_yuv_frame;
	void*				m_yuv_frame_out;

	int32_t				m_image_w;
	int32_t				m_image_h;

	int32_t				m_play_frame_w;
	int32_t				m_play_frame_h;

	int32_t				m_play_frame_rate;

	char				m_file_name[1024];
	void*				m_ring_buffer_for_yuv;

	int					m_type;//1--cctv.com input, 2--3d input

	
};

typedef int(*SourceDataCallback)(char*, int, void*);

class DLL_API CSdiOutput {
public:
	CSdiOutput();
	CSdiOutput(int w, int h, int frame_rate);
	bool start(int k);
	bool stop();

	void setSourceCallback(SourceDataCallback _cb, void* ctx);


	SourceDataCallback		m_cb_need_data;
	void*				    m_cb_need_data_ctx;

	void*					m_scale_ctx;
	void*					m_uyvy422_frame;
	void*					m_rgba_frame;
	//void*					m_yuv420p_frame;

	char*					m_output_uyvy422_buffer;
	int32_t					m_output_uyvy422_buffer_len;

	//char*					m_output_yuv420_buffer;
	//int32_t					m_output_yuv420_buffer_len;

	char*					m_input_rgba_buffer;
	int32_t					m_input_rgba_buffer_len;

	int32_t					m_width;
	int32_t					m_height;
	int32_t					m_frame_rate;
private:
	void *m_deck_link_output_device;//CDeckLinkOutputDevice*

};


#endif // !__PLAYBACK_CTRL_SDI_H__
