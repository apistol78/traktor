/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Ui/Container.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class ListBox;

}

namespace traktor::render
{

class ShaderDependencyTracker;

class ShaderDependencyPane
:	public ui::Container
,	public ShaderDependencyTracker::IListener
{
	T_RTTI_CLASS;

public:
	explicit ShaderDependencyPane(editor::IEditor* editor, const Guid& shaderId);

	virtual void destroy() override final;

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Guid m_shaderId;
	Ref< ShaderDependencyTracker > m_dependencyTracker;
	Ref< ui::ListBox > m_refereeList;

	virtual void dependencyAdded(const Guid& fromShader, const Guid& toShader) override final;

	virtual void dependencyRemoved(const Guid& fromShader, const Guid& toShader) override final;

	void eventRefereeListDoubleClick(ui::MouseDoubleClickEvent* event);
};

}
