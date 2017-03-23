#ifndef traktor_script_ScriptSettingsPage_H
#define traktor_script_ScriptSettingsPage_H

#include "Editor/ISettingsPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
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

	namespace script
	{

/*! \brief Editor script settings page.
 * \ingroup Script
 */
class T_DLLCLASS ScriptSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool apply(PropertyGroup* settings) T_OVERRIDE T_FINAL;

private:
	Ref< ui::CheckBox > m_checkAutoOpenDebugger;
	Ref< ui::CheckBox > m_checkAutoOpenScript;
};

	}
}

#endif	// traktor_script_ScriptSettingsPage_H
