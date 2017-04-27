/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_BlobPs4_H
#define traktor_render_BlobPs4_H

#include "Core/Object.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace render
	{

class BlobPs4 : public Object
{
	T_RTTI_CLASS;

public:
	BlobPs4();

	static Ref< BlobPs4 > create(uint32_t size);

	static Ref< BlobPs4 > create(const void* data, uint32_t size);

	void* getData() { return m_data.ptr(); }

	const void* getData() const { return m_data.c_ptr(); }

	uint32_t getSize() const { return m_size; }

private:
	AutoArrayPtr< uint8_t, AllocatorFree > m_data;
	uint32_t m_size;
};

	}
}

#endif	// traktor_render_BlobPs4_H
