/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Editor/IEditorPageSite.h"

namespace traktor
{
	namespace editor
	{

class EditorForm;

/*! Editor page site implementation.
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
	explicit EditorPageSite(EditorForm* editor, bool active);

	void show();

	void hide();

	virtual Ref< PropertiesView > createPropertiesView(ui::Widget* parent) override final;

	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south) override final;

	virtual void destroyAdditionalPanel(ui::Widget* widget) override final;

	virtual void showAdditionalPanel(ui::Widget* widget) override final;

	virtual void hideAdditionalPanel(ui::Widget* widget) override final;

	const std::map< ui::Widget*, bool >& getPanelWidgets() const { return m_panelWidgets; }

private:
	EditorForm* m_editor;
	std::map< ui::Widget*, bool > m_panelWidgets;
	bool m_active;
};

	}
}

