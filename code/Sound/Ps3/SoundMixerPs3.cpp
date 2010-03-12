#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#include "Core/Thread/Ps3/Spurs/SpursManager.h"
#include "Sound/SoundMixer.h"
#include "Sound/Ps3/SoundMixerPs3.h"

extern char _binary_jqjob_Traktor_Sound_Ps3_JobMixerMC_bin_start[];
extern char _binary_jqjob_Traktor_Sound_Ps3_JobMixerMC_bin_size[];
extern char _binary_jqjob_Traktor_Sound_Ps3_JobMixerAMC_bin_start[];
extern char _binary_jqjob_Traktor_Sound_Ps3_JobMixerAMC_bin_size[];

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundMixerPs3", SoundMixerPs3, ISoundMixer)

bool SoundMixerPs3::create()
{
	m_jobQueue = SpursManager::getInstance().createJobQueue(sizeof(CellSpursJob128), 1);
	m_mixer = new SoundMixer();
	return true;
}

void SoundMixerPs3::destroy()
{
	safeDestroy(m_jobQueue);
}

void SoundMixerPs3::mulConst(float* sb, uint32_t count, float factor) const
{
	CellSpursJob128 job;

	if (alignUp(sb, 16) != sb || count <= 4 || count >= 1024)
		return;

	__builtin_memset(&job, 0, sizeof(CellSpursJob128));
	job.header.eaBinary = (uintptr_t)_binary_jqjob_Traktor_Sound_Ps3_JobMixerMC_bin_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(_binary_jqjob_Traktor_Sound_Ps3_JobMixerMC_bin_size);
	job.workArea.userData[0] = (uintptr_t)sb;
	job.workArea.userData[1] = (uintptr_t)sb;
	job.workArea.userData[2] = count;
	job.workArea.userData[3] = *(uint32_t*)&factor;

	m_jobQueue->push(&job);
	m_jobQueue->wait();
}

void SoundMixerPs3::mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	CellSpursJob128 job;

	if (alignUp(lsb, 16) != lsb || alignUp(rsb, 16) != rsb || count <= 4 || count >= 1024)
		return;

	__builtin_memset(&job, 0, sizeof(CellSpursJob128));
	job.header.eaBinary = (uintptr_t)_binary_jqjob_Traktor_Sound_Ps3_JobMixerMC_bin_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(_binary_jqjob_Traktor_Sound_Ps3_JobMixerMC_bin_size);
	job.workArea.userData[0] = (uintptr_t)lsb;
	job.workArea.userData[1] = (uintptr_t)rsb;
	job.workArea.userData[2] = count;
	job.workArea.userData[3] = *(uint32_t*)&factor;

	m_jobQueue->push(&job);
	m_jobQueue->wait();
}

void SoundMixerPs3::addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	m_mixer->addMulConst(lsb, rsb, count, factor);
}

void SoundMixerPs3::mute(float* sb, uint32_t count) const
{
	m_mixer->mute(sb, count);
}

	}
}
