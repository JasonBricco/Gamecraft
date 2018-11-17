//
// Jason Bricco
//

struct AudioEngine : public IXAudio2VoiceCallback
{
	IXAudio2* pXAudio;
	IXAudio2MasteringVoice* masteringVoice;
	IXAudio2SourceVoice* sourceVoice;
	
	int16_t* samples;

	// Buffer size is the total size of the buffer in bytes.
	// Sample count is the total size of the buffer in samples.
	// A sample includes both channels in stereo sound.
	int bufferSize, sampleCount;
	int write;

	// A handle to the music file that is being streamed from.
	stb_vorbis* musicPtr;

	void OnVoiceProcessingPassStart(UINT32 bytesRequired);
	void OnVoiceProcessingPassEnd() {}
	void OnStreamEnd() {}
	void OnBufferStart(void*) {}
	void OnBufferEnd(void*) {}
	void OnLoopEnd(void*) {}
	void OnVoiceError(void*, HRESULT) {}
};
