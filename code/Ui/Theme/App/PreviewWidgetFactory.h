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
#include "Core/Ref.h"

namespace traktor
{
	namespace ui
	{

class StyleSheet;
class Widget;

class PreviewWidgetFactory : public Object
{
	T_RTTI_CLASS;

public:
	Ref< Widget > create(Widget* parent, const StyleSheet* styleSheet, const std::wstring& typeName) const;
};

	}
}
