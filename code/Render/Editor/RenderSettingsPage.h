#ifndef traktor_render_RenderSettingsPage_H
#define traktor_render_RenderSettingsPage_H

#include "Editor/ISettingsPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class DropDown;
class Edit;

	}

	namespace render
	{

/*! \brief Editor render settings page.
 * \ingroup Render
 */
class T_DLLCLASS RenderSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, editor::Settings* settings, const std::list< ui::Command >& shortcutCommands);

	virtual void destroy();

	virtual bool apply(editor::Settings* settings);

private:
	Ref< ui::DropDown > m_dropRenderSystem;
	Ref< ui::Edit > m_editMipBias;
};

	}
}

#endif	// traktor_render_RenderSettingsPage_H
