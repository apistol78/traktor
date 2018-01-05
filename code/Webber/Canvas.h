#ifndef traktor_wbr_Canvas_H
#define traktor_wbr_Canvas_H

#include "Webber/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace wbr
	{
	
class T_DLLCLASS Canvas : public Widget
{
	T_RTTI_CLASS;

public:
	Canvas();

	virtual void build(html::Element* parent) const override;

	virtual void consume(int32_t senderId, int32_t action) override;
};

	}
}

#endif	// traktor_wbr_Canvas_H
