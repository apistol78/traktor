/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Rtti/TypeInfo.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Instance;

}

namespace traktor::ui
{

class Widget;

}

namespace traktor::editor
{

class IEditor;

/*! Resolves guids which refer to something inside an instance.
 * \ingroup Editor
 */
class T_DLLCLASS IReferenceResolver : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool initialize(IEditor* editor) = 0;

	virtual TypeInfoSet getReferenceTypes() const = 0;

	virtual bool resolve(const Guid& id, std::wstring& outName) const = 0;

	virtual bool browse(ui::Widget* parent, const TypeInfo& referenceType, const Guid& current, Guid& outId) const = 0;

	virtual Ref< db::Instance > getOwnerInstance(const Guid& id) const = 0;
};

}
