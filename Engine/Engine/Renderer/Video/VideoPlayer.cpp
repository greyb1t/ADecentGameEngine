#include "pch.h"
#include "VideoPlayer.h"
#include "VideoAudio.h"
#include "libavutil\avutil.h"

#pragma comment(lib, "winmm.lib")

Engine::CVideoPlayer::CVideoPlayer()
{
	myVideoCodec = nullptr;
	myVideoCodecContext = nullptr;
	mySwsVideoContext = nullptr;
	myVideoFormatContext = nullptr;
	myAVVideoFrame = nullptr;
	myAVVideoFrameBGR = nullptr;
	myUIBuffer = nullptr;
}

Engine::CVideoPlayer::~CVideoPlayer()
{

	// Free the RGB image
	if (myAVVideoFrameBGR)
	{
		av_frame_free(&myAVVideoFrameBGR);
	}


	// Free the YUV frame
	if (myAVVideoFrame)
	{
		av_frame_free(&myAVVideoFrame);
	}


	// Close the codecs
	if (myVideoCodecContext)
	{
		avcodec_close(myVideoCodecContext);
	}


	// Close the video file
	if (myVideoFormatContext)
	{
		avformat_close_input(&myVideoFormatContext);
	}

	if (myUIBuffer)
	{
		av_free(myUIBuffer);
	}


	if (mySwsVideoContext)
	{
		sws_freeContext(mySwsVideoContext);
	}
}

bool Engine::CVideoPlayer::DoFirstFrame()
{
	GrabNextFrame();
	return true;
}



void Engine::CVideoPlayer::Stop()
{
	av_seek_frame(myVideoFormatContext, myVideoStreamIndex, 0, AVSEEK_FLAG_ANY);
}

