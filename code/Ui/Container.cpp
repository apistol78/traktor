#include "Core/Log/Log.h"
#include "Ui/Container.h"
#include "Ui/Application.h"
#include "Ui/Layout.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Container", Container, Widget)

bool Container::create(Widget* parent, int style, Layout* layout)
{
	m_layout = layout;

	if (!m_widget)
	{
		IContainer* container = Application::getInstance()->getWidgetFactory()->createContainer(this);
		if (!container)
		{
			log::error << L"Failed to create native widget peer (Container)" << Endl;
			return false;
		}

		if (!container->create(parent->getIWidget(), style))
		{
			container->destroy();
			return false;
		}

		m_widget = container;
	}

	addEventHandler< SizeEvent >(this, &Container::eventSize);
	
	return Widget::create(parent);
}

void Container::fit()
{
	if (m_layout)
	{
		// First fit child containers.
		for (Ref< Widget > child = getFirstChild(); child; child = child->getNextSibling())
		{
			if (is_a< Container >(child))
				static_cast< Container* >(child.ptr())->fit();
		}
	
		// Use layout to calculate size of container.
		Size inner = getInnerRect().getSize();
		Size bounds;
		if (m_layout->fit(this, inner, bounds))
		{
			Rect outer = getRect();
			Size nc = outer.getSize() - inner;
			outer.setSize(bounds + nc);
			setRect(outer);
		}
	}
}

void Container::update(const Rect* rc, bool immediate)
{
	if (m_layout)
		m_layout->update(this);

	Widget::update(rc, immediate);
}

Size Container::getMinimumSize() const
{
	return Widget::getMinimumSize();
}

Size Container::getPreferedSize() const
{
	if (m_layout)
	{
		// Calculate hint size, use largest child's preferred size as hint.
		Size inner = getInnerRect().getSize();
		for (Ref< Widget > child = getFirstChild(); child; child = child->getNextSibling())
		{
			Size childSize = child->getPreferedSize();
			inner.cx = std::max< int >(inner.cx, childSize.cx);
			inner.cy = std::max< int >(inner.cy, childSize.cy);
		}

		// Use layout to fit hinted size.
		Size bounds;
		if (m_layout->fit(const_cast< Container* >(this), inner, bounds))
			return bounds;
	}
	return Widget::getPreferedSize();
}

Size Container::getMaximumSize() const
{
	return Widget::getMaximumSize();
}

Ref< Layout > Container::getLayout() const
{
	return m_layout;
}

void Container::setLayout(Layout* layout)
{
	m_layout = layout;
}

void Container::eventSize(SizeEvent* event)
{
	update(0, false);
}

	}
}
