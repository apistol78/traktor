#ifndef traktor_sound_RandomGrainData_H
#define traktor_sound_RandomGrainData_H

#include "Core/RefArray.h"
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

class T_DLLCLASS RandomGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	RandomGrainData();

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const;

	virtual void serialize(ISerializer& s);

	void addGrain(IGrainData* grain) { m_grains.push_back(grain); }

	void removeGrain(IGrainData* grain) { m_grains.remove(grain); }

	const RefArray< IGrainData >& getGrains() const { return m_grains; }

private:
	RefArray< IGrainData > m_grains;
	bool m_humanize;
};

	}
}

#endif	// traktor_sound_RandomGrainData_H
