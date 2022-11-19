#pragma once

extern "C"
{
#include "ffmpeg-2.0/libavcodec/avcodec.h"
#include "ffmpeg-2.0/libavdevice/avdevice.h"
#include "ffmpeg-2.0/libavfilter/avfilter.h"
#include "ffmpeg-2.0/libavformat/avformat.h"
#include "ffmpeg-2.0/libavformat/avio.h"
#include "ffmpeg-2.0/libavutil/avutil.h"
#include "ffmpeg-2.0/libpostproc/postprocess.h"
#include "ffmpeg-2.0/libswresample/swresample.h"
#include "ffmpeg-2.0/libswscale/swscale.h"
#include "ffmpeg-2.0/libavutil/frame.h"
#include <wtypes.h>
}

enum EVideoError {
	EVideoError_NoError = 0,
	EVideoError_WrongFormat,
	EVideoError_FileNotFound,
};

namespace Engine {

	// class CWaveOut;

	// struct WavData {
	// 	short channels;
	// 	int sampleRate;
	// 	int sampleSize; // in bytes
	// 	unsigned char* data; // packed PCM sample data
	// 	int mySize = 0;
	// };

	class CVideoPlayer {
	public:
		CVideoPlayer();
		~CVideoPlayer();

		EVideoError Init(const char* aPath, bool aPlayAudio);

		bool DoFirstFrame();
		void Stop();
		void RestartStream(float aTimeStamp = 0.0f);
		void SeekToSeconds(const float aSeconds);

		double GetFps();
		bool ConvertVideoToArray(unsigned int*& aBuffer, unsigned int aSizeX = 0, unsigned int aSizeY = 0) const;

		int GrabNextFrame();
		inline AVFrame* GetAvVideoFrame() const { return myAVVideoFrame; }

	private:
		AVCodec* myVideoCodec;

		AVCodecContext* myVideoCodecContext;
		SwsContext* mySwsVideoContext;
		AVFormatContext* myVideoFormatContext;

		AVFrame* myAVVideoFrame;
		AVFrame* myAVVideoFrameBGR;
		AVPacket            myAVPacket;
		std::string         myFileName;
		uint8_t* myUIBuffer;
		int                 myGotFrame;
		int                 myDecodedBytes;
		int                 myVideoStreamIndex;
		AVStream* myVideoStream = nullptr;

		int                 myNumberOfBytes;

		bool myAudioInitialized = false;
	};
}