double r2d(AVRational r)
{
	return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

#define CALC_FFMPEG_VERSION(a,b,c) ( a<<16 | b<<8 | c )
double get_fps(AVFormatContext* aContext, int aStream)
{
	double eps_zero = 0.000025;
	double fps = r2d(aContext->streams[aStream]->r_frame_rate);

#if LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(52, 111, 0)
	if (fps < eps_zero)
	{
		fps = r2d(aContext->streams[aStream]->avg_frame_rate);
	}
#endif

	if (fps < eps_zero)
	{
		fps = 1.0 / r2d(aContext->streams[aStream]->codec->time_base);
	}

	return fps;
}



void Engine::CVideoPlayer::RestartStream(float aTimeStamp)
{
	int64_t timestamp = (int64_t)aTimeStamp;
	av_seek_frame(myVideoFormatContext, myVideoStreamIndex, aTimeStamp, AVSEEK_FLAG_BACKWARD);
}

void Engine::CVideoPlayer::SeekToSeconds(const float aSeconds)
{
	// auto lol = FrameToPts(myVideoStream, aFrame);

	double timeBase = (double)myVideoStream->time_base.num / (double)myVideoStream->time_base.den;
	double durationSeconds = (double)myVideoStream->duration * timeBase;

	// myVideoFormatContext->duration / 2;

	double dur = static_cast<float>(myVideoStream->duration) * (aSeconds / durationSeconds);

	const float time = 0.5f;

	// int64_t durationInFfmpegFormat = static_cast<double>(aSeconds) / (timeBase * 1000.f);

	// av_seek_frame(myVideoFormatContext, -1, 100.15, 0);

	// aFrame* timeBase, 
	//static_cast<int64_t>(static_cast<float>(myVideoFormatContext->duration / AV_TIME_BASE) * time),

	// int64_t seek_target = myVideoFormatContext->duration * 0.95f/*dur*/;
	// seek_target = av_rescale_q(seek_target, { 1, AV_TIME_BASE }, myVideoStream->time_base);

	// works
	auto half = myVideoStream->duration / 2;

	int result = av_seek_frame(
		myVideoFormatContext,
		myVideoStreamIndex,
		static_cast<int64_t>(dur),
		AVSEEK_FLAG_BACKWARD);

	if (result < 0)
	{
		assert(false);
	}

	// int result = av_seek_frame(
	// 	myVideoFormatContext,
	// 	myVideoStreamIndex,
	// 	myVideoFormatContext->duration / 2,
	// 	/*AVSEEK_FLAG_ANY*/0);

	// av_seek_frame(fmt_ctx, -1, 2, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);

	//result = av_seek_frame(
	//	myVideoFormatContext,
	//	myVideoStreamIndex,
	//	,
	//	AVSEEK_FLAG_FRAME | AVSEEK_FLAG_ANY);

	//int result = av_seek_frame(
	//	myVideoFormatContext, 
	//	myVideoStreamIndex, 
	//	/*aFrame * timeBase*/static_cast<int64_t>(static_cast<float>(myVideoFormatContext->duration / AV_TIME_BASE) * time),
	//	/*AVSEEK_FLAG_FRAME |*/ AVSEEK_FLAG_ANY);
	//
	int test = 0;
}

double Engine::CVideoPlayer::GetFps()
{
	if (!myVideoFormatContext)
	{
		return 0;
	}
	return get_fps(myVideoFormatContext, myVideoStreamIndex);
}

EVideoError Engine::CVideoPlayer::Init(const char* aPath, bool aPlayAudio)
{
	if (!std::filesystem::exists(aPath))
	{
		return EVideoError_FileNotFound;
	}

	myFileName = std::string(aPath);

	av_register_all();

	int returnResul = avformat_open_input(&myVideoFormatContext,
		myFileName.c_str(),
		NULL,
		NULL);



	if (returnResul >= 0)
	{
		returnResul = avformat_find_stream_info(myVideoFormatContext, NULL);

		if (returnResul >= 0)
		{
			for (unsigned int i = 0; i < myVideoFormatContext->nb_streams; i++)
			{
				if (myVideoFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
				{
					myVideoStreamIndex = i;
					myVideoStream = myVideoFormatContext->streams[myVideoStreamIndex];

					myVideoCodecContext = myVideoFormatContext->streams[myVideoStreamIndex]->codec;
					if (myVideoCodecContext)
					{
						myVideoCodec = avcodec_find_decoder(myVideoCodecContext->codec_id);
					}

				}
			}
		}
	}
	else
	{
		return EVideoError_WrongFormat;
	}

	if (myVideoCodec && myVideoCodecContext)
	{
		returnResul = avcodec_open2(myVideoCodecContext, myVideoCodec, NULL);

		if (returnResul >= 0)
		{
			myAVVideoFrame = av_frame_alloc();
			myAVVideoFrameBGR = av_frame_alloc();

			AVPixelFormat format = AV_PIX_FMT_RGBA;

			myNumberOfBytes = avpicture_get_size(format,
				myVideoCodecContext->width,
				myVideoCodecContext->height);

			myUIBuffer = (uint8_t*)av_malloc(myNumberOfBytes * sizeof(uint8_t));



			avpicture_fill((AVPicture*)myAVVideoFrameBGR,
				myUIBuffer,
				format,
				myVideoCodecContext->width,
				myVideoCodecContext->height);



			mySwsVideoContext = sws_getContext(
				myVideoCodecContext->width,
				myVideoCodecContext->height,
				myVideoCodecContext->pix_fmt,
				myVideoCodecContext->width,
				myVideoCodecContext->height,
				format,
				SWS_BILINEAR,
				NULL,
				NULL,
				NULL);

		}
	}
	else
	{
		return EVideoError_WrongFormat;
	}

	return EVideoError_NoError;
}

int Engine::CVideoPlayer::GrabNextFrame()
{
	ZoneScopedN("CVideoPlayer::GrabNextFrame");

	bool valid = false;
	int readFrame = 0;
	int safeValue = 0;
	const int maxSafeVal = 10000;

	while (!valid || safeValue > maxSafeVal)
	{
		safeValue++;
		readFrame = av_read_frame(myVideoFormatContext, &myAVPacket);
		if (readFrame >= 0)
		{
			if (myAVPacket.stream_index == myVideoStreamIndex)
			{
				myDecodedBytes = avcodec_decode_video2(
					myVideoCodecContext,
					myAVVideoFrame,
					&myGotFrame,
					&myAVPacket);

				if (myGotFrame)
				{
					valid = true;
				}
			}
		}
		else
		{
			valid = true;
		}
		av_free_packet(&myAVPacket);
	}

	if (safeValue >= maxSafeVal)
	{
		LOG_ERROR(LogType::Video) << "Video error: Could not find a valid video frame!";
	}

	return readFrame;
}

bool Engine::CVideoPlayer::ConvertVideoToArray(
	unsigned int*& aBuffer,
	unsigned int aSizeX,
	unsigned int aSizeY) const
{
	ZoneNamedN(zone1, "CVideoPlayer::ConvertVideoToArray", true);

	if (!myGotFrame)
	{
		return false;
	}
	if (mySwsVideoContext)
	{
		int returnResult;

		{
			ZoneNamedN(zone2, "CVideoPlayer::ConvertVideoToArray-(sws_scale)", true);

			returnResult = sws_scale(
				mySwsVideoContext,
				myAVVideoFrame->data,
				myAVVideoFrame->linesize,
				0,
				myVideoCodecContext->height,
				myAVVideoFrameBGR->data,
				myAVVideoFrameBGR->linesize);
		}

		if (returnResult > 0)
		{
			uint8_t* data = myAVVideoFrameBGR->data[0];

			/*
			for (int i = 0; i < myVideoCodecContext->height; i++)
			{
				for (int j = 0; j < myVideoCodecContext->width; j++)
				{
					const int idx = i * aSizeX + j;

					// Index2 is just because aBuffer and data is different texture sizes
					const int idx2 = i * myVideoCodecContext->width + j;

					aBuffer[idx] = reinterpret_cast<unsigned int*>(data)[idx2];
				}
			}
			*/

			{
				ZoneNamedN(zone3, "CVideoPlayer::ConvertVideoToArray-(memcpy)", true);

				const int rgabBytesSize = 4;
				memcpy(
					&aBuffer[0],
					&reinterpret_cast<unsigned int*>(data)[0],
					myVideoCodecContext->height * myVideoCodecContext->width * rgabBytesSize);
			}
		}

	}

	return true;
}
