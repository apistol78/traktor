#ifndef traktor_sound_MuteGrainData_H
#define traktor_sound_MuteGrainData_H

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

class T_DLLCLASS MuteGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	MuteGrainData();

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	double getDuration() const { return m_duration; }

private:
	double m_duration;
};

	}
}

#endif	// traktor_sound_MuteGrainData_H
