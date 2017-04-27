/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cell/audio.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Sound/Ps3/SoundDriverPs3.h"
#include "Sound/Ps3/SoundMixerPs3.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverPs3", 0, SoundDriverPs3, ISoundDriver)

SoundDriverPs3::SoundDriverPs3()
:	m_port(0)
,	m_eventQueueKey(0)
,	m_blockPtr(0)
,	m_readIndexPtr(0)
,	m_writeCount(0)
,	m_blockChannels(0)
,	m_blocksPerFrame(0)
{
}

SoundDriverPs3::~SoundDriverPs3()
{
}

bool SoundDriverPs3::create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	CellAudioOutConfiguration audioOutConfig;
	CellAudioPortParam audioParam;
	CellAudioPortConfig	portConfig;
	int err;

	if (desc.sampleRate != 48000)
	{
		log::error << L"Unable to create PS3 audio; \"sampleRate\" must be 48000" << Endl;
		return false;
	}
	if ((desc.frameSamples % CELL_AUDIO_BLOCK_SAMPLES) != 0)
	{
		log::error << L"Unable to create PS3 audio; \"frameSamples\" must be a multiple of " << CELL_AUDIO_BLOCK_SAMPLES << Endl;
		return false;
	}

	if (desc.hwChannels > 2)
	{
		int32_t channelsAvailPcm = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM, CELL_AUDIO_OUT_FS_48KHZ, 0);
		int32_t channelsAvailDts = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_DTS, CELL_AUDIO_OUT_FS_48KHZ, 0);
		int32_t channelsAvailAc3 = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_AC3, CELL_AUDIO_OUT_FS_48KHZ, 0);
		int32_t channelsAvail = 0;

		log::info << L"Available channels:" << Endl <<
			L"\tPCM " << channelsAvailPcm << Endl <<
			L"\tDTS " << channelsAvailDts << Endl <<
			L"\tAC3 " << channelsAvailAc3 << Endl;

		std::memset(&audioOutConfig, 0, sizeof(audioOutConfig));
		if (channelsAvailPcm >= channelsAvailAc3 && channelsAvailPcm >= channelsAvailDts)
		{
			channelsAvail = channelsAvailPcm;
			audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
			log::info << L"Using PCM encoding" << Endl;
		}
		else if (channelsAvailDts >= channelsAvailAc3)
		{
			channelsAvail = channelsAvailDts;
			audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_DTS;
			log::info << L"Using DTS encoding" << Endl;
		}
		else
		{
			channelsAvail = channelsAvailAc3;
			audioOutConfig.encoder = CELL_AUDIO_OUT_CODING_TYPE_AC3;
			log::info << L"Using AC3 encoding" << Endl;
		}

		if (channelsAvail == 6)
		{
			log::info << L"Trying with 7.1 -> 5.1 downmix" << Endl;

			audioOutConfig.channel = channelsAvail;
			audioOutConfig.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_B;

			err = cellAudioOutConfigure(CELL_AUDIO_OUT_PRIMARY, &audioOutConfig, NULL, 0);
			if (err != CELL_OK)
			{
				log::error << L"Unable to create PS3 audio; cellAudioOutConfigure failed" << Endl;
				return false;
			}
		}
		else if (channelsAvail == 2)
		{
			log::info << L"Trying with 7.1 -> 2.0 downmix" << Endl;

			audioOutConfig.channel = channelsAvail;
			audioOutConfig.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_A;

			err = cellAudioOutConfigure(CELL_AUDIO_OUT_PRIMARY, &audioOutConfig, NULL, 0);
			if (err != CELL_OK)
			{
				log::error << L"Unable to create PS3 audio; cellAudioOutConfigure failed" << Endl;
				return false;
			}
		}
	}

	err = cellAudioInit();
	if (err != CELL_OK)
	{
		log::error << L"Unable to create PS3 audio; cellAudioInit failed" << Endl;
		return false;
	}

	std::memset(&audioParam, 0, sizeof(audioParam));

	switch (desc.hwChannels)
	{
	case 2:
		audioParam.nChannel = CELL_AUDIO_PORT_2CH;
		m_blockChannels = 2;
		break;

	case 5+1:
		audioParam.nChannel = CELL_AUDIO_PORT_8CH;
		m_blockChannels = 8;
		break;

	case 7+1:
		audioParam.nChannel = CELL_AUDIO_PORT_8CH;
		m_blockChannels = 8;
		break;

	default:
		cellAudioQuit();
		return false;
	}

	m_blocksPerFrame = desc.frameSamples / CELL_AUDIO_BLOCK_SAMPLES;

	audioParam.nBlock = CELL_AUDIO_BLOCK_16;
	audioParam.attr = CELL_AUDIO_PORTATTR_INITLEVEL;
	audioParam.level = 1.0f;

	err = cellAudioPortOpen(&audioParam, &m_port);
	if (err != CELL_OK)
	{
		cellAudioQuit();
		return false;
	}

	std::memset(&portConfig, 0, sizeof(portConfig));

	err = cellAudioGetPortConfig(m_port, &portConfig);
	if (err != CELL_OK)
	{
		cellAudioPortClose(m_port);
		cellAudioQuit();
		return false;
	}

	err = cellAudioCreateNotifyEventQueue(&m_eventQueue, &m_eventQueueKey);
	if (err != CELL_OK)
	{
		cellAudioPortClose(m_port);
		cellAudioQuit();
		return false;
	}

	err = cellAudioSetNotifyEventQueue(m_eventQueueKey);
	if (err < 0)
	{
		sys_event_queue_destroy(m_eventQueue, 0);
		cellAudioPortClose(m_port);
		cellAudioQuit();
		return false;
	}

	m_blockPtr = (uint8_t*)portConfig.portAddr; 
	m_readIndexPtr = (uint64_t*)portConfig.readIndexAddr;
	m_writeCount = 0;

	sys_event_queue_drain(m_eventQueue);
	cellAudioPortStart(m_port);

	// Create SPU-accelerated sound mixer.
	Ref< SoundMixerPs3 > mixer = new SoundMixerPs3();
	if (mixer->create())
		outMixer = mixer;

	return true;
}

