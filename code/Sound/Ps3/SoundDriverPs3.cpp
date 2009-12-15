#include <cell/audio.h>
#include <sys/timer.h>
#include <sysutil/sysutil_sysparam.h>
#include "Core/Log/Log.h"
#include "Sound/Ps3/SoundDriverPs3.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const int32_t c_remap2ch[] = { 0, 1,  2, 3, 4, 5, 6, 7 };
const int32_t c_remap5_1ch[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
const int32_t c_remap7_1ch[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverPs3", 0, SoundDriverPs3, ISoundDriver)

SoundDriverPs3::SoundDriverPs3()
:	m_port(0)
{
}

SoundDriverPs3::~SoundDriverPs3()
{
}

bool SoundDriverPs3::create(const SoundDriverCreateDesc& desc)
{
	CellAudioOutConfiguration audioOutConfig;
	CellAudioPortParam audioParam;
	CellAudioPortConfig	portConfig;
	int err;

	if (desc.frameSamples != CELL_AUDIO_BLOCK_SAMPLES)
	{
		log::error << L"Unable to create PS3 audio; must have " << CELL_AUDIO_BLOCK_SAMPLES << L" frameSamples" << Endl;
		return false;
	}

	int32_t channelsAvailPcm = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_LPCM, CELL_AUDIO_OUT_FS_44KHZ, 0);
	int32_t channelsAvailDts = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_DTS, CELL_AUDIO_OUT_FS_44KHZ, 0);
	int32_t channelsAvailAc3 = cellAudioOutGetSoundAvailability(CELL_AUDIO_OUT_PRIMARY, CELL_AUDIO_OUT_CODING_TYPE_AC3, CELL_AUDIO_OUT_FS_44KHZ, 0);
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
		T_BREAKPOINT;
	}

	err = cellAudioInit();
	if (err != CELL_OK)
	{
		log::error << L"Unable to create PS3 audio; cellAudioInit failed" << Endl;
		return false;
	}

	switch (desc.hwChannels)
	{
	case 2:
		audioParam.nChannel = CELL_AUDIO_PORT_2CH;
		m_blockChannels = 2;
		m_remap = c_remap2ch;
		break;

	case 5+1:
		audioParam.nChannel = CELL_AUDIO_PORT_8CH;
		m_blockChannels = 8;
		m_remap = c_remap5_1ch;
		break;

	case 7+1:
		audioParam.nChannel = CELL_AUDIO_PORT_8CH;
		m_blockChannels = 8;
		m_remap = c_remap7_1ch;
		break;

	default:
		cellAudioQuit();
		return false;
	}
	
	audioParam.nBlock = CELL_AUDIO_BLOCK_8;
	audioParam.attr = 0;

	err = cellAudioPortOpen(&audioParam, &m_port);
	if (err != CELL_OK)
	{
		cellAudioQuit();
		return false;
	}

	err = cellAudioGetPortConfig(m_port, &portConfig);
	if (err != CELL_OK)
	{
		cellAudioPortClose(m_port);
		cellAudioQuit();
		return false;
	}

	m_blockPtr = (uint8_t*)portConfig.portAddr; 
	m_readIndexPtr = (uint64_t*)portConfig.readIndexAddr;
	m_lastReadBlock = *m_readIndexPtr;
	m_writeBlock = 0;

	cellAudioPortStart(m_port);

	return true;
}

void SoundDriverPs3::destroy()
{
	cellAudioPortStop(m_port);
	cellAudioPortClose(m_port);
	cellAudioQuit();
}

void SoundDriverPs3::wait()
{
	uint64_t currentReadBlock;

	for (;;)
	{
		currentReadBlock = *m_readIndexPtr;
		if (currentReadBlock != m_lastReadBlock)
			break;

		sys_timer_usleep(2 * 1000);
	}

	m_writeBlock = (currentReadBlock + 1) % 8;
	m_lastReadBlock = currentReadBlock;
}

void SoundDriverPs3::submit(const SoundBlock& soundBlock)
{
	uint32_t blockSize = m_blockChannels * CELL_AUDIO_BLOCK_SAMPLES * sizeof(float);
	float* blockPtr = (float*)(m_blockPtr + m_writeBlock * blockSize);

	for (int32_t i = 0; i < CELL_AUDIO_BLOCK_SAMPLES; ++i)
	{
		for (int32_t c = 0; c < m_blockChannels; ++c)
		{
			int32_t oc = m_remap[c];
			blockPtr[oc] = soundBlock.samples[c] ? soundBlock.samples[c][i] : 0.0f;
		}
		blockPtr += m_blockChannels;
	}
}

	}
}
