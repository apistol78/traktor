/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include <string>
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

class PropertyGroup;

}

namespace traktor::ui
{

class IBitmap;
class Widget;

}

namespace traktor::editor
{

class IEditor;

/*! Editor tool base class.
 * \ingroup Editor
 */
class T_DLLCLASS IEditorTool : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const = 0;

	virtual Ref< ui::IBitmap > getIcon() const = 0;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const = 0;

	virtual bool launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param) = 0;
};

}
