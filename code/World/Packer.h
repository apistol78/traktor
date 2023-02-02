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

namespace traktor::world
{

/*!
 * \ingroup World
 */
class Packer : public Object
{
	T_RTTI_CLASS;

public:
	struct Rectangle
	{
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
	};

	explicit Packer(int32_t width, int32_t height);

	virtual ~Packer();

	bool insert(int32_t width, int32_t height, Rectangle& outRectangle);

	void reset();

private:
	class PackerImpl* m_impl;
};

}
