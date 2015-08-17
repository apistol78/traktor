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
	virtual bool create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(PropertyGroup* settings);

private:
	Ref< ui::Edit > m_editCertificate;
	Ref< ui::CheckBox > m_checkInheritCache;
	Ref< ui::CheckBox > m_checkHidePipeline;
	Ref< ui::CheckBox > m_checkUseDebugBinaries;
};

	}
}

#endif	// traktor_amalgam_DeploySettingsPage_H
