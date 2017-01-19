#ifndef traktor_amalgam_DeploySettingsPage_H
#define traktor_amalgam_DeploySettingsPage_H

#include "Editor/ISettingsPage.h"
#include "Ui/CheckBox.h"
#include "Ui/Edit.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Deployment editor settings page.
 * \ingroup Amalgam
 */
class T_DLLCLASS DeploySettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool apply(PropertyGroup* settings) T_OVERRIDE T_FINAL;

private:
	Ref< ui::CheckBox > m_checkInheritCache;
	Ref< ui::CheckBox > m_checkHidePipeline;
	Ref< ui::CheckBox > m_checkUseDebugBinaries;
	Ref< ui::CheckBox > m_checkStaticallyLinked;
	Ref< ui::CheckBox > m_checkUseVS;
	Ref< ui::Edit > m_editAndroidHome;
	Ref< ui::Edit > m_editAndroidNdkRoot;
	Ref< ui::Edit > m_editAndroidAntHome;
	Ref< ui::Edit > m_editAndroidToolchain;
	Ref< ui::Edit > m_editAndroidApiLevel;
};

	}
}

#endif	// traktor_amalgam_DeploySettingsPage_H
