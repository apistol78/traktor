/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBuffer", IndexBuffer, Object)

IndexBuffer::IndexBuffer(IndexType indexType, uint32_t bufferSize)
:	m_indexType(indexType)
,	m_bufferSize(bufferSize)
{
}

IndexType IndexBuffer::getIndexType() const
{
	return m_indexType;
}

uint32_t IndexBuffer::getBufferSize() const
{
	return m_bufferSize;
}

	}
}
