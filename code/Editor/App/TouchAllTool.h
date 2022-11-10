/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_editor_TouchAllTool_H
#define traktor_editor_TouchAllTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class TouchAllTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param) override final;
};

	}
}

#endif	// traktor_editor_TouchAllTool_H

