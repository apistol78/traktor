#pragma once

#include "Spark/Path.h"
#include "Spark/Editor/Svg/SvgShape.h"

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class SvgPathShape : public SvgShape
{
	T_RTTI_CLASS;

public:
	SvgPathShape(const Path& path);

	const Path& getPath() const;

private:
	Path m_path;
};

	}
}

