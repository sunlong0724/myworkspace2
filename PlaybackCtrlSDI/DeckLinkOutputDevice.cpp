/* -LICENSE-START-
** Copyright (c) 2014 Blackmagic Design
**
** Permission is hereby granted, free of charge, to any person or organization
** obtaining a copy of the software and accompanying documentation covered by
** this license (the "Software") to use, reproduce, display, distribute,
** execute, and transmit the Software, and to prepare derivative works of the
** Software, and to permit third-parties to whom the Software is furnished to
** do so, all subject to the following:
**
** The copyright notices in the Software and this entire statement, including
** the above license grant, this restriction and the following disclaimer,
** must be included in all copies of the Software, in whole or in part, and
** all derivative works of the Software, unless such copies or derivative
** works are solely in the form of machine-executable object code generated by
** a source language processor.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
** SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
** FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
** -LICENSE-END-
*/

#include "platform.h"


//
//namespace decklink_api {
//
//
//	// Video mode parameters
//	const BMDDisplayMode      kDisplayMode = bmdModeHD1080i50;
//	const BMDVideoOutputFlags kOutputFlags = bmdVideoOutputRP188;
//	const BMDPixelFormat      kPixelFormat = bmdFormat10BitYUV;
//
//	// Frame parameters
//	const INT32_UNSIGNED kRowBytes = 5120;
//	BMDTimeValue         gFrameDuration = 0;
//	BMDTimeScale         gTimeScale = 0;
//
//	// Timecode options
//	const bool kIsDropFrame = false;
//
//	// 10-bit YUV blue pixels
//	const			 INT32_UNSIGNED kBlueData[4] = { 0x40aa298, 0x2a8a62a8, 0x298aa040, 0x2a8102a8 };
//
//	// The display mode object corresponding to kDisplayMode
//	IDeckLinkDisplayMode* gDisplayMode = NULL;
//
//	// Keep track of the number of scheduled frames
//	INT32_UNSIGNED gTotalFramesScheduled = 0;
//
//	static void convertFrameCountToTimecode(INT32_UNSIGNED frameCount, INT8_UNSIGNED* hours, INT8_UNSIGNED* minutes, INT8_UNSIGNED* seconds, INT8_UNSIGNED* frames)
//	{
//		INT32_UNSIGNED maxFPS = (INT32_UNSIGNED)(gTimeScale / 1000);
//
//		// Dropped frame counting only applies to M-rate modes (i.e. Fractional frame rates)
//		if (kIsDropFrame && gFrameDuration == 1001)
//		{
//			INT32_UNSIGNED deciMins, deciMinsRemainder;
//
//			INT32_UNSIGNED framesIn10mins = (INT32_UNSIGNED)((60 * 10 * maxFPS) - (9 * 2));
//			deciMins = frameCount / framesIn10mins;
//			deciMinsRemainder = frameCount - (deciMins * framesIn10mins);
//
//			// Add 2 frames for 9 minutes of every 10 minutes that have elapsed
//			// AND 2 frames for every minute (over the first minute) in this 10-minute block.
//			frameCount += 2 * 9 * deciMins;
//			if (deciMinsRemainder >= 2)
//				frameCount += 2 * ((deciMinsRemainder - 2) / (framesIn10mins / 10));
//		}
//
//		*frames = (INT8_UNSIGNED)(frameCount % maxFPS);
//		frameCount /= maxFPS;
//		*seconds = (INT8_UNSIGNED)(frameCount % 60);
//		frameCount /= 60;
//		*minutes = (INT8_UNSIGNED)(frameCount % 60);
//		frameCount /= 60;
//		*hours = (INT8_UNSIGNED)frameCount;
//	}
//
//	static HRESULT setRP188VitcTimecodeOnFrame(IDeckLinkMutableVideoFrame* videoFrame, INT8_UNSIGNED hours, INT8_UNSIGNED minutes, INT8_UNSIGNED seconds, INT8_UNSIGNED frames)
//	{
//		HRESULT	result = S_OK;
//
//		BMDTimecodeFlags flags = bmdTimecodeFlagDefault;
//		bool setVITC1Timecode = false;
//		bool setVITC2Timecode = false;
//
//		if (kIsDropFrame)
//			flags |= bmdTimecodeIsDropFrame;
//
//		if (gDisplayMode->GetFieldDominance() != bmdProgressiveFrame)
//		{
//			// An interlaced or PsF frame has both VITC1 and VITC2 set with the same timecode value (SMPTE ST 12-2:2014 7.2)
//			setVITC1Timecode = true;
//			setVITC2Timecode = true;
//		}
//		else if (gTimeScale / gFrameDuration <= 30)
//		{
//			// If this isn't a High-P mode, then just use VITC1 (SMPTE ST 12-2:2014 7.2)
//			setVITC1Timecode = true;
//		}
//		else
//		{
//			// If this is a High-P mode then use VITC1 on even frames and VITC2 on odd frames. This is done because the 
//			// frames field of the RP188 VITC timecode cannot hold values greater than 30 (SMPTE ST 12-2:2014 7.2, 9.2)
//			if ((frames & 1) == 0)
//				setVITC1Timecode = true;
//			else
//				setVITC2Timecode = true;
//
//			// Shift the frame count so it's maximum is within the valid range
//			frames >>= 1;
//		}
//
//		if (setVITC1Timecode)
//		{
//			result = videoFrame->SetTimecodeFromComponents(bmdTimecodeRP188VITC1, hours, minutes, seconds, frames, flags);
//			if (result != S_OK)
//			{
//				fprintf(stderr, "Could not set VITC1 timecode on interlaced frame - result = %08x\n", result);
//				goto bail;
//			}
//		}
//
//		if (setVITC2Timecode)
//		{
//			// The VITC2 timecode also has the field mark flag set
//			result = videoFrame->SetTimecodeFromComponents(bmdTimecodeRP188VITC2, hours, minutes, seconds, frames, flags | bmdTimecodeFieldMark);
//			if (result != S_OK)
//			{
//				fprintf(stderr, "Could not set VITC1 timecode on interlaced frame - result = %08x\n", result);
//				goto bail;
//			}
//		}
//
//	bail:
//		return result;
//	}
//
//	class OutputCallback : public IDeckLinkVideoOutputCallback
//	{
//	public:
//		OutputCallback(IDeckLinkOutput* deckLinkOutput)
//		{
//			m_deckLinkOutput = deckLinkOutput;
//			m_deckLinkOutput->AddRef();
//		}
//
//		virtual ~OutputCallback(void)
//		{
//			m_deckLinkOutput->Release();
//		}
//
//		virtual HRESULT	STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult completionResult)
//		{
//			return scheduleNextFrame(completedFrame);
//		}
//
//		virtual HRESULT	STDMETHODCALLTYPE ScheduledPlaybackHasStopped(void)
//		{
//			return S_OK;
//		}
//
//		// IUnknown needs only a dummy implementation
//		virtual HRESULT	STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv)
//		{
//			return E_NOINTERFACE;
//		}
//
//		virtual ULONG STDMETHODCALLTYPE AddRef()
//		{
//			return 1;
//		}
//
//		virtual ULONG STDMETHODCALLTYPE Release()
//		{
//			return 1;
//		}
//
//		virtual HRESULT scheduleNextFrame(IDeckLinkVideoFrame* videoFrame)
//		{
//			HRESULT                     result;
//			IDeckLinkMutableVideoFrame* mutableFrame = NULL;
//			INT8_UNSIGNED               hours;
//			INT8_UNSIGNED               minutes;
//			INT8_UNSIGNED               seconds;
//			INT8_UNSIGNED               frames;
//
//			convertFrameCountToTimecode(gTotalFramesScheduled, &hours, &minutes, &seconds, &frames);
//
//			result = videoFrame->QueryInterface(IID_IDeckLinkMutableVideoFrame, (void**)&mutableFrame);
//			if (result != S_OK)
//				goto bail;
//
//			result = setRP188VitcTimecodeOnFrame(mutableFrame, hours, minutes, seconds, frames);
//			if (result != S_OK)
//				goto bail;
//
//			// When a video frame completes,reschedule another frame
//			result = m_deckLinkOutput->ScheduleVideoFrame(videoFrame, gTotalFramesScheduled * gFrameDuration, gFrameDuration, gTimeScale);
//			if (result != S_OK)
//				goto bail;
//
//			gTotalFramesScheduled++;
//
//		bail:
//			if (mutableFrame)
//				mutableFrame->Release();
//
//			return result;
//		}
//
//	private:
//		IDeckLinkOutput*  m_deckLinkOutput;
//	};
//
//	static void FillBlue(IDeckLinkMutableVideoFrame* theFrame)
//	{
//		INT32_UNSIGNED* nextWord;
//		INT32_UNSIGNED  wordsRemaining;
//		INT32_UNSIGNED  height = (INT32_UNSIGNED)gDisplayMode->GetHeight();
//
//		theFrame->GetBytes((void**)&nextWord);
//		wordsRemaining = (kRowBytes * height) / 4;
//
//		while (wordsRemaining > 0)
//		{
//			*(nextWord++) = kBlueData[0];
//			*(nextWord++) = kBlueData[1];
//			*(nextWord++) = kBlueData[2];
//			*(nextWord++) = kBlueData[3];
//			wordsRemaining = wordsRemaining - 4;
//		}
//	}
//
//	static IDeckLinkMutableVideoFrame* CreateFrame(IDeckLinkOutput* deckLinkOutput)
//	{
//		HRESULT                         result;
//		IDeckLinkMutableVideoFrame*     frame = NULL;
//
//		result = deckLinkOutput->CreateVideoFrame((int)gDisplayMode->GetWidth(), (int)gDisplayMode->GetHeight(), kRowBytes, kPixelFormat, bmdFrameFlagDefault, &frame);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "Could not create a video frame - result = %08x\n", result);
//			goto bail;
//		}
//
//		FillBlue(frame);
//
//	bail:
//		return frame;
//	}
//
//
//	int main(int argc, const char * argv[])
//	{
//
//		IDeckLinkIterator*      deckLinkIterator = NULL;
//		IDeckLink*              deckLink = NULL;
//		IDeckLinkOutput*        deckLinkOutput = NULL;
//		OutputCallback*         outputCallback = NULL;
//		IDeckLinkVideoFrame*    videoFrameBlue = NULL;
//		BMDDisplayModeSupport   supported;
//		HRESULT                 result;
//
//		Initialize();
//
//		// Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
//		result = GetDeckLinkIterator(&deckLinkIterator);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "A DeckLink iterator could not be created.  The DeckLink drivers may not be installed.\n");
//			goto bail;
//		}
//
//		// Obtain the first DeckLink device
//		result = deckLinkIterator->Next(&deckLink);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "Could not find DeckLink device - result = %08x\n", result);
//			goto bail;
//		}
//
//		// Obtain the output interface for the DeckLink device
//		result = deckLink->QueryInterface(IID_IDeckLinkOutput, (void**)&deckLinkOutput);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "Could not obtain the IDeckLinkInput interface - result = %08x\n", result);
//			goto bail;
//		}
//
//		// Create an instance of output callback
//		outputCallback = new OutputCallback(deckLinkOutput);
//		if (outputCallback == NULL)
//		{
//			fprintf(stderr, "Could not create output callback object\n");
//			goto bail;
//		}
//
//		// Set the callback object to the DeckLink device's output interface
//		result = deckLinkOutput->SetScheduledFrameCompletionCallback(outputCallback);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "Could not set callback - result = %08x\n", result);
//			goto bail;
//		}
//
//		// Check the selected display mode/pixelformat can be output 
//		result = deckLinkOutput->DoesSupportVideoMode(kDisplayMode, kPixelFormat, kOutputFlags, &supported, &gDisplayMode);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "Could not determine whether video mode is supported - result = %08x\n", result);
//			goto bail;
//		}
//
//		if (supported == bmdDisplayModeNotSupported)
//		{
//			fprintf(stderr, "Video mode is not supported\n");
//			goto bail;
//		}
//
//		// Store the frame duration and timesale for later use
//		gDisplayMode->GetFrameRate(&gFrameDuration, &gTimeScale);
//
//		// Enable video output
//		result = deckLinkOutput->EnableVideoOutput(kDisplayMode, kOutputFlags);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "Could not enable video output - result = %08x\n", result);
//			goto bail;
//		}
//
//		// Create a frame with defined format
//		videoFrameBlue = CreateFrame(deckLinkOutput);
//
//		// Schedule a blue frame 3 times
//		for (int i = 0; i < 3; i++)
//		{
//			result = outputCallback->scheduleNextFrame(videoFrameBlue);
//			if (result != S_OK)
//			{
//				fprintf(stderr, "Could not schedule video frame - result = %08x\n", result);
//				goto bail;
//			}
//		}
//
//		// Start
//		result = deckLinkOutput->StartScheduledPlayback(0, gTimeScale, 1.0);
//		if (result != S_OK)
//		{
//			fprintf(stderr, "Could not start - result = %08x\n", result);
//			goto bail;
//		}
//
//		// Wait until user presses Enter
//		printf("Monitoring... Press <RETURN> to exit\n");
//
//		getchar();
//
//		printf("Exiting.\n");
//
//		// Stop capture
//		result = deckLinkOutput->StopScheduledPlayback(0, NULL, 0);
//
//		// Disable the video input interface
//		result = deckLinkOutput->DisableVideoOutput();
//
//		// Release resources
//	bail:
//
//		// Release the video input interface
//		if (deckLinkOutput != NULL)
//			deckLinkOutput->Release();
//
//		// Release the Decklink object
//		if (deckLink != NULL)
//			deckLink->Release();
//
//		// Release the DeckLink iterator
//		if (deckLinkIterator != NULL)
//			deckLinkIterator->Release();
//
//		// Release the videoframe object
//		if (videoFrameBlue != NULL)
//			videoFrameBlue->Release();
//
//		// Release the outputCallback callback object
//		if (outputCallback)
//			delete outputCallback;
//
//		return (result == S_OK) ? 0 : 1;
//	}
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DeckLinkIODevice.h"

