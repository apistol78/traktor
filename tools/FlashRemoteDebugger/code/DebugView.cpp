/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/StringOutputStream.h>
#include <Core/Log/Log.h>
#include <Core/Math/Const.h>
#include <Core/Math/Format.h>
#include <Core/Misc/TString.h>
#include <Flash/Debug/ButtonInstanceDebugInfo.h>
#include <Flash/Debug/EditInstanceDebugInfo.h>
#include <Flash/Debug/MorphShapeInstanceDebugInfo.h>
#include <Flash/Debug/ShapeInstanceDebugInfo.h>
#include <Flash/Debug/SpriteInstanceDebugInfo.h>
#include <Flash/Debug/TextInstanceDebugInfo.h>
#include <Ui/Application.h>
#include "DebugView.h"

using namespace traktor;

namespace
{

void flattenDebugInfo(const RefArray< flash::InstanceDebugInfo >& debugInfo, RefArray< flash::InstanceDebugInfo >& outDebugInfo)
{
	for (auto i : debugInfo)
	{
		outDebugInfo.push_back(i);
		if (const flash::SpriteInstanceDebugInfo* spriteDebugInfo = dynamic_type_cast< flash::SpriteInstanceDebugInfo* >(i))
			flattenDebugInfo(spriteDebugInfo->getChildrenDebugInfo(), outDebugInfo);
	}
}

}

bool DebugView::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsAccelerated | ui::WsDoubleBuffer))
		return false;

	m_offset = ui::Point(0, 0);
	m_counter = 0;

	addEventHandler< ui::PaintEvent >(this, &DebugView::eventPaint);
	addEventHandler< ui::MouseButtonDownEvent >(this, &DebugView::eventMouseDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &DebugView::eventMouseUp);
	addEventHandler< ui::MouseMoveEvent >(this, &DebugView::eventMouseMove);

	update();
	return true;
}

void DebugView::setDebugInfo(const flash::PostFrameDebugInfo* debugInfo)
{
	m_debugInfo = debugInfo;
	m_counter++;
	update();
}

