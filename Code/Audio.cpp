//
// Gamecraft
//

#define CheckForError(hr, ...) if (FAILED(hr)) Error(__VA_ARGS__)

static void InitAudio(AudioEngine* engine)
{
	HRESULT hr;

	IXAudio2* pXAudio = NULL;
	hr = XAudio2Create(&pXAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
	CheckForError(hr, "Failed to create IXAudio2.\n");

	// The mastering voice sends final audio data to the hardware.
	IXAudio2MasteringVoice* masteringVoice = NULL;

	hr = pXAudio->CreateMasteringVoice(&masteringVoice);
	CheckForError(hr, "Failed to create the mastering voice.\n");

    engine->pXAudio = pXAudio;
    engine->masteringVoice = masteringVoice;
    engine->maxMusicVolume = 0.75f;
}

static WAVEFORMATEX GetFormat(int sampleRate)
{
	assert(sampleRate == 44100);
	WAVEFORMATEX format = {};
	format.wFormatTag = WAVE_FORMAT_PCM;	
	format.nChannels = 2;
	format.nSamplesPerSec = sampleRate;
	format.wBitsPerSample = 16;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	return format;
}

static void ChangeVolume(AudioEngine* engine, float target, float seconds)
{
	IXAudio2SourceVoice* music = engine->musicSource;

	float volume;
	music->GetVolume(&volume);

	engine->volumeLerp = { volume, target, 0.0f, seconds };
}

static void LoadMusic(AudioEngine* engine, char* path)
{
	char buffer[MAX_PATH];
	path = PathToExe(path, buffer, MAX_PATH);

	int error;
	stb_vorbis* ptr = stb_vorbis_open_filename(path, &error, nullptr);

	if (ptr == nullptr) 
		Error("Couldn't open music at %s. Error: %i\n", path, error);

	stb_vorbis_info info = stb_vorbis_get_info(ptr);
	int sampleRate = info.sample_rate;

	WAVEFORMATEX format = GetFormat(sampleRate);

	IXAudio2SourceVoice* source = NULL;
	HRESULT hr = engine->pXAudio->CreateSourceVoice(&source, &format, 0, 2.0f, engine);
	CheckForError(hr, "Failed to create the source voice.\n");

	source->SetVolume(0.0f);

	int bufferSize = sampleRate * 2;
	engine->musicSamples = new int16_t[bufferSize];
	engine->bufferSize = bufferSize;
	engine->sampleCount = bufferSize / format.nChannels;

	engine->bufferSize = bufferSize;
	engine->musicPtr = ptr;
	engine->musicSource = source;

	hr = source->Start(0);
    CheckForError(hr, "Failed to start playing sound.\n");

    ChangeVolume(engine, engine->maxMusicVolume, 3.0f);
}

static inline int GetMusicSamples(AudioEngine* engine, int count)
{
	return stb_vorbis_get_samples_short_interleaved(engine->musicPtr, 2, engine->musicSamples, count * 2);
}

static void SubmitMusicSamples(AudioEngine* engine, int count)
{
	int n = GetMusicSamples(engine, count);

	if (n == 0)
	{
		stb_vorbis_seek_start(engine->musicPtr);
		n = GetMusicSamples(engine, count);
	}

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = engine->bufferSize * sizeof(int16_t);
	buffer.pAudioData = (BYTE*)engine->musicSamples;
	buffer.PlayBegin = 0;
	buffer.PlayLength = n;
	HRESULT hr = engine->musicSource->SubmitSourceBuffer(&buffer);
	CheckForError(hr, "Failed to submit the source buffer to the source voice. %s\n", GetLastErrorText().c_str());
}

void AudioEngine::OnVoiceProcessingPassStart(UINT32 bytesRequired)
{
	if (bytesRequired == 0)
		return;

	int samplesNeeded = bytesRequired / 4;
	SubmitMusicSamples(this, samplesNeeded);
}

void SoundCallback::OnStreamEnd()
{
	auto& pool = engine->voicePool;
	source->Stop();
	pool[engine->voiceCount++] = source;
	assert(engine->voiceCount < VOICE_POOL_SIZE);
}

static void ToggleMute(AudioEngine* audio)
{
	IXAudio2SourceVoice* music = audio->musicSource;

    if (audio->muted)
    {
    	music->SetVolume(audio->lastMusicVolume);
        audio->muted = false;
    }
    else
    {
    	music->GetVolume(&audio->lastMusicVolume);
        music->SetVolume(0.0f);
        audio->muted = true;
    }
}

static void PlaySound(Sound sound)
{
	AudioEngine* engine = sound.engine;

	if (engine->muted)
		return;

	IXAudio2SourceVoice* source;
	HRESULT hr;

	if (engine->voiceCount > 0)
	{
		source = engine->voicePool[--engine->voiceCount];
		assert(engine->voiceCount >= 0);
	}
	else
	{
		SoundCallback* callback = new SoundCallback();

		WAVEFORMATEX format = GetFormat(sound.sampleRate);
		hr = engine->pXAudio->CreateSourceVoice(&source, &format, 0, 2.0f, callback);
		CheckForError(hr, "Failed to create the source voice.\n");

		callback->engine = engine;
		callback->source = source;
	}

	XAUDIO2_BUFFER buffer = {};
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = sound.sampleCount * 4;
	buffer.pAudioData = (BYTE*)sound.samples;
	hr = source->SubmitSourceBuffer(&buffer);
	CheckForError(hr, "Failed to submit the source buffer to the source voice. %s\n", GetLastErrorText().c_str());

	hr = source->Start(0);
	CheckForError(hr, "Failed to start playing sound.\n");
}

static void UpdateAudio(AudioEngine* engine, float deltaTime)
{
	if (engine->muted) return;

	IXAudio2SourceVoice* music = engine->musicSource;
	LerpData<float>& lData = engine->volumeLerp;

	if (lData.t < 1.0f)
	{
		lData.t = Min(lData.t + (deltaTime / lData.time), 1.0f);
		music->SetVolume(Lerp(lData.start, lData.end, lData.t));
	}
}
