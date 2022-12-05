/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
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
	namespace render
	{

class ITexture;

	}

	namespace spark
	{

/*! Bitmap texture container.
 * \ingroup Spark
 */
class T_DLLCLASS BitmapTexture : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapTexture() = default;

	explicit BitmapTexture(render::ITexture* texture);

	render::ITexture* getTexture() const { return m_texture; }

private:
	Ref< render::ITexture > m_texture;
};

	}
}

