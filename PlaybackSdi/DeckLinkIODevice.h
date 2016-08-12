#ifndef __DECK_LINK_IO_DEVICE_H__
#define __DECK_LINK_IO_DEVICE_H__

//#define DLL_API __declspec(dllexport)  
#define DLL_API   

#define MAX_DEVICES_COUNT 8
typedef int(*SourceDataCallback)(char*, int, void*);
typedef int(*SinkDataCallback)(char*, int, void*);
typedef int(*PixelFormatChangedCallback)(int, char*, void*);
class DLL_API CDeckLinkInputDevice {
public:
	CDeckLinkInputDevice();

	BOOL CreateObjects(int k) {
		return FALSE;
	}
	void DestroyObjects();
	void setSinkDataCallback(SinkDataCallback cb, void* ctx);
	void setVideoInputFormatChangedCallback(PixelFormatChangedCallback cb, void* ctx);

	BOOL start_capture();
	BOOL stop_capture();

	double get_fps();
private:
	static int		input_reference_counter;
	static void*	inputDeckLinkIterator;
	void*			deckLinkAttributes;
	void*			deckLink;
	void*           deckLinkInput;
	void*			notificationCallback;
};


class DLL_API CDeckLinkOutputDevice {
public:
	CDeckLinkOutputDevice();

	BOOL CreateObjects(int k);
	void DestroyObjects();
	void setSourceCallback(SourceDataCallback cb, void* ctx);
	//void setVideoInputFormatChangedCallback(PixelFormatChangedCallback cb, void* ctx);

	BOOL start();
	BOOL stop();

	void  convertFrameCountToTimecode(/*INT32_UNSIGNED*/unsigned int frameCount, /*INT8_UNSIGNED*/unsigned char* hours, /*INT8_UNSIGNED*/unsigned char* minutes, /*INT8_UNSIGNED*/unsigned char* seconds, /*INT8_UNSIGNED*/unsigned char* frames);
	long  setRP188VitcTimecodeOnFrame(/*IDeckLinkMutableVideoFrame*/void* videoFrame, /*INT8_UNSIGNED*/unsigned char hours, /*INT8_UNSIGNED*/unsigned char minutes, /*INT8_UNSIGNED*/unsigned char seconds, /*INT8_UNSIGNED*/unsigned char frames);
private:
	void* CreateFrame(void* deckLinkOutput);
	void  FillBlue(void* theFrame);

private:
	static int		output_reference_counter;

	//IDeckLinkIterator*      deckLinkIterator = NULL;
	//IDeckLink*              deckLink = NULL;
	//IDeckLinkOutput*        deckLinkOutput = NULL;
	//OutputCallback*         outputCallback = NULL;
	//IDeckLinkVideoFrame*    videoFrameBlue = NULL;
	//IDeckLinkDisplayMode*   gDisplayMode = NULL;
public:
	static void*	outputDeckLinkIterator;
	void*			deckLinkAttributes;
	void*			deckLink;
	void*           deckLinkOutput;
	void*			outputCallback;
	void*			deckLinkDisplayMode;//IDeckLinkDisplayMode*
	void*			videoFrameBlue; //IDeckLinkVideoFrame*

	SourceDataCallback m_cb;
	void*			   m_cb_ctx;

	long long            frameDuration;//BMDTimeValue
	long long            timeScale;//BMDTimeScale
};




#endif // !__DECK_LINK_IO_DEVICE_H__

