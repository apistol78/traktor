#pragma once

#include "Core/Config.h"

namespace traktor
{
	namespace ui
	{

/*! Size
 * \ingroup UI
 */
class Size
{
public:
	int32_t cx;
	int32_t cy;

	inline Size();

	inline Size(int32_t x, int32_t y);

	inline Size(const Size& size);

	inline Size operator - () const;

	inline Size operator + (const Size& r) const;

	inline Size& operator += (const Size& r);

	inline Size operator - (const Size& r) const;

	inline Size& operator -= (const Size& r);

	inline bool operator == (const Size& r) const;

	inline bool operator != (const Size& r) const;
};

	}
}

#include "Ui/Size.inl"

