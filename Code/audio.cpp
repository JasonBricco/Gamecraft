//
// Jason Bricco
//

static AudioEngine* InitAudio()
{
	AudioEngine* engine = Calloc<AudioEngine>();

	IXAudio2* device;
	HRESULT result = XAudio2Create(&device, 0, XAUDIO2_DEFAULT_PROCESSOR);

	if (FAILED(result))
	{
		Print("Failed to initialize XAudio2\n");
		return nullptr;
	}

	IXAudio2MasteringVoice* master;
	result = device->CreateMasteringVoice(&master);

	if (FAILED(result))
	{
		Print("Failed to create the mastering voice in XAudio2.\n");
		return nullptr;
	}

	engine->device = device;
	engine->master = master;
}
