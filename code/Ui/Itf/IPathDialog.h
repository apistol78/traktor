/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Io/Path.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

class IWidget;

/*! PathDialog interface.
 * \ingroup UI
 */
class IPathDialog
{
public:
	virtual ~IPathDialog() {}

	virtual bool create(IWidget* parent, const std::wstring& title) = 0;

	virtual void destroy() = 0;

	virtual DialogResult showModal(Path& outPath) = 0;
};

	}
}

