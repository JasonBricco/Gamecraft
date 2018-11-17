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
	hr = pXAudio->CreateSourceVoice(&sourceVoice, &format, 0, 1.0f, engine);
	CheckForError(hr, "Failed to create the source voice.\n");

	engine->samples = Malloc<int16_t>(bufferSize);
	engine->bufferSize = bufferSize;

	hr = sourceVoice->Start(0);
    CheckForError(hr, "Failed to start playing sound.\n");

    engine->pXAudio = pXAudio;
    engine->masteringVoice = masteringVoice;
    engine->sourceVoice = sourceVoice;
}

static void FillAndSubmit(AudioEngine* engine, int start, int count)
{
	for (int i = start; i < start + count; i += 2)
	{
		int16_t sample = (int16_t)(sin(engine->tSin) * 16000);
		engine->samples[i] = sample;
		engine->samples[i + 1] = sample;
		engine->tSin = fmod(engine->tSin + 0.016f, 2.0f * PI);
	}

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = engine->bufferSize * sizeof(int16_t);
	buffer.pAudioData = (BYTE*)engine->samples;
	buffer.Flags = 0;
	buffer.PlayBegin = start;
	buffer.PlayLength = count;
	HRESULT hr = engine->sourceVoice->SubmitSourceBuffer(&buffer);
	CheckForError(hr, "Failed to submit the source buffer to the source voice. %s\n", GetLastErrorText().c_str());
}

void AudioEngine::OnVoiceProcessingPassStart(UINT32 bytesRequired)
{
	if (bytesRequired == 0)
		return;

	int count = bytesRequired / sizeof(int16_t);
	int startIndex = write;
	int endIndex = startIndex + count;

	if (endIndex <= bufferSize)
		FillAndSubmit(this, startIndex, endIndex - startIndex);
	else
	{
		FillAndSubmit(this, startIndex, bufferSize - startIndex);
		FillAndSubmit(this, 0, endIndex % bufferSize);
	}

	write = (write + count) % bufferSize;
}

static void PlaySound(SoundAsset*)
{
	// TODO
}
