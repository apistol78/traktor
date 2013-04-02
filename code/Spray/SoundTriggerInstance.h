#ifndef traktor_spray_SoundTriggerInstance_H
#define traktor_spray_SoundTriggerInstance_H

#include "Resource/Proxy.h"
#include "Spray/ITriggerInstance.h"

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

class ISoundHandle;
class Sound;

	}

	namespace spray
	{

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS SoundTriggerInstance : public ITriggerInstance
{
	T_RTTI_CLASS;

public:
	virtual ~SoundTriggerInstance();

	virtual void perform(Context& context, const Transform& transform, bool enable);

	virtual void update(Context& context, const Transform& transform, bool enable);

private:
	friend class SoundTrigger;

	resource::Proxy< sound::Sound > m_sound;
	Ref< sound::ISoundHandle > m_handle;
	bool m_positional;
	bool m_follow;
	bool m_repeat;

	SoundTriggerInstance(const resource::Proxy< sound::Sound >& sound, bool positional, bool follow, bool repeat);
};

	}
}

#endif	// traktor_spray_SoundTriggerInstance_H
