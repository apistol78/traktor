#ifndef traktor_flash_ColorTransform_H
#define traktor_flash_ColorTransform_H

#include "Core/Math/Color4f.h"
#include "Flash/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Color transform wrapper.
 * \ingroup Flash
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
		cxtr.add = (add * rh.mul + rh.add).saturated();
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

#endif	// traktor_flash_ColorTransform_H
