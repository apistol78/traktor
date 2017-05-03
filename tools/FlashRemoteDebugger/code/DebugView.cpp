/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Log/Log.h>
#include <Core/Misc/TString.h>
#include <Flash/Debug/EditInstanceDebugInfo.h>
#include <Flash/Debug/SpriteInstanceDebugInfo.h>
#include <Flash/Debug/TextInstanceDebugInfo.h>
#include "DebugView.h"

using namespace traktor;

bool DebugView::create(ui::Widget* parent, net::BidirectionalObjectTransport* transport)
{
	if (!ui::Widget::create(parent, ui::WsAccelerated | ui::WsDoubleBuffer))
		return false;

	m_transport = transport;

	addEventHandler< ui::PaintEvent >(this, &DebugView::eventPaint);
	addEventHandler< ui::TimerEvent >(this, &DebugView::eventTimer);
	startTimer(100);

	update();
	return true;
}

Vector2 norm(const Aabb2& into, const Vector2& pt)
{
	return Vector2(
		(pt.x - into.mn.x) / (into.mx.x - into.mn.x),
		(pt.y - into.mn.y) / (into.mx.y - into.mn.y)
	);
}

void DebugView::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	ui::Rect innerRect = getInnerRect();

	canvas.setBackground(Color4ub(255, 255, 255, 255));
	canvas.fillRect(innerRect);

	if (m_debugInfo)
	{
		const RefArray< flash::InstanceDebugInfo >& instances = m_debugInfo->getInstances();
		for (auto instance : instances)
		{
			Vector2 extents[4];
			instance->getBounds().getExtents(extents);

			Vector2 f[4] =
			{
				instance->getTransform() * extents[0],
				instance->getTransform() * extents[1],
				instance->getTransform() * extents[2],
				instance->getTransform() * extents[3]
			};

			Aabb2 frameBounds = m_debugInfo->getFrameBounds();

			Vector2 s[4] =
			{
				norm(frameBounds, f[0]),
				norm(frameBounds, f[1]),
				norm(frameBounds, f[2]),
				norm(frameBounds, f[3])
			};

			int32_t w = innerRect.getWidth() / 2;
			int32_t h = innerRect.getHeight() / 2;

			ui::Point pnts[4] =
			{
				ui::Point(s[0].x * w + w / 2, s[0].y * h + h / 2),
				ui::Point(s[1].x * w + w / 2, s[1].y * h + h / 2),
				ui::Point(s[2].x * w + w / 2, s[2].y * h + h / 2),
				ui::Point(s[3].x * w + w / 2, s[3].y * h + h / 2)
			};

			canvas.setForeground(Color4ub(255, 255, 0, 255));
			canvas.drawRect(ui::Rect(
				w / 2, h / 2,
				w + w / 2, h + h / 2
			));

			canvas.setForeground(Color4ub(0, 0, 0, 255));
			canvas.drawPolygon(
				pnts,
				4
			);

			canvas.drawText(pnts[0], mbstows(instance->getName()));

			if (const flash::EditInstanceDebugInfo* editInstance = dynamic_type_cast< const flash::EditInstanceDebugInfo* >(instance))
			{
				canvas.drawText(pnts[0] + ui::Size(0, 10), editInstance->getText());
			}
			else if (const flash::SpriteInstanceDebugInfo* spriteInstance = dynamic_type_cast< const flash::SpriteInstanceDebugInfo* >(instance))
			{
			}
			else if (const flash::TextInstanceDebugInfo* textInstance = dynamic_type_cast< const flash::TextInstanceDebugInfo* >(instance))
			{
			}
		}
	}

	event->consume();
}

void DebugView::eventTimer(ui::TimerEvent* event)
{
	if (m_transport->recv(0, m_debugInfo) != net::BidirectionalObjectTransport::RtSuccess)
		return;

	m_transport->flush< flash::PostFrameDebugInfo >();

	update(0, false);
}
