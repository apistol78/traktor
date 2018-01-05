#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/TabContainer.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.TabContainer", TabContainer, Container)

void TabContainer::build(html::Element* parent) const
{
	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_tabContainer");
	ediv->setAttribute(L"id", L"_" + toString(getId()));
	parent->addChild(ediv);

	auto ediv2 = new html::Element(L"div");
	ediv2->setAttribute(L"class", L"wtk_tabHeaders");
	ediv->addChild(ediv2);

	{
		auto eth = new html::Element(L"div");
		eth->setAttribute(L"class", L"wtk_tabHeader");
		eth->addChild(new html::Text(L"First"));
		ediv2->addChild(eth);
	}

	{
		auto eth = new html::Element(L"div");
		eth->setAttribute(L"class", L"wtk_tabHeader");
		eth->addChild(new html::Text(L"Second"));
		ediv2->addChild(eth);
	}

	auto ediv3 = new html::Element(L"div");
	ediv3->setAttribute(L"class", L"wtk_tabPages");
	ediv->addChild(ediv3);

	for (auto c : getChildren())
		c->build(ediv3);
}

void TabContainer::consume(int32_t senderId, int32_t action)
{
}

	}
}
