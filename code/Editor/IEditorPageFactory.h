/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <set>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

}

namespace traktor::ui
{

class Command;

}

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPage;
class IEditorPageSite;

/*! Interface for creating editor pages.
 * \ingroup Editor
 */
class T_DLLCLASS IEditorPageFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const = 0;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const = 0;

	virtual Ref< IEditorPage > createEditorPage(IEditor* editor, IEditorPageSite* site, IDocument* document) const = 0;

	virtual void getCommands(std::list< ui::Command >& outCommands) const = 0;

	virtual Ref< ISerializable > cloneAsset(const ISerializable* asset) const = 0;
};

}
