/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
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
	namespace spark
	{

/*! Bitmap resource container.
 * \ingroup Spark
 */
class T_DLLCLASS BitmapResource : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapResource() = default;

	explicit BitmapResource(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t atlasWidth, uint32_t atlasHeight, const Guid& resourceId);

	uint32_t getAtlasWidth() const { return m_atlasWidth; }

	uint32_t getAtlasHeight() const { return m_atlasHeight; }

	const Guid& getResourceId() const { return m_resourceId; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_atlasWidth = 0;
	uint32_t m_atlasHeight = 0;
	Guid m_resourceId;
};

	}
}

