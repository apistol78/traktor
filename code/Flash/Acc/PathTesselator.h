#ifndef traktor_flash_PathTesselator_H
#define traktor_flash_PathTesselator_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/Polygon.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Path;
struct SubPath;
struct SubPathSegment;

/*! \brief Tessellate path into linear segments.
 * \ingroup Flash
 */
class T_DLLCLASS PathTesselator : public Object
{
	T_RTTI_CLASS;

public:
	void tesselate(const Path& path, AlignedVector< Segment >& outSegments) const;

private:
	void tesselateLinearSegment(const Path& path, const SubPath& subPath, const SubPathSegment& segment, AlignedVector< Segment >& outSegments) const;

	void tesselateQuadraticSegment(const Path& path, const SubPath& subPath, const SubPathSegment& segment, AlignedVector< Segment >& outSegments) const;
};

	}
}

#endif	// traktor_flash_PathTesselator_H
