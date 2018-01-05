#ifndef traktor_wbr_ILayout_H
#define traktor_wbr_ILayout_H

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace html
	{

class Element;

	}

	namespace wbr
	{

class Widget;

class T_DLLCLASS ILayout : public Object
{
	T_RTTI_CLASS;

public:
	virtual void build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const = 0;
};

	}
}

#endif	// traktor_wbr_ILayout_H
