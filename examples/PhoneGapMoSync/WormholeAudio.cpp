#include "WormholeAudio.h"

#include <mastdlib.h>

#define MEDIA_NONE 		"0"
#define MEDIA_STARTING 	"1"
#define MEDIA_RUNNING	"2"
#define MEDIA_PAUSED	"3"
#define MEDIA_STOPPED	"4"

#define MEDIA_STATE		"1"
#define MEDIA_DURATION	"2"
#define MEDIA_POSITION	"3"
#define MEDIA_ERROR 	"9"

#define MEDIA_ERR_NONE_ACTIVE		"0"
#define MEDIA_ERR_ABORTED			"1"
#define MEDIA_ERR_NETWORK			"2"
#define MEDIA_ERR_DECODE			"3"
#define MEDIA_ERR_NONE_SUPPORTED	"4"

WormholeAudio::WormholeAudio(PhoneGapMessageHandler* messageHandler) :
	mMessageHandler(messageHandler),
	mIsActive(false),
	mIsPrepared(false),
	mPlayWhenPrepared(false)
{
	mURL = "";
	mGetDurationCallbackID = "";
}

WormholeAudio::~WormholeAudio()
{
	if(mIsActive)
		release();
}

void WormholeAudio::handleMessage(PhoneGapMessage& message)
{
	printf("Message: %s\n" , message.getParam("action").c_str());

	printf("id: %s\n" , message.getArgsField("id").c_str());

	if((message.getParam("action") == "startPlayingAudio"))
	{
		printf("play play play!");

		mPlayCallbackID = message.getArgsField("id");

		MAUtil::String URL = message.getArgsField("src");

		printf("old URL:%s\n new URL:%s\n", mURL.c_str(), URL.c_str());
		//printf("URL: %s\n", URL.c_str());

		if(mURL != URL)
		{
			init(URL);
			mURL = URL;
		}
		play();
	}
	else if((message.getParam("action") == "stopPlayingAudio"))
	{
		mStopCallbackID = message.getArgsField("id");

		stop();
	}
	else if((message.getParam("action") == "pausePlayingAudio"))
	{
		mPauseCallbackID = message.getArgsField("id");
		pause();
	}
	else if((message.getParam("action") == "release"))
	{
		mReleaseCallbackID = message.getArgsField("id");
		release();
	}
	else if((message.getParam("action") == "seekToAudio"))
	{
		mSetPositionCallbackID = message.getArgsField("id");

		MAUtil::String ms = message.getArgsField("milliseconds");
		int pos = atoi(ms.c_str());

		setPosition(pos);
	}

	else if((message.getParam("action") == "getCurrentPositionAudio"))
	{
		mGetPositionCallbackID = message.getArgsField("id");
		getPosition();
	}

}

void WormholeAudio::init(MAUtil::String URL)
{
	printf("init!");

	if(mIsActive)
		release();

	// Create the audio data object
	mAudioData = maAudioDataCreateFromURL(NULL, URL.c_str(), MA_AUDIO_DATA_STREAM);
	if(mAudioData<0)
	{
		signalJS(mPlayCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
		return;
	}

	printf("Audio Data Created!");

	// create the audio instance
	mAudioInstance = maAudioInstanceCreate(mAudioData);
	if(mAudioInstance<0)
	{
		signalJS(mPlayCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
		return;
	}

	printf("Audio Instance Created!");

	mIsActive = true;
	mIsPrepared = false;

	signalJS(mPlayCallbackID, MEDIA_STATE, MEDIA_STARTING);

	// Prepare this asynchronous
	maAudioPrepare(mAudioInstance, 1);
}

void WormholeAudio::play()
{
	printf("play");

	if(!mIsActive) return;
	if(!mIsPrepared)
	{
		mPlayWhenPrepared = true;
		return;
	}

	printf("go go!");

	if(maAudioPlay(mAudioInstance) < 0)
	{
		signalJS(mPlayCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
		return;
	}
	getDuration();
	signalJS(mPlayCallbackID, MEDIA_STATE, MEDIA_RUNNING);
}

void WormholeAudio::stop()
{
	maAudioStop(mAudioInstance);
	signalJS(mStopCallbackID, MEDIA_STATE, MEDIA_STOPPED);
}

void WormholeAudio::pause()
{
	maAudioPause(mAudioInstance);
	signalJS(mPauseCallbackID, MEDIA_STATE, MEDIA_PAUSED);
}

void WormholeAudio::release()
{
	// stop the sound if it's playing
	stop();

	// release all the resources
	maAudioInstanceDestroy(mAudioInstance);
	maAudioDataDestroy(mAudioData);

	mIsActive = false;
}

void WormholeAudio::getPosition()
{
	int len = maAudioGetPosition(mAudioInstance);

	if(len<=0)
	{
		signalJS(mGetPositionCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
		return;
	}

	float flen = len / 1000.0f;
	char temp[10];
	sprintf(temp, "%d", flen);

	signalJS(mGetPositionCallbackID, MEDIA_POSITION, temp);
}

void WormholeAudio::setPosition(int pos)
{
	if(maAudioSetPosition(mAudioInstance, pos) <= 0)
	{
		signalJS(mSetPositionCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
		return;
	}

	float fpos = pos/1000.0f;
	char temp[10];
	sprintf(temp, "%f", fpos);

	signalJS(mSetPositionCallbackID, MEDIA_POSITION, temp);
}

void WormholeAudio::getDuration()
{
	int duration = maAudioGetLength(mAudioInstance);
	if(duration<=0)
	{
		if(mGetDurationCallbackID != "")
			signalJS(mGetDurationCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
		else
			signalJS(mPlayCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
		return;
	}

	float fduration = duration/1000.0f;

	char temp[10];
	sprintf(temp, "%f", fduration);
	if(mGetDurationCallbackID != "")
		signalJS(mGetDurationCallbackID, MEDIA_DURATION, temp);
	else
		signalJS(mPlayCallbackID, MEDIA_ERROR, MEDIA_ERR_ABORTED);
}

void WormholeAudio::handleAudioEvents(const MAEvent& event)
{
	//mPlayWhenPrepared = false;
	printf("Got audio event back!\n");

	if(event.type == EVENT_TYPE_AUDIO_PREPARED)
	{
		if(mAudioInstance == event.audioInstance)
		{
			if(mPlayWhenPrepared)
			{
				printf("Play when prepared");

				maAudioPlay(mAudioInstance);
				mPlayWhenPrepared = false;

				getDuration();
				signalJS(mPlayCallbackID, MEDIA_STATE, MEDIA_RUNNING);
			}
		}
		else
		{
			signalJS(mPlayCallbackID, MEDIA_STATE, MEDIA_ERR_ABORTED);
		}
	}
	else if(event.type == EVENT_TYPE_AUDIO_COMPLETED)
	{
		signalJS(mPlayCallbackID, MEDIA_STATE, MEDIA_STOPPED);
	}

}

void WormholeAudio::signalJS(MAUtil::String callback, char* status, char* message)
{
	char temp[100];
	sprintf(temp, "PhoneGap.Media.onStatus('%s', %s, %s)", callback.c_str(), status, message);
	printf("%s\n", temp);
	mMessageHandler->callJS(temp);
}
