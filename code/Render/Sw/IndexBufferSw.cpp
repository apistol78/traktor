/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Sw/IndexBufferSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferSw", IndexBufferSw, IndexBuffer)

IndexBufferSw::IndexBufferSw(IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	m_lock(0)
{
	if (indexType == ItUInt16)
		m_indices.resize(bufferSize / sizeof(uint16_t));
	else
		m_indices.resize(bufferSize / sizeof(uint32_t));
}

void IndexBufferSw::destroy()
{
}

void* IndexBufferSw::lock()
{
	if (m_lock)
		return 0;

	if (getIndexType() == ItUInt16)
		m_lock = new uint16_t [m_indices.size()];
	else	// ItUInt32
		m_lock = new uint32_t [m_indices.size()];

	return m_lock;
}

void IndexBufferSw::unlock()
{
	if (!m_lock)
		return;

	if (getIndexType() == ItUInt16)
	{
		for (size_t i = 0; i < m_indices.size(); ++i)
			m_indices[i] = reinterpret_cast< uint16_t* >(m_lock)[i];
		delete[] reinterpret_cast< uint16_t* >(m_lock);
	}
	else	// ItUInt32
	{
		for (size_t i = 0; i < m_indices.size(); ++i)
			m_indices[i] = reinterpret_cast< uint32_t* >(m_lock)[i];
		delete[] reinterpret_cast< uint32_t* >(m_lock);
	}

	m_lock = 0;
}

const std::vector< uint32_t >& IndexBufferSw::getIndices() const
{
	return m_indices;
}

	}
}
