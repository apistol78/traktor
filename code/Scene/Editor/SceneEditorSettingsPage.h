#ifndef traktor_scene_SceneEditorSettingsPage_H
#define traktor_scene_SceneEditorSettingsPage_H

#include "Editor/ISettingsPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class CheckBox;

	}

	namespace scene
	{

/*! \brief Scene editor settings page.
 * \ingroup Scene
 */
class SceneEditorSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(PropertyGroup* settings);

private:
	Ref< ui::CheckBox > m_checkInvertMouseWheel;
	Ref< ui::CheckBox > m_checkInvertPanY;
};

	}
}

#endif	// traktor_scene_SceneEditorSettingsPage_H
