//
// Jason Bricco
//

#define CheckForError(hr, ...) if (FAILED(hr)) ErrorBox(__VA_ARGS__)

static void InitAudio(AudioEngine* engine, int sampleRate, int bufferSize)
{
	HRESULT hr;

	IXAudio2* pXAudio = NULL;
	hr = XAudio2Create(&pXAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
	CheckForError(hr, "Failed to create IXAudio2.\n");

	// The mastering voice sends final audio data to the hardware.
	IXAudio2MasteringVoice* masteringVoice = NULL;

	hr = pXAudio->CreateMasteringVoice(&masteringVoice);
	CheckForError(hr, "Failed to create the mastering voice.\n");

	WAVEFORMATEX format = {};
	format.wFormatTag = WAVE_FORMAT_PCM;	
	format.nChannels = 2;
	format.nSamplesPerSec = sampleRate;
	format.wBitsPerSample = 16;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	IXAudio2SourceVoice* sourceVoice = NULL;
	hr = pXAudio->CreateSourceVoice(&sourceVoice, &format, 0, 2.0f, engine);
	sourceVoice->SetFrequencyRatio(1.1f);

	CheckForError(hr, "Failed to create the source voice.\n");

	engine->samples = Malloc<int16_t>(bufferSize);
	engine->bufferSize = bufferSize;
	engine->sampleCount = bufferSize / format.nChannels;

	hr = sourceVoice->Start(0);
    CheckForError(hr, "Failed to start playing sound.\n");

    engine->pXAudio = pXAudio;
    engine->masteringVoice = masteringVoice;
    engine->sourceVoice = sourceVoice;
}

static void OpenMusic(AudioEngine* engine, char* path)
{
	path = PathToExe(path);

	int error;
	engine->musicPtr = stb_vorbis_open_filename(path, &error, nullptr);

	if (engine->musicPtr == nullptr) 
		ErrorBox("Couldn't open music at %s. Error: %i\n", path, error);
}

static void FillAndSubmit(AudioEngine* engine, int count)
{
	int n = stb_vorbis_get_samples_short_interleaved(engine->musicPtr, 2, engine->samples, count * 2);
	assert(n > 0);
	Unused(n);

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = engine->bufferSize * sizeof(int16_t);
	buffer.pAudioData = (BYTE*)engine->samples;
	buffer.PlayBegin = 0;
	buffer.PlayLength = count;
	HRESULT hr = engine->sourceVoice->SubmitSourceBuffer(&buffer);
	CheckForError(hr, "Failed to submit the source buffer to the source voice. %s\n", GetLastErrorText().c_str());
}

void AudioEngine::OnVoiceProcessingPassStart(UINT32 bytesRequired)
{
	if (bytesRequired == 0)
		return;

	int samplesNeeded = bytesRequired / 4;
	FillAndSubmit(this, samplesNeeded);
}

static void PlaySound(SoundAsset*)
{
	// TODO
}
