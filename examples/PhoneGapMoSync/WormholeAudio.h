#ifndef _WORMHOLE_AUDIO_H_
#define _WORMHOLE_AUDIO_H_

#include <Wormhole/WebViewMessage.h>
#include <NativeUI/WebView.h>
#include "PhoneGapMessage.h"
#include "PhoneGapMessageHandler.h"

class PhoneGapMessageHandler;

class WormholeAudio
{
public:

	WormholeAudio(PhoneGapMessageHandler* messageHandler);
	virtual ~WormholeAudio();

	void handleMessage(PhoneGapMessage& message);

	//Called from PhoneGapMessageHandler on audio events
	void handleAudioEvents(const MAEvent& event);

private:

	void init(MAUtil::String URL);

	void play();
	void stop();
	void pause();
	void release();

	void getPosition();
	void setPosition(int pos);

	void getDuration();

	void signalJS(MAUtil::String callback, char* state, char* successStr);

	PhoneGapMessageHandler* mMessageHandler;

	MAUtil::String mPlayCallbackID;
	MAUtil::String mStopCallbackID;
	MAUtil::String mPauseCallbackID;
	MAUtil::String mReleaseCallbackID;
	MAUtil::String mGetPositionCallbackID;
	MAUtil::String mSetPositionCallbackID;
	MAUtil::String mGetDurationCallbackID;

	MAUtil::String mURL;

	MAAudioData mAudioData;
	MAAudioInstance mAudioInstance;
	bool mIsActive;
	bool mIsPrepared;
	bool mPlayWhenPrepared;
};

#endif // _WORMHOLE_AUDIO_H_
