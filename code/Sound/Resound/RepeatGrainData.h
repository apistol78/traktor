#ifndef traktor_sound_RepeatGrainData_H
#define traktor_sound_RepeatGrainData_H

#include "Sound/Resound/IGrainData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS RepeatGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	RepeatGrainData();

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const;

	virtual void serialize(ISerializer& s);

	uint32_t getCount() const { return m_count; }

	IGrainData* getGrain() const { return m_grain; }

private:
	uint32_t m_count;
	Ref< IGrainData > m_grain;
};

	}
}

#endif	// traktor_sound_RepeatGrainData_H
