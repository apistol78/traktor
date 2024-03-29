/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Widget;

}

namespace traktor::db
{

class Group;
class Instance;

}

namespace traktor::editor
{

class IEditor;

/*! Wizard tool base class.
 * \ingroup Editor
 *
 * Wizards are launched by context menu
 * from database view.
 */
class T_DLLCLASS IWizardTool : public Object
{
	T_RTTI_CLASS;

public:
	enum WizardFlags
	{
		WfGroup = 1,
		WfInstance = 2
	};

	virtual std::wstring getDescription() const = 0;

	virtual const TypeInfoSet getSupportedTypes() const = 0;

	virtual uint32_t getFlags() const = 0;

	virtual bool launch(ui::Widget* parent, IEditor* editor, db::Group* group, db::Instance* instance) = 0;
};

}
