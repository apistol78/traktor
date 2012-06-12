#ifndef traktor_flash_SoundRenderer_H
#define traktor_flash_SoundRenderer_H

#include "Core/Ref.h"
#include "Flash/ISoundRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class SoundSystem;
class SoundChannel;

	}

	namespace flash
	{

/*! \brief Default sound renderer using Traktor sound package.
 * \ingroup Flash
 */
class T_DLLCLASS SoundRenderer : public ISoundRenderer
{
	T_RTTI_CLASS;

public:
	bool create(sound::SoundSystem* soundSystem);

	virtual void destroy();

	virtual void play(const FlashSound* sound);

private:
	Ref< sound::SoundSystem > m_soundSystem;
	Ref< sound::SoundChannel > m_soundChannel;
};

	}
}

#endif	// traktor_flash_SoundRenderer_H