void DebugView::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	ui::Rect innerRect = getInnerRect();

	canvas.setBackground(Color4ub(40, 40, 40, 255));
	canvas.fillRect(innerRect);

	if (m_debugInfo)
	{
		ui::Point mousePosition = getMousePosition();

		int32_t outputWidth = innerRect.getWidth();
		int32_t outputHeight = innerRect.getHeight();

		int32_t targetWidth = m_debugInfo->getViewWidth();
		int32_t targetHeight = m_debugInfo->getViewHeight();

		float frameWidth = m_debugInfo->getFrameBounds().mx.x;
		float frameHeight = m_debugInfo->getFrameBounds().mx.y;

		Matrix33 rasterTransform =
			traktor::scale(targetWidth / frameWidth, targetHeight / frameHeight) *
			traktor::scale(m_debugInfo->getStageTransform().z(), m_debugInfo->getStageTransform().w()) *
			traktor::translate(m_debugInfo->getStageTransform().x(), m_debugInfo->getStageTransform().y());

		ui::Rect targetRect(
			ui::Point(
				m_offset.x + (outputWidth - targetWidth) / 2,
				m_offset.y + (outputHeight - targetHeight) / 2
			),
			ui::Size(
				targetWidth,
				targetHeight
			)
		);

		int32_t mx = mousePosition.x - targetRect.left;
		int32_t my = mousePosition.y - targetRect.top;
		Vector2 t0 = rasterTransform.inverse() * Vector2(mx, my);

		int32_t ty = ui::scaleBySystemDPI(12);
		int32_t insideCount = 0;


		// Print mouse position in stage coordinates.
		{
			StringOutputStream ss;
			ss << L"Mouse " << int32_t(t0.x) << L", " << int32_t(t0.y) << L" - Frames " << m_counter;
			canvas.setForeground(Color4ub(255, 255, 255, 200));
			canvas.drawText(ui::Point(ty, ty), ss.str());
		}

		RefArray< flash::InstanceDebugInfo > instances;
		flattenDebugInfo(m_debugInfo->getInstances(), instances);

		for (auto instance : instances)
		{
			if (instance->getColorTransform().mul.getAlpha() + instance->getColorTransform().add.getAlpha() <= FUZZY_EPSILON)
				continue;

			Vector2 extents[4];
			instance->getBounds().getExtents(extents);

			Vector2 f[4] =
			{
				instance->getTransform() * extents[0],
				instance->getTransform() * extents[1],
				instance->getTransform() * extents[2],
				instance->getTransform() * extents[3]
			};

			Vector2 s[4] =
			{
				rasterTransform * f[0],
				rasterTransform * f[1],
				rasterTransform * f[2],
				rasterTransform * f[3]
			};

			ui::Point pnts[4] =
			{
				targetRect.getTopLeft() + ui::Size(s[0].x, s[0].y),
				targetRect.getTopLeft() + ui::Size(s[1].x, s[1].y),
				targetRect.getTopLeft() + ui::Size(s[2].x, s[2].y),
				targetRect.getTopLeft() + ui::Size(s[3].x, s[3].y)
			};

			// Check if mouse is inside instance's bounds.
			Vector2 t1 = instance->getTransform().inverse() * t0;
			bool inside = instance->getBounds().inside(t1);

			// 
			if (inside)
			{
				float x = instance->getTransform().e13;
				float y = instance->getTransform().e23;

				StringOutputStream ss;

				if (is_a< flash::ButtonInstanceDebugInfo >(instance))
					ss << L"BT: ";
				else if (is_a< flash::EditInstanceDebugInfo >(instance))
					ss << L"ED: ";
				else if (is_a< flash::MorphShapeInstanceDebugInfo >(instance))
					ss << L"MO: ";
				else if (is_a< flash::ShapeInstanceDebugInfo >(instance))
					ss << L"SH: ";
				else if (is_a< flash::SpriteInstanceDebugInfo >(instance))
					ss << L"MC: ";
				else if (is_a< flash::TextInstanceDebugInfo >(instance))
					ss << L"TX: ";
				else
					ss << L"??: ";

				if (!instance->getName().empty())
					ss << mbstows(instance->getName());
				else
					ss << L"<unnamed>";

				ss << L" : " << int32_t(x) << L", " << int32_t(y);

				auto cx = instance->getColorTransform();
				ss << L", +[" << cx.add << L"], *[" << cx.mul << L"]";
				
				canvas.setForeground(Color4ub(255, 255, 255, 200));
				canvas.drawText(ui::Point(ty, ty * (3 + insideCount)), ss.str());

				++insideCount;
			}

			if (const flash::ButtonInstanceDebugInfo* buttonInstance = dynamic_type_cast< const flash::ButtonInstanceDebugInfo* >(instance))
			{
				canvas.setBackground(inside ? Color4ub(80, 255, 255, 20) : Color4ub(200, 255, 255, 20));
				canvas.setForeground(Color4ub(255, 255, 255, 100));
				canvas.fillPolygon(pnts, 4);
				canvas.drawPolygon(pnts, 4);
				canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
				canvas.drawText(pnts[0], mbstows(instance->getName()));
			}
			else if (const flash::EditInstanceDebugInfo* editInstance = dynamic_type_cast< const flash::EditInstanceDebugInfo* >(instance))
			{
				canvas.setBackground(inside ? Color4ub(80, 80, 255, 20) : Color4ub(200, 200, 255, 20));
				canvas.setForeground(Color4ub(255, 255, 255, 100));
				canvas.fillPolygon(pnts, 4);
				canvas.drawPolygon(pnts, 4);
				canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
				canvas.drawText(pnts[0], mbstows(instance->getName()));
				canvas.drawText(pnts[0] + ui::Size(0, ty), editInstance->getText());
			}
			else if (const flash::MorphShapeInstanceDebugInfo* morphShapeInstance = dynamic_type_cast< const flash::MorphShapeInstanceDebugInfo* >(instance))
			{
				canvas.setBackground(inside ? Color4ub(255, 255, 255, 20) : Color4ub(255, 255, 255, 20));
				canvas.setForeground(Color4ub(255, 255, 255, 100));
				canvas.fillPolygon(pnts, 4);
				canvas.drawPolygon(pnts, 4);
				canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
				canvas.drawText(pnts[0], mbstows(instance->getName()));
			}
			else if (const flash::ShapeInstanceDebugInfo* shapeInstance = dynamic_type_cast< const flash::ShapeInstanceDebugInfo* >(instance))
			{
				canvas.setBackground(inside ? Color4ub(255, 255, 255, 20) : Color4ub(255, 255, 255, 20));
				canvas.setForeground(Color4ub(255, 255, 255, 100));
				canvas.fillPolygon(pnts, 4);
				canvas.drawPolygon(pnts, 4);
				canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
				canvas.drawText(pnts[0], mbstows(instance->getName()));
			}
			else if (const flash::SpriteInstanceDebugInfo* spriteInstance = dynamic_type_cast< const flash::SpriteInstanceDebugInfo* >(instance))
			{
				StringOutputStream ss;
				ss << spriteInstance->getCurrentFrame() << L"/" << spriteInstance->getFrames();

				canvas.setBackground(inside ? Color4ub(80, 255, 80, 20) : Color4ub(200, 255, 200, 20));
				canvas.setForeground(Color4ub(255, 255, 255, 100));
				canvas.fillPolygon(pnts, 4);
				canvas.drawPolygon(pnts, 4);
				canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
				canvas.drawText(pnts[0], mbstows(instance->getName()));
				canvas.drawText(pnts[0] + ui::Size(0, ty), ss.str());
			}
			else if (const flash::TextInstanceDebugInfo* textInstance = dynamic_type_cast< const flash::TextInstanceDebugInfo* >(instance))
			{
				canvas.setBackground(inside ? Color4ub(255, 80, 80, 20) : Color4ub(255, 200, 200, 20));
				canvas.setForeground(Color4ub(255, 255, 255, 100));
				canvas.fillPolygon(pnts, 4);
				canvas.drawPolygon(pnts, 4);
				canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
				canvas.drawText(pnts[0], mbstows(instance->getName()));
			}
		}

		// Frame
		canvas.setForeground(Color4ub(255, 255, 0, 128));
		canvas.drawLine(targetRect.left, innerRect.top, targetRect.left, innerRect.bottom);
		canvas.drawLine(targetRect.right, innerRect.top, targetRect.right, innerRect.bottom);
		canvas.drawLine(innerRect.left, targetRect.top, innerRect.right, targetRect.top);
		canvas.drawLine(innerRect.left, targetRect.bottom, innerRect.right, targetRect.bottom);
	}

	event->consume();
}

void DebugView::eventMouseDown(ui::MouseButtonDownEvent* event)
{
	m_mouseLast = event->getPosition();
	setCapture();
}

void DebugView::eventMouseUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void DebugView::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (hasCapture())
	{
		m_offset.x += event->getPosition().x - m_mouseLast.x;
		m_offset.y += event->getPosition().y - m_mouseLast.y;
		m_mouseLast = event->getPosition();
	}
	update();
}
