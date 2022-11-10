/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <Editor/IEditorTool.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

class ModelToolDialog;

class T_DLLCLASS ModelTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual ~ModelTool();

	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param) override final;

private:
	Ref< ModelToolDialog > m_dialog;
};

	}
}

