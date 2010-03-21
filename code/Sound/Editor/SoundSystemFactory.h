#ifndef traktor_sound_SoundSystemFactory_H
#define traktor_sound_SoundSystemFactory_H

#include "Core/Object.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace sound
	{

class SoundSystem;

/*! \brief Sound system factory.
 * \ingroup Sound
 */
class SoundSystemFactory : public Object
{
	T_RTTI_CLASS;

public:
	SoundSystemFactory(editor::IEditor* editor);

	virtual Ref< SoundSystem > createSoundSystem() const;

private:
	editor::IEditor* m_editor;
};

	}
}

#endif	// traktor_sound_SoundSystemFactory_H