int CDeckLinkOutputDevice::output_reference_counter = 0;
void* CDeckLinkOutputDevice::outputDeckLinkIterator = NULL;

const BMDVideoOutputFlags kOutputFlags = bmdVideoOutputRP188;
const BMDDisplayMode      kDisplayMode = bmdModeHD1080i50;
const BMDPixelFormat      kPixelFormat = bmdFormat10BitYUV;
const /*INT32_UNSIGNED*/unsigned int kRowBytes = 5120;
// Timecode options
const bool kIsDropFrame = false;


	class OutputCallback : public IDeckLinkVideoOutputCallback
	{
	public:
		OutputCallback(IDeckLinkOutput* deckLinkOutput)
		{
			m_deckLinkOutput = deckLinkOutput;
			m_deckLinkOutput->AddRef();
			mutableVideoFrame = NULL;
		}

		void SetContext(CDeckLinkOutputDevice* ctx) {
			m_ctx = ctx;
		}

		virtual ~OutputCallback(void)
		{
			m_deckLinkOutput->Release();
			if (mutableVideoFrame)
				mutableVideoFrame->Release();
		}

		virtual HRESULT	STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult completionResult)
		{

			HRESULT                         result;
			result = ((IDeckLinkOutput*)m_ctx->deckLinkOutput)->CreateVideoFrame((int)((IDeckLinkDisplayMode*)m_ctx->deckLinkDisplayMode)->GetWidth(),\
				(int)((IDeckLinkDisplayMode*)m_ctx->deckLinkDisplayMode)->GetHeight(),kRowBytes, kPixelFormat, bmdFrameFlagDefault, &mutableVideoFrame);
			if (result != S_OK)
			{
				fprintf(stderr, "Could not create a video frame - result = %08x\n", result);
				return E_FAIL;
			}

			INT32_UNSIGNED* nextWord;
			INT32_UNSIGNED  wordsRemaining;
			INT32_UNSIGNED  height = (INT32_UNSIGNED)((IDeckLinkDisplayMode*)m_ctx->deckLinkDisplayMode)->GetHeight();

			mutableVideoFrame->GetBytes((void**)&nextWord);
			wordsRemaining = (kRowBytes * height) / 4;

			if (m_ctx->m_cb) {
				int ret = m_ctx->m_cb((char*)nextWord, wordsRemaining, m_ctx->m_cb_ctx);
				if (ret != wordsRemaining) {
					fprintf(stderr, "read data failed ret:%ld,wordsRemaining:%ld - result = %08x\n", ret, wordsRemaining);
					return E_FAIL;
				}
			}

			return scheduleNextFrame(mutableVideoFrame);
			//return scheduleNextFrame(completedFrame);
		}

		virtual HRESULT	STDMETHODCALLTYPE ScheduledPlaybackHasStopped(void)
		{
			return S_OK;
		}

		// IUnknown needs only a dummy implementation
		virtual HRESULT	STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv)
		{
			return E_NOINTERFACE;
		}

		virtual ULONG STDMETHODCALLTYPE AddRef()
		{
			return 1;
		}

		virtual ULONG STDMETHODCALLTYPE Release()
		{
			return 1;
		}

		virtual HRESULT scheduleNextFrame(IDeckLinkVideoFrame* videoFrame)
		{
			HRESULT                     result;
			IDeckLinkMutableVideoFrame* mutableFrame = NULL;
			INT8_UNSIGNED               hours;
			INT8_UNSIGNED               minutes;
			INT8_UNSIGNED               seconds;
			INT8_UNSIGNED               frames;

			static INT32_UNSIGNED totalFramesScheduled = 0;

			m_ctx->convertFrameCountToTimecode(totalFramesScheduled, &hours, &minutes, &seconds, &frames);

			result = videoFrame->QueryInterface(IID_IDeckLinkMutableVideoFrame, (void**)&mutableFrame);
			if (result != S_OK)
				goto bail;

			result = m_ctx->setRP188VitcTimecodeOnFrame(mutableFrame, hours, minutes, seconds, frames);
			if (result != S_OK)
				goto bail;

			// When a video frame completes,reschedule another frame
			result = m_deckLinkOutput->ScheduleVideoFrame(videoFrame, totalFramesScheduled * m_ctx->frameDuration, m_ctx->frameDuration, m_ctx->timeScale);
			if (result != S_OK)
				goto bail;

			totalFramesScheduled++;

		bail:
			if (mutableFrame)
				mutableFrame->Release();

			return result;
		}

	private:
		IDeckLinkOutput*	   m_deckLinkOutput;
		CDeckLinkOutputDevice* m_ctx;

		IDeckLinkMutableVideoFrame*     mutableVideoFrame;
	};

	CDeckLinkOutputDevice::CDeckLinkOutputDevice() :deckLinkAttributes(NULL), deckLink(NULL), deckLinkOutput(NULL), outputCallback(NULL){

	}

	bool CDeckLinkOutputDevice::CreateObjects(int k) {

		//IDeckLinkIterator*      deckLinkIterator = NULL;
		//IDeckLink*              deckLink = NULL;
		//IDeckLinkOutput*        deckLinkOutput = NULL;
		//OutputCallback*         outputCallback = NULL;
		//IDeckLinkVideoFrame*    videoFrameBlue = NULL;
		BMDDisplayModeSupport	  supported;
		HRESULT					  result;

		Initialize();

		// Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
		if (NULL == outputDeckLinkIterator) {
			if (GetDeckLinkIterator((IDeckLinkIterator**)(&outputDeckLinkIterator)) != S_OK)
			{
				fprintf(stderr, "A DeckLink iterator could not be created.  The DeckLink drivers may not be installed.\n");
				goto bail;
			}
			output_reference_counter++;
		}
		else {
			output_reference_counter++;
		}

		// Obtain every DeckLink device
		for (int i = 0; i < MAX_DEVICES_COUNT; ++i) {
			// Obtain the first DeckLink device
			result = (((IDeckLinkIterator*)outputDeckLinkIterator)->Next((IDeckLink**)(&deckLink)));

			if (result != S_OK) {
				fprintf(stderr, "Could not find DeckLink device - result = %08x\n", result);
				goto bail;
			}
			else {
				if (i == k) {
					break;
				}
				else {
					((IDeckLink*)deckLink)->Release();
				}
			}
		}




		// Obtain the output interface for the DeckLink device
		result = ((IDeckLink*)deckLink)->QueryInterface(IID_IDeckLinkOutput, (void**)&deckLinkOutput);
		if (result != S_OK)
		{
			fprintf(stderr, "Could not obtain the IDeckLinkInput interface - result = %08x\n", result);
			goto bail;
		}

		// Create an instance of output callback
		outputCallback = new OutputCallback((IDeckLinkOutput*)deckLinkOutput);
		if (outputCallback == NULL)
		{
			fprintf(stderr, "Could not create output callback object\n");
			goto bail;
		}

		// Set the callback object to the DeckLink device's output interface
		result = ((IDeckLinkOutput*)deckLinkOutput)->SetScheduledFrameCompletionCallback((OutputCallback*)outputCallback);
		if (result != S_OK)
		{
			fprintf(stderr, "Could not set callback - result = %08x\n", result);
			goto bail;
		}

		// Check the selected display mode/pixelformat can be output 
		result = ((IDeckLinkOutput*)deckLinkOutput)->DoesSupportVideoMode(kDisplayMode, kPixelFormat, kOutputFlags, &supported, (IDeckLinkDisplayMode**)&deckLinkDisplayMode);
		if (result != S_OK)
		{
			fprintf(stderr, "Could not determine whether video mode is supported - result = %08x\n", result);
			goto bail;
		}

		if (supported == bmdDisplayModeNotSupported)
		{
			fprintf(stderr, "Video mode is not supported\n");
			goto bail;
		}
		return true;

	bail:
		DestroyObjects();
		return false;
	}
	void CDeckLinkOutputDevice::DestroyObjects() {
		// Release the video input interface
		if (deckLinkOutput != NULL)
			((IDeckLinkOutput*)deckLinkOutput)->Release();

		// Release the Decklink object
		if (deckLink != NULL)
			((IDeckLink*)deckLink)->Release();

		// Release the DeckLink iterator
		if (outputDeckLinkIterator != NULL)
			((IDeckLinkIterator*)outputDeckLinkIterator)->Release();

		// Release the videoframe object
		if (videoFrameBlue != NULL)
			((IDeckLinkVideoFrame*)videoFrameBlue)->Release();

		// Release the outputCallback callback object
		if (outputCallback)
			delete outputCallback;
	}


	void CDeckLinkOutputDevice::setSourceCallback(SourceDataCallback cb, void* ctx) {
		m_cb = cb;
		m_cb_ctx = ctx;
	}

	bool CDeckLinkOutputDevice::start() {

		HRESULT					  result;

		// Store the frame duration and timesale for later use
		((IDeckLinkDisplayMode*)deckLinkDisplayMode)->GetFrameRate(&frameDuration, &timeScale);

		// Enable video output
		result = ((IDeckLinkOutput*)deckLinkOutput)->EnableVideoOutput(kDisplayMode, kOutputFlags);
		if (result != S_OK)
		{
			fprintf(stderr, "Could not enable video output - result = %08x\n", result);
			return false;
		}

		// Create a frame with defined format
		videoFrameBlue = (IDeckLinkVideoFrame*)CreateFrame(deckLinkOutput);

		// Schedule a blue frame 3 times
		for (int i = 0; i < 3; i++)
		{
			result = ((OutputCallback*)outputCallback)->scheduleNextFrame((IDeckLinkVideoFrame*)videoFrameBlue);
			if (result != S_OK)
			{
				fprintf(stderr, "Could not schedule video frame - result = %08x\n", result);
				return false;
			}
		}

		// Start
		result = ((IDeckLinkOutput*)deckLinkOutput)->StartScheduledPlayback(0, timeScale, 1.0);
		if (result != S_OK)
		{
			fprintf(stderr, "Could not start - result = %08x\n", result);
			return FALSE;
		}
		return TRUE;
	}

	bool CDeckLinkOutputDevice::stop() {
		HRESULT					  result;
		// Stop capture
		result = ((IDeckLinkOutput*)deckLinkOutput)->StopScheduledPlayback(0, NULL, 0);
		if (result != S_OK)
		{
			fprintf(stderr, "Could not StopScheduledPlayback - result = %08x\n", result);
			return false;
		}
		// Disable the video input interface
		result = ((IDeckLinkOutput*)deckLinkOutput)->DisableVideoOutput();
		if (result != S_OK)
		{
			fprintf(stderr, "Could not DisableVideoOutput - result = %08x\n", result);
			return false;
		}
		return true;
	}


	void* CDeckLinkOutputDevice::CreateFrame(/*IDeckLinkOutput*/void* deckLinkOutput)
	{
		HRESULT                         result;
		IDeckLinkMutableVideoFrame*     frame = NULL;

		result = ((IDeckLinkOutput*)deckLinkOutput)->CreateVideoFrame((int)((IDeckLinkDisplayMode*)deckLinkDisplayMode)->GetWidth(), (int)((IDeckLinkDisplayMode*)deckLinkDisplayMode)->GetHeight(), kRowBytes, kPixelFormat, bmdFrameFlagDefault, &frame);
		if (result != S_OK)
		{
			fprintf(stderr, "Could not create a video frame - result = %08x\n", result);
			goto bail;
		}

		FillBlue(frame);

	bail:
		return frame;
	}

	void CDeckLinkOutputDevice::FillBlue(/* IDeckLinkMutableVideoFrame* */void* theFrame)
	{
		INT32_UNSIGNED* nextWord;
		INT32_UNSIGNED  wordsRemaining;
		INT32_UNSIGNED  height = (INT32_UNSIGNED)((IDeckLinkDisplayMode*)deckLinkDisplayMode)->GetHeight();
		const			 INT32_UNSIGNED kBlueData[4] = { 0x40aa298, 0x2a8a62a8, 0x298aa040, 0x2a8102a8 };


		((IDeckLinkMutableVideoFrame*)theFrame)->GetBytes((void**)&nextWord);
		wordsRemaining = (kRowBytes * height) / 4;

		while (wordsRemaining > 0)
		{
			*(nextWord++) = kBlueData[0];
			*(nextWord++) = kBlueData[1];
			*(nextWord++) = kBlueData[2];
			*(nextWord++) = kBlueData[3];
			wordsRemaining = wordsRemaining - 4;
		}
	}

	void CDeckLinkOutputDevice::convertFrameCountToTimecode(INT32_UNSIGNED frameCount, INT8_UNSIGNED* hours, INT8_UNSIGNED* minutes, INT8_UNSIGNED* seconds, INT8_UNSIGNED* frames)
	{


		INT32_UNSIGNED maxFPS = (INT32_UNSIGNED)(timeScale / 1000);

		// Dropped frame counting only applies to M-rate modes (i.e. Fractional frame rates)
		if (kIsDropFrame && frameDuration == 1001)
		{
			INT32_UNSIGNED deciMins, deciMinsRemainder;

			INT32_UNSIGNED framesIn10mins = (INT32_UNSIGNED)((60 * 10 * maxFPS) - (9 * 2));
			deciMins = frameCount / framesIn10mins;
			deciMinsRemainder = frameCount - (deciMins * framesIn10mins);

			// Add 2 frames for 9 minutes of every 10 minutes that have elapsed
			// AND 2 frames for every minute (over the first minute) in this 10-minute block.
			frameCount += 2 * 9 * deciMins;
			if (deciMinsRemainder >= 2)
				frameCount += 2 * ((deciMinsRemainder - 2) / (framesIn10mins / 10));
		}

		*frames = (INT8_UNSIGNED)(frameCount % maxFPS);
		frameCount /= maxFPS;
		*seconds = (INT8_UNSIGNED)(frameCount % 60);
		frameCount /= 60;
		*minutes = (INT8_UNSIGNED)(frameCount % 60);
		frameCount /= 60;
		*hours = (INT8_UNSIGNED)frameCount;
	}


	long CDeckLinkOutputDevice::setRP188VitcTimecodeOnFrame(/*IDeckLinkMutableVideoFrame*/void* videoFrame, INT8_UNSIGNED hours, INT8_UNSIGNED minutes, INT8_UNSIGNED seconds, INT8_UNSIGNED frames)
	{
		HRESULT	result = S_OK;
		// Timecode options
		const bool kIsDropFrame = false;

		BMDTimecodeFlags flags = bmdTimecodeFlagDefault;
		bool setVITC1Timecode = false;
		bool setVITC2Timecode = false;

		if (kIsDropFrame)
			flags |= bmdTimecodeIsDropFrame;

		if (((IDeckLinkDisplayMode*)deckLinkDisplayMode)->GetFieldDominance() != bmdProgressiveFrame)
		{
			// An interlaced or PsF frame has both VITC1 and VITC2 set with the same timecode value (SMPTE ST 12-2:2014 7.2)
			setVITC1Timecode = true;
			setVITC2Timecode = true;
		}
		else if (timeScale / frameDuration <= 30)
		{
			// If this isn't a High-P mode, then just use VITC1 (SMPTE ST 12-2:2014 7.2)
			setVITC1Timecode = true;
		}
		else
		{
			// If this is a High-P mode then use VITC1 on even frames and VITC2 on odd frames. This is done because the 
			// frames field of the RP188 VITC timecode cannot hold values greater than 30 (SMPTE ST 12-2:2014 7.2, 9.2)
			if ((frames & 1) == 0)
				setVITC1Timecode = true;
			else
				setVITC2Timecode = true;

			// Shift the frame count so it's maximum is within the valid range
			frames >>= 1;
		}

		if (setVITC1Timecode)
		{
			result = ((IDeckLinkMutableVideoFrame*)videoFrame)->SetTimecodeFromComponents(bmdTimecodeRP188VITC1, hours, minutes, seconds, frames, flags);
			if (result != S_OK)
			{
				fprintf(stderr, "Could not set VITC1 timecode on interlaced frame - result = %08x\n", result);
				goto bail;
			}
		}

		if (setVITC2Timecode)
		{
			// The VITC2 timecode also has the field mark flag set
			result = ((IDeckLinkMutableVideoFrame*)videoFrame)->SetTimecodeFromComponents(bmdTimecodeRP188VITC2, hours, minutes, seconds, frames, flags | bmdTimecodeFieldMark);
			if (result != S_OK)
			{
				fprintf(stderr, "Could not set VITC1 timecode on interlaced frame - result = %08x\n", result);
				goto bail;
			}
		}

	bail:
		return result;
	}

int source_callback(char* buffer, int len, void* ctx) {
	FILE* fp = (FILE*)ctx;
	return fread(buffer, 1, len, fp);
}

int main0000(int argc, char** argv) {

	int index = 0;
	if (argc > 1)
		index = atoi(argv[1]);

	FILE* fp = fopen("1.yuv", "rb");
	CDeckLinkOutputDevice d_output;
	d_output.CreateObjects(index);
	d_output.setSourceCallback(source_callback, fp);

	d_output.start();

	getchar();

	d_output.stop();
	d_output.DestroyObjects();

	return 0;
}