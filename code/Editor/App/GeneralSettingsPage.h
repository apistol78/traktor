#ifndef traktor_editor_GeneralSettingsPage_H
#define traktor_editor_GeneralSettingsPage_H

#include "Core/Heap/Ref.h"
#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class DropDown;
class Edit;
class CheckBox;

	}

	namespace editor
	{

/*! \brief Interface for settings pages.
 * \ingroup Editor
 */
class GeneralSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS(GeneralSettingsPage)

public:
	virtual bool create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(Settings* settings);

private:
	Ref< ui::DropDown > m_dropRenderSystem;
	Ref< ui::Edit > m_editDictionary;
	Ref< ui::Edit > m_editAssetPath;
	Ref< ui::CheckBox > m_checkOpenLastProject;
	Ref< ui::CheckBox > m_checkBuildWhenSourceModified;
	Ref< ui::CheckBox > m_checkBuildWhenAssetModified;
};

	}
}

#endif	// traktor_editor_GeneralSettingsPage_H
