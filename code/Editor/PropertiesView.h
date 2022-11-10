/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Widget.h"

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

/*!
 * \ingroup Editor
 */
class T_DLLCLASS PropertiesView : public ui::Widget
{
	T_RTTI_CLASS;

public:
	/*! Attach object to property view. */
	virtual void setPropertyObject(ISerializable* object) = 0;

	/*! Handle command. */
	virtual bool handleCommand(const ui::Command& command) = 0;
};

}
