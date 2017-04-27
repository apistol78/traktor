/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Spray/PointSet.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSet", PointSet, Object)

PointSet::PointSet()
{
}

void PointSet::add(const Point& point)
{
	m_points.push_back(point);
}

bool PointSet::read(IStream* stream)
{
	Reader r(stream);
	
	uint32_t pointCount;
	r >> pointCount;
	
	m_points.resize(pointCount);
	for (uint32_t i = 0; i < pointCount; ++i)
	{
		float tmp[3+3+4];
		if (r.read(tmp, 3+3+4, sizeof(float)) != (3+3+4) * sizeof(float))
			return false;
		m_points[i].position = Vector4::loadUnaligned(&tmp[0]).xyz1();
		m_points[i].normal = Vector4::loadUnaligned(&tmp[3]).xyz1();
		m_points[i].color = Vector4::loadUnaligned(&tmp[6]);
	}
		
	return true;
}

bool PointSet::write(IStream* stream) const
{
	Writer w(stream);
	
	w << uint32_t(m_points.size());
	
	for (uint32_t i = 0; i < m_points.size(); ++i)
	{
		float tmp[3+3+4];
		m_points[i].position.storeUnaligned(&tmp[0]);
		m_points[i].normal.storeUnaligned(&tmp[3]);
		m_points[i].color.storeUnaligned(&tmp[6]);
		if (w.write(tmp, 3+3+4, sizeof(float)) != (3+3+4) * sizeof(float))
			return false;
	}
		
	return true;
}

	}
}
