/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>
#include "Spark/Packer.h"

namespace traktor::spark
{

class PackerImpl
{
public:
	explicit PackerImpl(int32_t width, int32_t height)
	{
		m_packer = new stbrp_context();
		m_nodes = new stbrp_node [width];
		stbrp_setup_allow_out_of_mem(m_packer, 1);
		stbrp_init_target(m_packer, width, height, m_nodes, width);
	}

	~PackerImpl()
	{
		delete m_packer;
		delete[] m_nodes;
	}

	bool insert(int32_t width, int32_t height, Packer::Rectangle& outRectangle)
	{
		stbrp_rect r = { 0 };
		r.w = width;
		r.h = height;
		stbrp_pack_rects(m_packer, &r, 1);
		if (r.was_packed)
		{
			outRectangle = { r.x, r.y, r.w, r.h };
			return true;
		}
		else
			return false;
	}

private:
	stbrp_context* m_packer;
	stbrp_node* m_nodes;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Packer", Packer, Object)

Packer::Packer(int32_t width, int32_t height)
{
	m_impl = new PackerImpl(width, height);
}

Packer::~Packer()
{
	delete m_impl;
}

bool Packer::insert(int32_t width, int32_t height, Rectangle& outRectangle)
{
	return m_impl->insert(width, height, outRectangle);
}

}
