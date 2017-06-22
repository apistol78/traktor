/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>
#include "Flash/Packer.h"

namespace traktor
{
	namespace flash
	{

class PackerImpl
{
public:
	PackerImpl(int32_t width, int32_t height)
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
			outRectangle.x = r.x;
			outRectangle.y = r.y;
			outRectangle.width = r.w;
			outRectangle.height = r.h;
			return true;
		}
		else
			return false;
	}

private:
	stbrp_context* m_packer;
	stbrp_node* m_nodes;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Packer", Packer, Object)

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
}
