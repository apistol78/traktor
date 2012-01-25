#ifndef traktor_spray_SoundTrigger_H
#define traktor_spray_SoundTrigger_H

#include "Resource/Proxy.h"
#include "Spray/ITrigger.h"

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
class T_DLLCLASS SoundTrigger : public ITrigger
{
	T_RTTI_CLASS;

public:
	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual Ref< ITriggerInstance > createInstance() const;

	virtual bool serialize(ISerializer& s);

	const resource::Proxy< sound::Sound >& getSound() const { return m_sound; }

private:
	resource::Proxy< sound::Sound > m_sound;
};

	}
}

#endif	// traktor_spray_SoundTrigger_H
