/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::ui
{

class ISystemBitmap;

/*! Bitmap interface.
 * \ingroup UI
 */
class T_DLLCLASS IBitmap : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual Size getSize() const = 0;

	virtual Ref< drawing::Image > getImage() const = 0;

	virtual ISystemBitmap* getSystemBitmap() const = 0;
};

}
