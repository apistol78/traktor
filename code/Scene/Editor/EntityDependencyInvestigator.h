/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class Menu;
class TreeView;
class TreeViewItemActivateEvent;

	}

	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;

/*! Entity asset dependency investigator. */
class EntityDependencyInvestigator : public ui::Container
{
	T_RTTI_CLASS;

public:
	EntityDependencyInvestigator(SceneEditorContext* context);

	virtual void destroy() override final;

	bool create(ui::Widget* parent);

	void setEntityAdapter(EntityAdapter* entityAdapter);

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::TreeView > m_dependencyTree;
	Ref< ui::Menu > m_menuInstance;
	Ref< ui::Menu > m_menuAsset;
	Ref< ui::Menu > m_menuExternalFile;
	Ref< EntityAdapter > m_currentEntityAdapter;

	void eventDependencyButtonDown(ui::MouseButtonDownEvent* event);

	void eventDependencyActivate(ui::TreeViewItemActivateEvent* event);

	void eventContextSelect(ui::SelectionChangeEvent* event);
};

	}
}

