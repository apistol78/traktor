#ifndef traktor_sound_JobMC_H
#define traktor_sound_JobMC_H

#include <cell/spurs/job_descriptor.h>

namespace traktor
{
	namespace sound
	{
		
struct JobMC
{
	CellSpursJobHeader header;
	uintptr_t lsbEA;
	uintptr_t rsbEA;
	uint32_t count;
	uint32_t rcount;
	float factor;
};

	}
}

#endif	// traktor_sound_JobMC_H
