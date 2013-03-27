#ifndef traktor_spray_SoundTriggerData_H
#define traktor_spray_SoundTriggerData_H

#include "Resource/Id.h"
#include "Spray/ITriggerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Sound;

	}

	namespace spray
	{

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS SoundTriggerData : public ITriggerData
{
	T_RTTI_CLASS;

public:
	SoundTriggerData();

	virtual Ref< ITrigger > createTrigger(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(ISerializer& s);

	const resource::Id< sound::Sound >& getSound() const { return m_sound; }

private:
	friend class SoundTrigger;

	resource::Id< sound::Sound > m_sound;
	bool m_positional;
	bool m_follow;
	bool m_repeat;
	bool m_infinite;
};

	}
}

#endif	// traktor_spray_SoundTriggerData_H
