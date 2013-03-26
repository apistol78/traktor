#ifndef traktor_model_LwPolygon_H
#define traktor_model_LwPolygon_H

#include <vector>
#include "Core/Config.h"

namespace traktor
{

class IStream;

	namespace model
	{

class LwSurface;

struct LwPolygon
{
	uint32_t type;
	uint16_t flags;
	std::vector< uint32_t> indices;
	uint16_t surfaceTag;
	uint32_t surfaceIndex;
	const LwSurface* surface;
	uint16_t smoothGroup;

	LwPolygon();

	bool readLWO2(IStream* stream);

	bool readLWOB(IStream* stream);
};

	}
}

#endif	// traktor_model_LwPolygon_H
