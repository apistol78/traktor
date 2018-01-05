#ifndef traktor_wbr_Splitter_H
#define traktor_wbr_Splitter_H

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

class T_DLLCLASS Splitter : public ILayout
{
	T_RTTI_CLASS;

public:
	enum Direction
	{
		Horizontal = 0,
		Vertical = 1
	};

	Splitter(Direction direction);

	virtual void build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const;

private:
	Direction m_direction;
};

	}
}

#endif	// traktor_wbr_Splitter_H
