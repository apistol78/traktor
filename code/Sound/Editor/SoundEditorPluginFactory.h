#ifndef traktor_sound_SoundEditorPluginFactory_H
#define traktor_sound_SoundEditorPluginFactory_H

#include "Editor/IEditorPluginFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief Sound editor plugin factory.
 * \ingroup Sound
 */
class T_DLLCLASS SoundEditorPluginFactory : public editor::IEditorPluginFactory
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(std::list< ui::Command >& outCommands) const T_OVERRIDE T_FINAL;

	virtual Ref< editor::IEditorPlugin > createEditorPlugin(editor::IEditor* editor) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_sound_SoundEditorPluginFactory_H
