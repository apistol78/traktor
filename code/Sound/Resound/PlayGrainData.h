#ifndef traktor_sound_PlayGrainData_H
#define traktor_sound_PlayGrainData_H

#include "Core/RefArray.h"
#include "Core/Math/Range.h"
#include "Resource/Id.h"
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

class IFilter;
class Sound;

class T_DLLCLASS PlayGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	PlayGrainData();

	virtual Ref< IGrain > createInstance(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< Sound >& getSound() const { return m_sound; }

	const Range< float >& getGain() const { return m_gain; }

	const Range< float >& getPitch() const { return m_pitch; }

	bool getRepeat() const { return m_repeat; }

private:
	resource::Id< Sound > m_sound;
	RefArray< IFilter > m_filters;
	Range< float > m_gain;
	Range< float > m_pitch;
	bool m_repeat;
};

	}
}

#endif	// traktor_sound_PlayGrainData_H
