#ifndef traktor_sound_SequenceGrainData_H
#define traktor_sound_SequenceGrainData_H

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

class T_DLLCLASS SequenceGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IGrain > createInstance(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

	void addGrain(IGrainData* grain) { m_grains.push_back(grain); }

	void removeGrain(IGrainData* grain) { m_grains.remove(grain); }

	const RefArray< IGrainData >& getGrains() const { return m_grains; }

private:
	RefArray< IGrainData > m_grains;
};

	}
}

#endif	// traktor_sound_SequenceGrainData_H
