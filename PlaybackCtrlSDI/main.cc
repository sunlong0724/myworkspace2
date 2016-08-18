
#include "PlaybackCtrlSdi.h"
#include "utils.h"
#include "signal.h"

#include "opencv2/opencv.hpp"
#include "defs.h"

//#ifdef _DEBUG
//#pragma comment(lib, "opencv_core246d.lib")
//#pragma comment(lib, "opencv_highgui246d.lib")
//#pragma comment(lib, "opencv_imgproc246d.lib")
//#else
#pragma comment(lib, "opencv_core246.lib")
#pragma comment(lib, "opencv_highgui246.lib")
#pragma comment(lib, "opencv_imgproc246.lib")
//#endif

extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
}


#if 1
bool g_running_flag = true;
CPlaybackCtrlSdi client;
int64_t				g_recv_frame_no;
IplImage*			g_yuv_image = NULL;
IplImage*			g_rgb_image = NULL;
int64_t				g_width = MAX_IMAGE_WIDTH;
int64_t				g_height = MAX_IMAGE_HEIGHT;

int64_t				g_frame_a = 0;
int64_t				g_frame_b = 0;
BOOL				g_mode_ab_for_a_pressed = FALSE;
BOOL				g_mode_ab_for_b_pressed = FALSE;
BOOL				g_store_file = TRUE;
double				g_full_frame_rate = .0f;

void sig_cb(int sig)
{
	if (sig == SIGINT) {
		fprintf(stdout, "%s\n", __FUNCTION__);
		g_running_flag = false;
	}
}

char* g_rgb_buffer = new char[MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * 4];
void * g_scal_ctx = NULL;
FILE* g_rgba_fp = NULL;
int onHaveDataCb(unsigned char* data, int data_len, void* ctx) {
	int width = g_width;
	int height = g_height;
	int nByteWritten = 0;
	int64_t	 seq = 0;
	memcpy(&seq, &data[FRAME_SEQ_START], sizeof int64_t);
	g_recv_frame_no = seq;

	//if (NULL == g_scal_ctx)
	{
		g_scal_ctx = sws_getContext(width, height, AV_PIX_FMT_YUYV422, width, height, AV_PIX_FMT_BGRA, SWS_BILINEAR, 0, 0, 0);
	}
	//scale
	AVFrame*  yuv420_frame = av_frame_alloc();
	AVFrame*	rgb_frame = av_frame_alloc();
	av_image_fill_arrays(yuv420_frame->data, yuv420_frame->linesize, (const uint8_t*)(&data[FRAME_DATA_START]), AV_PIX_FMT_YUYV422, width, height, 16);
	av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, (const uint8_t*)g_rgb_buffer, AV_PIX_FMT_BGRA, width, height, 1);

	int ret = sws_scale((SwsContext*)g_scal_ctx, yuv420_frame->data, yuv420_frame->linesize, 0, height, rgb_frame->data, rgb_frame->linesize);
	if (ret != height) {
		fprintf(stderr, "yuv420p to rgba falied ret %d\n", ret);
		return -1;
	}

	g_rgb_image = cvCreateImage(cvSize(width, height), 8, 4);
	//CV_BayerBG2BGR

	memcpy(g_rgb_image->imageData, g_rgb_buffer, width*height * 4);
	cvShowImage("mywin0", g_rgb_image);
	cvWaitKey(1);

	cvReleaseImage(&g_rgb_image);
	sws_freeContext((SwsContext*)g_scal_ctx);
	av_frame_free(&yuv420_frame);
	av_frame_free(&rgb_frame);


	//if (g_rgba_fp == NULL) {
	//	g_rgba_fp = fopen("1.rgbas", "wb");
	//}

	//if (g_rgba_fp) {
	//	fwrite(g_rgb_buffer, 1, width*height * 4, g_rgba_fp);
	//}
	return 0;
}


void run() {
	while (g_running_flag) {

		char ci = getchar();
		fprintf(stdout, "####%c was pressed!\n", ci);
		switch (ci)
		{
		case 'p'://pause
			printf("pause frame no %d\n", g_recv_frame_no);
			client.play_pause();
			break;
		case 'f'://forward
			client.play_forward(g_full_frame_rate, 1);
			break;
		case 'b'://backplay
			client.play_backward(g_full_frame_rate, 1);
			break;
		case 'l'://live
			g_frame_a = g_frame_b = 0;
			g_width = MAX_IMAGE_WIDTH;
			g_height = MAX_IMAGE_HEIGHT;
			client.set_play_frame_resolution(g_width, g_height);
			client.play_live(g_full_frame_rate, 1);
			break;
		case 'k'://live
			g_frame_a = g_frame_b = 0;
			g_width = MAX_IMAGE_WIDTH/6;
			g_height = MAX_IMAGE_HEIGHT/6;
			client.set_play_frame_resolution(g_width, g_height);
			client.play_live(g_full_frame_rate, 1);
			break;
		case 's'://live
			g_width = MAX_IMAGE_WIDTH;
			g_height = MAX_IMAGE_HEIGHT;
			client.set_play_frame_resolution(g_width, g_height);
			break;
		case 'd'://live
			g_width = MAX_IMAGE_WIDTH / 6;
			g_height = MAX_IMAGE_HEIGHT / 6;
			client.set_play_frame_resolution(g_width, g_height);
			break;
		case 't':
			g_store_file = !g_store_file;
			client.set_store_file(g_store_file);
			break;
		case ' '://abmode
			if (0 == g_frame_a) {
				g_frame_a = g_recv_frame_no;
				break;
			}

			if (0 == g_frame_b) {
				g_frame_b = g_recv_frame_no;
				client.play_from_a2b(g_frame_a, g_frame_b);
				break;
			}
		case 'q':
			g_running_flag = false;
		default:
			break;
		}
	}
}