void SoundDriverPs3::destroy()
{
	cellAudioRemoveNotifyEventQueue(m_eventQueueKey);
	sys_event_queue_destroy(m_eventQueue, 0);

	cellAudioPortStop(m_port);
	cellAudioPortClose(m_port);
	cellAudioQuit();
}

void SoundDriverPs3::wait()
{
	sys_event_t event;

	for (;;)
	{
		uint64_t writeIndex = m_writeCount % 16;
		uint64_t readIndex = *m_readIndexPtr;

		int32_t nfree = 0;
		while (writeIndex != readIndex)
		{
			writeIndex = (writeIndex + 1) % 16;
			++nfree;
		}

		if (nfree >= m_blocksPerFrame)
			return;

		int err = sys_event_queue_receive(m_eventQueue, &event, 4 * 1000 * 1000);
		if (err == ETIMEDOUT)
			log::warning << L"No sound event; something wrong..." << Endl;
	}
}

void SoundDriverPs3::submit(const SoundBlock& soundBlock)
{
	T_ASSERT (soundBlock.samplesCount % CELL_AUDIO_BLOCK_SAMPLES == 0);
	T_ASSERT (soundBlock.samplesCount / CELL_AUDIO_BLOCK_SAMPLES <= m_blocksPerFrame);

	uint32_t blockSize = m_blockChannels * CELL_AUDIO_BLOCK_SAMPLES * sizeof(float);
	for (uint32_t offset = 0; offset < soundBlock.samplesCount; offset += CELL_AUDIO_BLOCK_SAMPLES)
	{
		uint64_t writeIndex = m_writeCount++ % 16;

		float* blockPtr = (float*)(m_blockPtr + writeIndex * blockSize);
		for (int32_t c = 0; c < m_blockChannels; ++c)
		{
			float* blockWritePtr = blockPtr + c;
			if (soundBlock.samples[c])
			{
				const float* samples = &soundBlock.samples[c][offset];
				for (int32_t i = 0; i < CELL_AUDIO_BLOCK_SAMPLES; ++i)
				{
					*blockWritePtr = *samples++;
					blockWritePtr += m_blockChannels;
				}
			}
			else
			{
				for (int32_t i = 0; i < CELL_AUDIO_BLOCK_SAMPLES; ++i)
				{
					*blockWritePtr = 0.0f;
					blockWritePtr += m_blockChannels;
				}
			}
		}
	}
}

	}
}
