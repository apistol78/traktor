#pragma once

#include "Core/Math/Color4f.h"
#include "Spark/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief Color transform wrapper.
 * \ingroup Spark
 */
class T_DLLCLASS ColorTransform : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Color4f mul;
	Color4f add;

	ColorTransform();

	ColorTransform(const ColorTransform& cxform);

	explicit ColorTransform(const Color4f& mul);

	explicit ColorTransform(const Color4f& mul, const Color4f& add);

	ColorTransform operator * (const ColorTransform& rh) const
	{
		ColorTransform cxtr;
		cxtr.mul = mul * rh.mul;
		cxtr.add = (mul * rh.add + add).saturated();
		return cxtr;
	}

	ColorTransform& operator = (const ColorTransform& rh)
	{
		mul = rh.mul;
		add = rh.add;
		return *this;
	}
};

	}
}

