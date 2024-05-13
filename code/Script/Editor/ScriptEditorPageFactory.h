/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IEditorPageFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::script
{

class T_DLLCLASS ScriptEditorPageFactory : public editor::IEditorPageFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const override final;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const override final;

	virtual Ref< editor::IEditorPage > createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const override final;

	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;

	virtual Ref< ISerializable > cloneAsset(const ISerializable* asset) const override final;
};

}
