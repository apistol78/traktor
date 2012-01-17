#ifndef traktor_sound_SoundSystemFactory_H
#define traktor_sound_SoundSystemFactory_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

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
class T_DLLCLASS SoundSystemFactory : public Object
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
