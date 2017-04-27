/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Blob_H
#define traktor_render_Blob_H

#include "Core/Object.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace render
	{

class Blob : public Object
{
	T_RTTI_CLASS;

public:
	Blob();

	static Ref< Blob > create(uint32_t size);

	static Ref< Blob > create(const void* data, uint32_t size);

	void* getData() { return m_data.ptr(); }

	const void* getData() const { return m_data.c_ptr(); }

	uint32_t getSize() const { return m_size; }

private:
	AutoArrayPtr< uint8_t, AllocatorFree > m_data;
	uint32_t m_size;
};

	}
}

#endif	// traktor_render_Blob_H
