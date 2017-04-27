/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Dock.h"
#include "Ui/DockPane.h"
#include "Ui/StyleSheet.h"
#include "Ui/ToolForm.h"
#include "Ui/FloodLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Image.h"

// Resources
#include "Resources/DockBottom.h"
#include "Resources/DockLeft.h"
#include "Resources/DockRight.h"
#include "Resources/DockTop.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_hintSize = 32 + 29 + 32;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Dock", Dock, Widget)

bool Dock::create(Widget* parent)
{
	if (!ui::Widget::create(parent, WsAccelerated))
		return false;

	m_hint = new ToolForm();
	m_hint->create(this, L"Hint", 0, 0, WsNone);
	m_hint->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	const int xy[] = { 0, 32, 32 + 29, 32 + 29 + 32 };
	Point p[] =
	{
		Point(xy[1], xy[0]),
		Point(xy[2], xy[0]),
		Point(xy[2], xy[1]),
		Point(xy[3], xy[1]),
		Point(xy[3], xy[2]),
		Point(xy[2], xy[2]),
		Point(xy[2], xy[3]),
		Point(xy[1], xy[3]),
		Point(xy[1], xy[2]),
		Point(xy[0], xy[2]),
		Point(xy[0], xy[1]),
		Point(xy[1], xy[1])
	};
	m_hint->setOutline(p, sizeof(p) / sizeof(Point));

	m_hintLeft = new ui::Image();
	m_hintLeft->create(m_hint, Bitmap::load(c_ResourceDockLeft, sizeof(c_ResourceDockLeft), L"png"), false);
	m_hintLeft->setRect(Rect(xy[0], xy[1], xy[0] + 32, xy[1] + 29));
	m_hintLeft->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_hintRight = new ui::Image();
	m_hintRight->create(m_hint, Bitmap::load(c_ResourceDockRight, sizeof(c_ResourceDockRight), L"png"), false);
	m_hintRight->setRect(Rect(xy[2], xy[1], xy[2] + 32, xy[1] + 29));
	m_hintRight->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_hintTop = new ui::Image();
	m_hintTop->create(m_hint, Bitmap::load(c_ResourceDockTop, sizeof(c_ResourceDockTop), L"png"), false);
	m_hintTop->setRect(Rect(xy[1], xy[0], xy[1] + 29, xy[0] + 32));
	m_hintTop->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_hintBottom = new ui::Image();
	m_hintBottom->create(m_hint, Bitmap::load(c_ResourceDockBottom, sizeof(c_ResourceDockBottom), L"png"), false);
	m_hintBottom->setRect(Rect(xy[1], xy[2], xy[1] + 29, xy[2] + 32));
	m_hintBottom->addEventHandler< MouseButtonUpEvent >(this, &Dock::eventHintButtonUp);

	m_pane = new DockPane(this, (DockPane*)0);

	addEventHandler< SizeEvent >(this, &Dock::eventSize);
	addEventHandler< MouseButtonDownEvent >(this, &Dock::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &Dock::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &Dock::eventMouseMove);
	addEventHandler< MouseDoubleClickEvent >(this, &Dock::eventDoubleClick);
	addEventHandler< PaintEvent >(this, &Dock::eventPaint);

	return true;
}

void Dock::destroy()
{
	if (m_hint)
	{
		m_hint->destroy();
		m_hint = 0;
	}
	ui::Widget::destroy();
}

DockPane* Dock::getPane()
{
	return m_pane;
}

void Dock::dumpLayout()
{
	if (m_pane)
		m_pane->dump();
}

void Dock::update(const Rect* rc, bool immediate)
{
	Rect innerRect = getInnerRect();

	// Update chains, calculate deferred child widget rectangles.
	std::vector< WidgetRect > widgetRects;
	m_pane->update(innerRect, widgetRects);

	// Update child widgets.
	setChildRects(widgetRects);

	// Continue updating widget.
	Widget::update(rc, immediate);
}

void Dock::eventSize(SizeEvent* event)
{
	update();
}

void Dock::eventButtonDown(MouseButtonDownEvent* event)
{
	Point position = event->getPosition();
	Ref< DockPane > pane;
		
	if ((pane = m_pane->getPaneFromPosition(position)) != 0)
	{
		if (pane->hitGripperClose(position))
		{
			if (pane->m_widget)
				pane->m_widget->hide();
			update();
		}

		if (pane->m_widget)
			pane->m_widget->setFocus();

		event->consume();
		return;
	}

	if ((pane = m_pane->getSplitterFromPosition(position)) != 0)
	{
		if (pane->hitSplitter(position))
		{
			m_splittingPane = pane;
			setCursor(pane->m_vertical ? CrSizeNS : CrSizeWE);
			setCapture();
		}
		event->consume();
		return;
	}

	event->consume();
}

void Dock::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();
	resetCursor();
}

