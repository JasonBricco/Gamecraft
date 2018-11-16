//
// Jason Bricco
//

struct AudioEngine : public IXAudio2VoiceCallback
{
	IXAudio2* pXAudio;
	IXAudio2MasteringVoice* masteringVoice;
	IXAudio2SourceVoice* sourceVoice;
	int16_t* samples;
	int bufferSize;
	int write;
	float tSin;

	void OnVoiceProcessingPassStart(UINT32 bytesRequired);
	void OnVoiceProcessingPassEnd() {}
	void OnStreamEnd() {}
	void OnBufferStart(void*) {}
	void OnBufferEnd(void*) {}
	void OnLoopEnd(void*) {}
	void OnVoiceError(void*, HRESULT) {}
};
