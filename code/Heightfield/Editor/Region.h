#ifndef traktor_hf_Region_H
#define traktor_hf_Region_H

#include "Core/Config.h"

namespace traktor
{
	namespace hf
	{

class Region
{
public:
	int32_t minX;
	int32_t minZ;
	int32_t maxX;
	int32_t maxZ;

	Region()
	:	minX(0)
	,	minZ(0)
	,	maxX(0)
	,	maxZ(0)
	{
	}

	Region(int32_t minX_, int32_t minZ_, int32_t maxX_, int32_t maxZ_)
	:	minX(minX_)
	,	minZ(minZ_)
	,	maxX(maxX_)
	,	maxZ(maxZ_)
	{
	}

	bool empty() const
	{
		return (maxX - minX) * (maxZ - minZ) <= 0;
	}

	void contain(const Region& r)
	{
		if (!empty())
		{
			minX = std::min(minX, r.minX);
			minZ = std::min(minZ, r.minZ);
			maxX = std::max(maxX, r.maxX);
			maxZ = std::max(maxZ, r.maxZ);
		}
		else
		{
			minX = r.minX;
			minZ = r.minZ;
			maxX = r.maxX;
			maxZ = r.maxZ;
		}
	}

	void intersect(const Region& r)
	{
		if (minX >= r.maxX || minZ >= r.maxZ || maxX <= r.minX || maxZ <= r.minZ)
		{
			minX =
			minZ =
			maxX =
			maxZ = 0;
		}
		else
		{
			minX = std::max(minX, r.minX);
			minZ = std::max(minZ, r.minZ);
			maxX = std::min(maxX, r.maxX);
			maxZ = std::min(maxZ, r.maxZ);
		}
	}
};

	}
}

#endif	// traktor_hf_Region_H
