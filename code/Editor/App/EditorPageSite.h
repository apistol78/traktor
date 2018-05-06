/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_EditorPageSite_H
#define traktor_editor_EditorPageSite_H

#include <map>
#include "Core/Ref.h"
#include "Editor/IEditorPageSite.h"

namespace traktor
{
	namespace editor
	{

class EditorForm;

/*! \brief Editor page site implementation.
 * \ingroup Editor
 *
 * Keep information about editor pages and their
 * current state.
 *
 * This class just dispatch method calls to editor
 * form if page is currently active; otherwise
 * it will just modify it's internal state
 * and wait until page becomes active again.
 */
class EditorPageSite : public IEditorPageSite
{
	T_RTTI_CLASS;

public:
	EditorPageSite(EditorForm* editor, bool active);

	void show();

	void hide();

	virtual void setPropertyObject(Object* properties) T_OVERRIDE T_FINAL;

	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south) T_OVERRIDE T_FINAL;

	virtual void destroyAdditionalPanel(ui::Widget* widget) T_OVERRIDE T_FINAL;

	virtual void showAdditionalPanel(ui::Widget* widget) T_OVERRIDE T_FINAL;

	virtual void hideAdditionalPanel(ui::Widget* widget) T_OVERRIDE T_FINAL;

	const std::map< Ref< ui::Widget >, bool >& getPanelWidgets() const { return m_panelWidgets; }

private:
	EditorForm* m_editor;
	Ref< Object > m_properties;
	std::map< Ref< ui::Widget >, bool > m_panelWidgets;
	bool m_active;
};

	}
}

#endif	// traktor_editor_EditorPageSite_H