int deck_link_output_callback(char* buffer, int len, void* ctx){
	int nByteRead = 0;
	if (g_rgba_fp) {
		nByteRead = fread(buffer, 1, g_width*g_height * 4, g_rgba_fp);
		fprintf(stdout, "%s nByteRead %d\n", __FUNCTION__, nByteRead);
	}
	return nByteRead;
}

int on_format_changed_cb0(int width, int height, double frame_rate, void* ctx) {//width,height,frame_rate
	client.set_cb_have_data(onHaveDataCb, &client);
	client.set_store_file(g_store_file, "outmy.h264");

	g_frame_a = g_frame_b = 0;
	/*g_width = MAX_IMAGE_WIDTH / 6;
	g_height = MAX_IMAGE_HEIGHT / 6;
	*/
	g_width = MAX_IMAGE_WIDTH;
	g_height = MAX_IMAGE_HEIGHT;
	client.set_play_frame_resolution(g_width, g_height);

	g_full_frame_rate = client.get_full_frame_rate();
	client.play_live(g_full_frame_rate, 1);
	return 0;
}

int main(int argc, char** argv) {

	std::thread t(run);
	client.set_format_changed(on_format_changed_cb0, NULL);
	client.start(1, 3);

	//sleep(2000);

	

	while (g_running_flag) {
		::Sleep(2);
	}

	client.stop();
	t.join();
	fprintf(stdout, "%s  exited!\n", __FUNCTION__);

//	fclose(g_rgba_fp);

#ifdef test_local
	int m_width = 1920;
	int m_height = 1080;

	void*					m_scale_ctx;
	void*					m_uyvy422_frame;
	void*					m_rgba_frame;
	void*					m_yuv420p_frame;

	char*					m_output_uyvy422_buffer;
	int32_t					m_output_uyvy422_buffer_len;

	char*					m_output_yuv420_buffer;
	int32_t					m_output_yuv420_buffer_len;

	char*					m_input_rgba_buffer;
	int32_t					m_input_rgba_buffer_len;


	m_scale_ctx = sws_getContext(m_width, m_height, AV_PIX_FMT_RGBA, m_width, m_height, AV_PIX_FMT_UYVY422, SWS_BILINEAR, 0, 0, 0);

	m_uyvy422_frame = av_frame_alloc();
	m_rgba_frame = av_frame_alloc();
	m_yuv420p_frame = av_frame_alloc();

	m_input_rgba_buffer_len = m_width*m_height * 4;
	m_input_rgba_buffer = new char[m_input_rgba_buffer_len];

	m_output_uyvy422_buffer_len = m_width*m_height * 2;
	m_output_uyvy422_buffer = new char[m_output_uyvy422_buffer_len];

	m_output_yuv420_buffer_len = m_width*m_height * 3 / 2;
	m_output_yuv420_buffer = new char[m_output_yuv420_buffer_len];
	

	FILE* out_yuv = fopen("1.yuv422", "wb");

	FILE* fp = fopen("1.rgbas", "rb");
	while (0 != fread(m_input_rgba_buffer, 1, m_input_rgba_buffer_len, fp) ) {
		av_image_fill_arrays(((AVFrame*)m_rgba_frame)->data, ((AVFrame*)m_rgba_frame)->linesize, (const uint8_t*)m_input_rgba_buffer, AV_PIX_FMT_RGBA, m_width, m_height, 1);
		av_image_fill_arrays(((AVFrame*)m_uyvy422_frame)->data, ((AVFrame*)m_uyvy422_frame)->linesize, (const uint8_t*)m_output_uyvy422_buffer, AV_PIX_FMT_UYVY422, m_width, m_height, 16);

		int ret = sws_scale((SwsContext*)m_scale_ctx, ((AVFrame*)m_rgba_frame)->data, ((AVFrame*)m_rgba_frame)->linesize, 0, m_height, ((AVFrame*)m_uyvy422_frame)->data, ((AVFrame*)m_uyvy422_frame)->linesize);
		if (ret != m_height) {
			fprintf(stderr, " rgba to yuv422 falied ret %d\n", ret);
			return 0;
		}
		fwrite(m_output_uyvy422_buffer, 1, m_output_uyvy422_buffer_len, out_yuv);
	}

	fclose(out_yuv);
	fclose(fp);
#endif

#if 0

	int k = 0;

	g_width = 1280;
	g_height = 720;

	char* name = "1.rgbas";

	if (argc > 1) {
		k = atoi(argv[1]);
	}

	if (argc > 2) {
		g_width = atoi(argv[2]);
	}

	if (argc > 3) {
		g_height = atoi(argv[3]);
	}

	if (argc > 4) {
		name = argv[4];
	}

	fprintf(stdout, "%s ##############3111\n", __FUNCTION__);
	g_rgba_fp = fopen(name, "rb");
	CSdiOutput sdi(g_width, g_height, 25);
	sdi.setSourceCallback(deck_link_output_callback, NULL);
	sdi.start(k);

	fprintf(stdout, "%s \n", __FUNCTION__);

	while (g_running_flag) {
		fprintf(stdout, "%s 1\n", __FUNCTION__);
		::Sleep(2);
	}

	sdi.stop();

#endif

	return 0;

}

#endif 0