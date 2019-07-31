#pragma once

#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix33.h"
#include "Spark/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace spark
	{

class ColorTransform;

/*! \brief Shape line style.
 * \ingroup Spark
 */
class T_DLLCLASS LineStyle
{
public:
	LineStyle();

	bool create(const SwfLineStyle* lineStyle);

	void transform(const ColorTransform& cxform);

	const Color4f& getLineColor() const;

	uint16_t getLineWidth() const;

	void serialize(ISerializer& s);

private:
	Color4f m_lineColor;
	uint16_t m_lineWidth;
};

	}
}

