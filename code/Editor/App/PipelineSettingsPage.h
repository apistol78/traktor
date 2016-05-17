#ifndef traktor_editor_PipelineSettingsPage_H
#define traktor_editor_PipelineSettingsPage_H

#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class ButtonClickEvent;
class CheckBox;
class Edit;

	}

	namespace editor
	{

/*! \brief Pipeline settings pages.
 * \ingroup Editor
 */
class PipelineSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool apply(PropertyGroup* settings) T_OVERRIDE T_FINAL;

private:
	Ref< ui::CheckBox > m_checkDependsThreads;
	Ref< ui::CheckBox > m_checkBuildThreads;
	Ref< ui::CheckBox > m_checkBuildDistributed;
	Ref< ui::CheckBox > m_checkUseMemCached;
	Ref< ui::Edit > m_editMemCachedHost;
	Ref< ui::Edit > m_editMemCachedPort;
	Ref< ui::CheckBox > m_checkMemCachedRead;
	Ref< ui::CheckBox > m_checkMemCachedWrite;
	Ref< ui::CheckBox > m_checkUseFileCache;
	Ref< ui::Edit > m_editFileCachePath;
	Ref< ui::CheckBox > m_checkFileCacheRead;
	Ref< ui::CheckBox > m_checkFileCacheWrite;
	
	void eventUseCacheClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_PipelineSettingsPage_H
