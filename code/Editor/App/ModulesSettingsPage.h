/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_ModulesSettingsPage_H
#define traktor_editor_ModulesSettingsPage_H

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class ButtonClickEvent;

		namespace custom
		{

class ListBox;

		}
	}

	namespace editor
	{

/*! \brief Interface for settings pages.
 * \ingroup Editor
 */
class ModulesSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool apply(PropertyGroup* settings) T_OVERRIDE T_FINAL;

private:
	Ref< const PropertyGroup > m_originalSettings;
	Ref< ui::custom::ListBox > m_listModules;

	void eventAddModule(ui::ButtonClickEvent* event);

	void eventRemoveModule(ui::ButtonClickEvent* event);

	void eventResetModules(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_ModulesSettingsPage_H
