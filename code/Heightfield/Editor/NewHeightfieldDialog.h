/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Plane.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class Command;
class Edit;

	}

	namespace hf
	{

class NewHeightfieldDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	int32_t getGridSize();

	Vector4 getWorldExtent();

	Plane getWorldPlane();

private:
	Ref< ui::Edit > m_editGridSize;
	Ref< ui::Edit > m_editExtent[3];
	Ref< ui::Edit > m_editPlane[3];
};

	}
}

