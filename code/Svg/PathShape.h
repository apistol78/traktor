#pragma once

#include "Svg/Path.h"
#include "Svg/Shape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace svg
	{

/*! SVG path shape.
 * \ingroup SVG
 */
class T_DLLCLASS PathShape : public Shape
{
	T_RTTI_CLASS;

public:
	explicit PathShape(const Path& path);

	const Path& getPath() const;

private:
	Path m_path;
};

	}
}
