#ifndef traktor_wbr_Float_H
#define traktor_wbr_Float_H

#include "Webber/ILayout.h"

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
	
class T_DLLCLASS Float : public ILayout
{
	T_RTTI_CLASS;

public:
	virtual void build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const;
};

	}
}

#endif	// traktor_wbr_Float_H