void Dock::eventMouseMove(MouseMoveEvent* event)
{
	Point position = event->getPosition();
	Cursor cursor = CrArrow;

	if (!hasCapture())
	{
		Ref< DockPane > pane = m_pane->getSplitterFromPosition(position);
		if (
			pane &&
			pane->hitSplitter(position)
		)
		{
			cursor = pane->m_vertical ? CrSizeNS : CrSizeWE;
		}
	}
	else
	{
		m_splittingPane->setSplitterPosition(position);
		cursor = m_splittingPane->m_vertical ? CrSizeNS : CrSizeWE;
		update();
	}

	setCursor(cursor);
}

void Dock::eventDoubleClick(MouseDoubleClickEvent* event)
{
	Point position = event->getPosition();

	Ref< DockPane > pane = m_pane->getPaneFromPosition(position);
	if (!pane)
		return;

	if (
		pane->hitGripper(position) &&
		!pane->hitGripperClose(position)
	)
	{
		T_ASSERT (pane->m_detachable);

		Ref< Widget > widget = pane->m_widget;

		pane->detach();

		// Create floating form.
		Size preferedSize = widget ? widget->getPreferedSize() : Size(100, 100);

		Ref< ToolForm > form = new ToolForm();

		form->create(
			this,
			widget ? widget->getText() : L"",
			preferedSize.cx,
			preferedSize.cy,
			ToolForm::WsDefault,
			new FloodLayout()
		);

		form->addEventHandler< MoveEvent >(this, &Dock::eventFormMove);
		form->addEventHandler< NcMouseButtonUpEvent >(this, &Dock::eventFormNcButtonUp);

		// Reparent widget into floating form.
		if (widget)
			widget->setParent(form);

		form->setData(L"WIDGET", widget);
		form->update();
		form->show();

		update();
	}
}

void Dock::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect innerRect = getInnerRect();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(innerRect);

	m_pane->draw(canvas);

	event->consume();
}

void Dock::eventFormMove(MoveEvent* event)
{
	Point position = getMousePosition();

	Ref< DockPane > pane = m_pane->getPaneFromPosition(position);
	if (pane)
	{
		// Is hint form already visible for this pane?
		if (pane == m_hintDockPane && m_hint->isVisible(false))
		{
			// Bring hint form to foreground.
			m_hint->raise();
			return;
		}
		else
			m_hint->hide();

		Rect rc = pane->getPaneRect();

		m_hint->setRect(Rect(
			rc.left + (rc.getWidth() - c_hintSize) / 2,
			rc.top + (rc.getHeight() - c_hintSize) / 2,
			rc.left + (rc.getWidth() - c_hintSize) / 2 + c_hintSize,
			rc.top + (rc.getHeight() - c_hintSize) / 2 + c_hintSize
		));

		m_hint->show();
		m_hint->raise();

		m_hintDockPane = pane;
		m_hintDockForm = checked_type_cast< ToolForm* >(event->getSender());
	}
	else
	{
		m_hint->hide();
		m_hintDockPane = 0;
		m_hintDockForm = 0;
	}
}

void Dock::eventFormNcButtonUp(NcMouseButtonUpEvent* event)
{
	Point position = screenToClient(event->getPosition());

	// Ensure hint form isn't visible.
	if (m_hint)
	{
		m_hint->hide();
		m_hintDockPane = 0;
		m_hintDockForm = 0;
	}

	Ref< DockPane > pane = m_pane->getPaneFromPosition(position);
	if (pane)
	{
		Ref< ToolForm > form = checked_type_cast< ToolForm* >(event->getSender());
		Ref< Widget > widget = form->getData< Widget >(L"WIDGET");

		// Reparent widget back to dock.
		if (widget)
			widget->setParent(this);

		form->destroy();
		form = 0;

		// Calculate docking direction.
		int dx = position.x - pane->m_rect.getCenter().x;
		int dy = position.y - pane->m_rect.getCenter().y;

		DockPane::Direction direction;
		if (traktor::abs(dx) > traktor::abs(dy))
			direction = dx > 0 ? DockPane::DrEast : DockPane::DrWest;
		else
			direction = dy > 0 ? DockPane::DrSouth : DockPane::DrNorth;

		pane->dock(
			widget,
			true,
			direction,
			100
		);

		update();
	}
}

void Dock::eventHintButtonUp(MouseButtonUpEvent* event)
{
	m_hint->hide();

	Ref< Image > hintImage = dynamic_type_cast< Image* >(event->getSender());
	if (!hintImage)
		return;

	T_ASSERT (m_hintDockForm);
	T_ASSERT (m_hintDockPane);

	Ref< Widget > widget = m_hintDockForm->getData< Widget >(L"WIDGET");

	// Reparent widget back to dock.
	if (widget)
		widget->setParent(this);

	m_hintDockForm->destroy();
	m_hintDockForm = 0;

	// Calculate docking direction.
	DockPane::Direction direction;
	if (hintImage == m_hintLeft)
		direction = DockPane::DrWest;
	else if (hintImage == m_hintRight)
		direction = DockPane::DrEast;
	else if (hintImage == m_hintTop)
		direction = DockPane::DrNorth;
	else/* if (hintImage == m_hintBottom)*/
		direction = DockPane::DrSouth;

	m_hintDockPane->dock(
		widget,
		true,
		direction,
		100
	);

	update();
}

	}
}
