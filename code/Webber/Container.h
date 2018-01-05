#ifndef traktor_wbr_Container_H
#define traktor_wbr_Container_H

#include "Core/RefArray.h"
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

class ILayout;
	
class T_DLLCLASS Container : public Widget
{
	T_RTTI_CLASS;

public:
	void setLayout(ILayout* layout);

	template < typename WidgetType >
	WidgetType* addChild(WidgetType* child) { m_children.push_back(child); return child; }

	void removeChild(Widget* child);

	const RefArray< Widget >& getChildren() const;

	virtual void build(html::Element* parent) const override;

	virtual void consume(int32_t senderId, int32_t action) override;

private:
	Ref< ILayout > m_layout;
	RefArray< Widget > m_children;
};

	}
}

#endif	// traktor_wbr_Container_H
