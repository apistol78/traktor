#ifndef traktor_sound_BankSound_H
#define traktor_sound_BankSound_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class Sound;

/*! \brief Bank sound.
 * \ingroup Sound
 */
class T_DLLCLASS BankSound : public ISerializable
{
	T_RTTI_CLASS;

public:
	BankSound();

	BankSound(const resource::Proxy< Sound >& sound, float pitch);

	bool bind(resource::IResourceManager* resourceManager) const;

	resource::Proxy< Sound >& getSound();

	void setPitch(float pitch);

	float getPitch() const;

	virtual bool serialize(ISerializer& s);

private:
	mutable resource::Proxy< Sound > m_sound;
	float m_pitch;
};

	}
}

#endif	// traktor_sound_BankSound_H
