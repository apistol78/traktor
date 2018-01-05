#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/Canvas.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Canvas", Canvas, Widget)

Canvas::Canvas()
{
}

void Canvas::build(html::Element* parent) const
{
	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_canvas_container");

	auto ecanvas = new html::Element(L"canvas");
	ecanvas->setAttribute(L"id", L"_" + toString(getId()));
	ediv->addChild(ecanvas);

	parent->addChild(ediv);
}

void Canvas::consume(int32_t senderId, int32_t action)
{
}

	}
}
