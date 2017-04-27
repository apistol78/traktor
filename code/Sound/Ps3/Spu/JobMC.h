/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	struct MixerData
	{
		uintptr_t lsbEA;
		uintptr_t rsbEA;
		uint32_t count;
		uint32_t rcount;
		float factor;
	}
	mixer;

	// Ensure job descriptor is 128 byte(s).
	uint8_t pad[128 - sizeof(CellSpursJobHeader) - sizeof(MixerData)];
};

	}
}

#endif	// traktor_sound_JobMC_H
