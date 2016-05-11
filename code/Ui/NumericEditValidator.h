#ifndef traktor_ui_NumericEditValidator_H
#define traktor_ui_NumericEditValidator_H

#include <limits>
#include "Ui/EditValidator.h"

#undef min
#undef max

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Numeric text edit validator.
 * \ingroup UI
 */
class T_DLLCLASS NumericEditValidator : public EditValidator
{
	T_RTTI_CLASS;

public:
	NumericEditValidator(
		bool floatPoint,
		float min = -std::numeric_limits< float >::max(),
		float max = std::numeric_limits< float >::max(),
		int decimals = 8
	);

	virtual EditValidator::Result validate(const std::wstring& text) const;

private:
	bool m_floatPoint;
	float m_min;
	float m_max;
	int m_decimals;
};

	}
}

#endif	// traktor_ui_NumericEditValidator_H
