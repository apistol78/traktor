/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Render/Ps4/BlobPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BlobPs4", BlobPs4, Object)

BlobPs4::BlobPs4()
:	m_size(0)
{
}

Ref< BlobPs4 > BlobPs4::create(uint32_t size)
{
	Ref< BlobPs4 > b = new BlobPs4();
	b->m_data.reset((uint8_t*)getAllocator()->alloc(size, 16, T_FILE_LINE));
	b->m_size = size;
	return b;
}

Ref< BlobPs4 > BlobPs4::create(const void* data, uint32_t size)
{
	Ref< BlobPs4 > b = create(size);
	std::memcpy(b->m_data.ptr(), data, size);
	return b;
}

	}
}
