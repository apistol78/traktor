#ifndef traktor_editor_PipelineSettingsPage_H
#define traktor_editor_PipelineSettingsPage_H

#include "Core/Heap/Ref.h"
#include "Editor/ISettingsPage.h"

namespace traktor
{
	namespace ui
	{

class CheckBox;
class Edit;
class Event;

	}

	namespace editor
	{

/*! \brief Pipeline settings pages.
 * \ingroup Editor
 */
class PipelineSettingsPage : public ISettingsPage
{
	T_RTTI_CLASS(PipelineSettingsPage)

public:
	virtual bool create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(Settings* settings);

private:
	Ref< ui::CheckBox > m_checkUseMemCached;
	Ref< ui::Edit > m_editMemCachedHost;
	Ref< ui::Edit > m_editMemCachedPort;

	void eventUseMemCachedClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_PipelineSettingsPage_H
