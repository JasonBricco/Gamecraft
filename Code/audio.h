//
// Jason Bricco
//

struct SoundCallback;

struct AudioEngine : public IXAudio2VoiceCallback
{
	IXAudio2* pXAudio;
	IXAudio2MasteringVoice* masteringVoice;
	IXAudio2SourceVoice* musicSource;
	
	int16_t* musicSamples;

	// Buffer size is the total size of the buffer in bytes.
	// Sample count is the total size of the buffer in samples.
	// A sample includes both channels in stereo sound.
	int bufferSize, sampleCount;
	int write;

	// A handle to the music file that is being streamed from.
	stb_vorbis* musicPtr;

	// Stores free voices for sound effects.
	queue<IXAudio2SourceVoice*> voicePool;

	// Stores sound callbacks so they can be freed if needed.
	vector<SoundCallback*> callbacks;

	LerpData volumeLerp;

	void OnVoiceProcessingPassStart(UINT32 bytesRequired);
	void OnVoiceProcessingPassEnd() {}
	void OnStreamEnd() {}
	void OnBufferStart(void*) {}
	void OnBufferEnd(void*) {}
	void OnLoopEnd(void*) {}
	void OnVoiceError(void*, HRESULT) {}
};

struct SoundCallback : public IXAudio2VoiceCallback
{
	AudioEngine* engine;
	IXAudio2SourceVoice* source;

	void OnStreamEnd();
	void OnVoiceProcessingPassStart(UINT32) {}
	void OnVoiceProcessingPassEnd() {}
	void OnBufferStart(void*) {}
	void OnBufferEnd(void*) {}
	void OnLoopEnd(void*) {}
	void OnVoiceError(void*, HRESULT) {}
};
