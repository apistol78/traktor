#ifndef traktor_editor_GeneralSettingsPage_H
#define traktor_editor_GeneralSettingsPage_H

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class DropDown;
class CheckBox;
class Edit;

	}

	namespace editor
	{

/*! \brief Interface for settings pages.
 * \ingroup Editor
 */
class GeneralSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool apply(PropertyGroup* settings) T_OVERRIDE T_FINAL;

private:
	Ref< ui::Edit > m_editDictionary;
	Ref< ui::DropDown > m_dropStyleSheet;
	Ref< ui::DropDown > m_dropFonts;
	Ref< ui::Edit > m_editFontSize;
	Ref< ui::CheckBox > m_checkAutoOpen;
	Ref< ui::CheckBox > m_checkAutoSave;
	Ref< ui::CheckBox > m_checkBuildWhenSourceModified;
	Ref< ui::CheckBox > m_checkBuildWhenAssetModified;
	Ref< ui::CheckBox > m_checkBuildAfterBrowseInstance;
	Ref< ui::CheckBox > m_checkPropertyHelpVisible;
};

	}
}

#endif	// traktor_editor_GeneralSettingsPage_H
