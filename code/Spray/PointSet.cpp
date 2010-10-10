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
	if (r.read(&m_points[0], pointCount, sizeof(Point)) != pointCount * sizeof(Point))
		return false;
		
	return true;
}

bool PointSet::write(IStream* stream) const
{
	Writer w(stream);
	
	w << uint32_t(m_points.size());
	
	if (w.write(&m_points[0], m_points.size(), sizeof(Point)) != m_points.size() * sizeof(Point))
		return false;
		
	return true;
}

	}
}
