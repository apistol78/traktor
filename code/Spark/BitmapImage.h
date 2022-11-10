/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Spark/Bitmap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace spark
	{

/*! Image bitmap container.
 * \ingroup Spark
 */
class T_DLLCLASS BitmapImage : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapImage() = default;

	explicit BitmapImage(const drawing::Image* image);

	const void* getBits() const;

	const drawing::Image* getImage() const { return m_image; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< drawing::Image > m_image;
};

	}
}

