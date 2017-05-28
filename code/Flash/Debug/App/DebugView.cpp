/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/TString.h"
#include "Drawing/Image.h"
#include "Drawing/Raster.h"
#include "Flash/FlashShape.h"
#include "Flash/Debug/ButtonInstanceDebugInfo.h"
#include "Flash/Debug/EditInstanceDebugInfo.h"
#include "Flash/Debug/MorphShapeInstanceDebugInfo.h"
#include "Flash/Debug/ShapeInstanceDebugInfo.h"
#include "Flash/Debug/SpriteInstanceDebugInfo.h"
#include "Flash/Debug/TextInstanceDebugInfo.h"
#include "Flash/Debug/App/DebugView.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const static Matrix33 c_textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

//bool childOf(const InstanceDebugInfo* of, const InstanceDebugInfo* child)
//{
//	if (of == child)
//		return true;
//
//	if (const SpriteInstanceDebugInfo* spriteDebugInfo = dynamic_type_cast< const SpriteInstanceDebugInfo* >(of))
//	{
//		for (auto i : spriteDebugInfo->getChildrenDebugInfo())
//		{
//			if (childOf(i, child))
//				return true;
//		}
//	}
//
//	return false;
//}

void flattenDebugInfo(const RefArray< InstanceDebugInfo >& debugInfo, bool visibleOnly, bool highlightOnly, const InstanceDebugInfo* highlight, RefArray< InstanceDebugInfo >& outDebugInfo)
{
	for (auto i : debugInfo)
	{
		bool visible = true;

		if (visibleOnly)
			visible = i->getVisible();

		if (!visible)
			continue;

		if (!highlightOnly || highlight == i)
			outDebugInfo.push_back(i);

		if (highlightOnly && highlight == i)
			highlightOnly = false;

		if (const SpriteInstanceDebugInfo* spriteDebugInfo = dynamic_type_cast< const SpriteInstanceDebugInfo* >(i))
			flattenDebugInfo(spriteDebugInfo->getChildrenDebugInfo(), visibleOnly, highlightOnly, highlight, outDebugInfo);
	}
}

		}

bool DebugView::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsAccelerated | ui::WsDoubleBuffer))
		return false;

	m_bitmapPivot = new ui::StyleBitmap(L"Flash.Pivot");
	m_highlightOnly = false;
	m_visibleOnly = true;
	m_outline = true;
	m_showMasks = false;
	m_offset = ui::Point(0, 0);
	m_scale = 1.0f;
	m_mousePosition = Vector2::zero();

	addEventHandler< ui::PaintEvent >(this, &DebugView::eventPaint);
	addEventHandler< ui::MouseButtonDownEvent >(this, &DebugView::eventMouseDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &DebugView::eventMouseUp);
	addEventHandler< ui::MouseMoveEvent >(this, &DebugView::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &DebugView::eventMouseWheel);

	update();
	return true;
}

void DebugView::setDebugInfo(const FrameDebugInfo* debugInfo)
{
	m_debugInfo = debugInfo;
	m_shapeCache.clear();
}

void DebugView::setHighlight(const InstanceDebugInfo* instance)
{
	m_highlightInstance = instance;
}

void DebugView::setHighlightOnly(bool highlightOnly)
{
	m_highlightOnly = highlightOnly;
}

void DebugView::setVisibleOnly(bool visibleOnly)
{
	m_visibleOnly = visibleOnly;
}

void DebugView::setOutline(bool outline)
{
	m_outline = outline;
}

void DebugView::setShowMasks(bool showMasks)
{
	m_showMasks = showMasks;
}

const Vector2& DebugView::getMousePosition() const
{
	return m_mousePosition;
}

void DebugView::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	ui::Rect innerRect = getInnerRect();

	canvas.setBackground(Color4ub(40, 40, 40, 255));
	canvas.fillRect(innerRect);

	if (m_debugInfo)
	{
		ui::Point mousePosition = ui::Widget::getMousePosition();

		int32_t outputWidth = innerRect.getWidth();
		int32_t outputHeight = innerRect.getHeight();

		int32_t targetWidth = m_debugInfo->getViewWidth() * m_scale;
		int32_t targetHeight = m_debugInfo->getViewHeight() * m_scale;

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

		// Outside frame
		canvas.setForeground(Color4ub(255, 255, 0, 64));

		int32_t dx = ui::scaleBySystemDPI(16);
		int32_t ox = m_offset.x % dx;
		int32_t oy = m_offset.y % dx;

		for (int32_t x = 0; x < max(outputWidth, outputHeight) * 2 + dx; x += dx)
			canvas.drawLine(x + ox - dx, oy - dx, ox - dx, x + oy - dx);

		canvas.setBackground(Color4ub(40, 40, 40, 255));
		canvas.fillRect(targetRect);

		int32_t mx = mousePosition.x - targetRect.left;
		int32_t my = mousePosition.y - targetRect.top;
		m_mousePosition = rasterTransform.inverse() * Vector2(mx, my);

		RefArray< InstanceDebugInfo > instances;
		flattenDebugInfo(m_debugInfo->getInstances(), m_visibleOnly, m_highlightOnly, m_highlightInstance, instances);

		for (auto instance : instances)
		{
			Vector2 extents[4];
			instance->getBounds().getExtents(extents);

			Vector2 f[4] =
			{
				instance->getGlobalTransform() * extents[0],
				instance->getGlobalTransform() * extents[1],
				instance->getGlobalTransform() * extents[2],
				instance->getGlobalTransform() * extents[3]
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

			Aabb2 globalBounds = instance->getGlobalTransform() * instance->getBounds();
			Aabb2 targetBounds = rasterTransform * globalBounds;
			ui::Rect globalRect(
				targetRect.getTopLeft() + ui::Size(targetBounds.mn.x, targetBounds.mn.y),
				targetRect.getTopLeft() + ui::Size(targetBounds.mx.x, targetBounds.mx.y)
			);

			// Override with selected instance.
			bool inside = bool(m_highlightInstance == instance);

			// Don't draw non visible instances (unless they are selected).
			ColorTransform cxform = instance->getColorTransform();
			if (!inside)
			{
				bool alphaVisible = bool(cxform.mul.getAlpha() + cxform.add.getAlpha() > FUZZY_EPSILON);
				if (!alphaVisible)
				{
					if (m_visibleOnly)
						continue;
					else
					{
						// Adjust alpha on color transform to ensure shape is visible.
						cxform.mul.setAlpha(Scalar(0.5f));
						cxform.add.setAlpha(Scalar(0.5f));
					}
				}
			}

			if (const ButtonInstanceDebugInfo* buttonInstance = dynamic_type_cast< const ButtonInstanceDebugInfo* >(instance))
			{
				if (m_outline)
				{
					canvas.setBackground(inside ? Color4ub(80, 255, 255, 20) : Color4ub(200, 255, 255, 10));
					canvas.setForeground(Color4ub(255, 255, 255, 100));
					canvas.fillPolygon(pnts, 4);
					canvas.drawPolygon(pnts, 4);
					canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
					canvas.drawText(pnts[0], mbstows(instance->getName()));
				}
			}
			else if (const EditInstanceDebugInfo* editInstance = dynamic_type_cast< const EditInstanceDebugInfo* >(instance))
			{
				if (m_outline)
				{
					canvas.setBackground(inside ? Color4ub(80, 80, 255, 20) : Color4ub(200, 200, 255, 10));
					canvas.setForeground(Color4ub(255, 255, 255, 100));
					canvas.fillPolygon(pnts, 4);
					canvas.drawPolygon(pnts, 4);
					canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
					canvas.drawText(pnts[0], mbstows(instance->getName()));
				}

				canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
				switch (editInstance->getTextAlign())
				{
				case StaRight:
					canvas.drawText(globalRect, editInstance->getText(), ui::AnRight, ui::AnCenter);
					break;

				case StaCenter:
					canvas.drawText(globalRect, editInstance->getText(), ui::AnCenter, ui::AnCenter);
					break;

				default:
					canvas.drawText(globalRect, editInstance->getText(), ui::AnLeft, ui::AnCenter);
					break;
				}
			}
			else if (const MorphShapeInstanceDebugInfo* morphShapeInstance = dynamic_type_cast< const MorphShapeInstanceDebugInfo* >(instance))
			{
				if (m_showMasks || !morphShapeInstance->getMask())
				{
					if (m_outline)
					{
						canvas.setBackground(inside ? Color4ub(255, 255, 255, 20) : Color4ub(255, 255, 255, 10));
						canvas.setForeground(Color4ub(255, 255, 255, 100));
						canvas.fillPolygon(pnts, 4);
						canvas.drawPolygon(pnts, 4);
						canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
						canvas.drawText(pnts[0], mbstows(instance->getName()));
					}
				}
			}
			else if (const ShapeInstanceDebugInfo* shapeInstance = dynamic_type_cast< const ShapeInstanceDebugInfo* >(instance))
			{
				if (m_showMasks || !shapeInstance->getMask())
				{
					canvas.setBackground(inside ? Color4ub(255, 255, 255, 20) : Color4ub(255, 255, 255, 20));
					canvas.setForeground(Color4ub(255, 255, 255, 100));

					const FlashShape* shape = shapeInstance->getShape();
					if (shape)
					{
						ShapeCache& sc = m_shapeCache[(void*)shapeInstance];

						if (!sc.image)
						{
							Aabb2 shapeBounds = instance->getGlobalTransform() * shape->getShapeBounds();

							const Color4f& cxm = cxform.mul;
							const Color4f& cxa = cxform.add;

							Vector2 s[] =
							{
								rasterTransform * shapeBounds.mn,
								rasterTransform * shapeBounds.mx
							};

							int32_t width = int32_t(s[1].x - s[0].x);
							int32_t height = int32_t(s[1].y - s[0].y);

							float frameWidth = shapeBounds.getSize().x;
							float frameHeight = shapeBounds.getSize().y;

							Matrix33 rasterTransform =
								traktor::scale(width, height) *
								traktor::scale(1.0f / frameWidth, 1.0f / frameHeight) *
								traktor::translate(-shapeBounds.mn.x, -shapeBounds.mn.y) *
								instance->getGlobalTransform();

							sc.image = new drawing::Image(
								drawing::PixelFormat::getA8R8G8B8(),
								width,
								height
							);
							sc.image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

							Ref< drawing::Raster > raster = new drawing::Raster(sc.image);

							float strokeScale = std::min(width / frameWidth, height / frameHeight);

							const AlignedVector< FlashFillStyle >& fillStyles = shape->getFillStyles();
							const AlignedVector< FlashLineStyle >& lineStyles = shape->getLineStyles();
							int32_t lineStyleBase = 0;

							// Convert all styles used by this shape.
							raster->clearStyles();

							for (uint32_t i = 0; i < uint32_t(fillStyles.size()); ++i)
							{
								const FlashFillStyle& style = fillStyles[i];
								const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();

								if (colorRecords.size() == 1)
								{
									const Color4f& c = colorRecords[0].color;
									raster->defineSolidStyle(c * cxm + cxa);
								}
								else if (colorRecords.size() > 1)
								{
									switch (style.getGradientType())
									{
									case FlashFillStyle::GtLinear:
										{
											AlignedVector< std::pair< Color4f, float > > colors;
											for (uint32_t j = 0; j < colorRecords.size(); ++j)
											{
												const Color4f& c = colorRecords[j].color;
												colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
											}
											raster->defineLinearGradientStyle(
												c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
												colors
											);
										}
										break;

									case FlashFillStyle::GtRadial:
										{
											AlignedVector< std::pair< Color4f, float > > colors;
											for (uint32_t j = 0; j < colorRecords.size(); ++j)
											{
												const Color4f& c = colorRecords[j].color;
												colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
											}
											raster->defineRadialGradientStyle(
												c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
												colors
											);
										}
										break;

									default:
										raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
										break;
									}
								}
								else
								{
									raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
								}
							}
							lineStyleBase = int32_t(fillStyles.size());
							for (uint32_t i = 0; i < uint32_t(lineStyles.size()); ++i)
							{
								const FlashLineStyle& style = lineStyles[i];
								raster->defineSolidStyle(style.getLineColor() * cxm + cxa);
							}

							// Rasterize every path in shape.
							const AlignedVector< Path >& paths = shape->getPaths();
							for (AlignedVector< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
							{
								const AlignedVector< Vector2 >& points = i->getPoints();
								const AlignedVector< SubPath >& subPaths = i->getSubPaths();
								for (AlignedVector< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
								{
									int32_t fs0 = j->fillStyle0 - 1;
									int32_t fs1 = j->fillStyle1 - 1;
									int32_t ls = j->lineStyle - 1;

									T_ASSERT (fs0 >= 0 || fs1 >= 0 || ls >= 0);

									raster->clear();

									const AlignedVector< SubPathSegment >& segments = j->segments;
									for (AlignedVector< SubPathSegment >::const_iterator k = segments.begin(); k != segments.end(); ++k)
									{
										raster->moveTo(rasterTransform * points[k->pointsOffset]);
										if (k->type == SpgtLinear)
											raster->lineTo(rasterTransform * points[k->pointsOffset + 1]);
										else
											raster->quadricTo(rasterTransform * points[k->pointsOffset + 1], rasterTransform * points[k->pointsOffset + 2]);
									}

									if (fs0 >= 0 || fs1 >= 0)
										raster->fill(fs0, fs1, drawing::Raster::FrNonZero);

									if (ls >= 0)
										raster->stroke(lineStyleBase + ls, lineStyles[ls].getLineWidth() * strokeScale, drawing::Raster::ScSquare);
								}

								raster->submit();
							}

							sc.bitmap = new ui::Bitmap(sc.image);
						}

						int32_t width = sc.image->getWidth();
						int32_t height = sc.image->getHeight();

						canvas.drawBitmap(globalRect.getTopLeft(), ui::Point(0, 0), ui::Size(width, height), sc.bitmap);
					}
					else
					{
						if (m_outline)
						{
							canvas.fillPolygon(pnts, 4);
							canvas.drawPolygon(pnts, 4);
						}
					}
				}
			}
			else if (const SpriteInstanceDebugInfo* spriteInstance = dynamic_type_cast< const SpriteInstanceDebugInfo* >(instance))
			{
				if (m_outline)
				{
					StringOutputStream ss;
					ss << spriteInstance->getCurrentFrame() << L"/" << spriteInstance->getFrames();

					canvas.setBackground(inside ? Color4ub(80, 255, 80, 20) : Color4ub(200, 255, 200, 10));
					canvas.setForeground(Color4ub(255, 255, 255, 100));
					canvas.fillPolygon(pnts, 4);
					canvas.drawPolygon(pnts, 4);
					canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
					canvas.drawText(pnts[0], mbstows(instance->getName()));
					canvas.drawText(pnts[0] + ui::Size(0, ui::scaleBySystemDPI(12)), ss.str());
				}
			}
			else if (const TextInstanceDebugInfo* textInstance = dynamic_type_cast< const TextInstanceDebugInfo* >(instance))
			{
				if (m_outline)
				{
					canvas.setBackground(inside ? Color4ub(255, 80, 80, 20) : Color4ub(255, 200, 200, 10));
					canvas.setForeground(Color4ub(255, 255, 255, 100));
					canvas.fillPolygon(pnts, 4);
					canvas.drawPolygon(pnts, 4);
					canvas.setForeground(Color4ub(255, 255, 255, inside ? 200 : 100));
					canvas.drawText(pnts[0], mbstows(instance->getName()));
				}
			}

			if (inside)
			{
				Vector2 p = rasterTransform * (instance->getGlobalTransform() * Vector2(0.0f, 0.0f));
				ui::Point pivot = targetRect.getTopLeft() + ui::Size(p.x, p.y);
				ui::Size sz = m_bitmapPivot->getSize();
				canvas.drawBitmap(pivot + ui::Size(-sz.cx / 2, -sz.cy / 2), ui::Point(0, 0), sz, m_bitmapPivot);
			}
		}

		// Frame
		canvas.setForeground(Color4ub(255, 255, 0, 128));
		canvas.drawLine(targetRect.left, innerRect.top, targetRect.left, innerRect.bottom);
		canvas.drawLine(targetRect.right, innerRect.top, targetRect.right, innerRect.bottom);
		canvas.drawLine(innerRect.left, targetRect.top, innerRect.right, targetRect.top);
		canvas.drawLine(innerRect.left, targetRect.bottom, innerRect.right, targetRect.bottom);
	}
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

void DebugView::eventMouseWheel(ui::MouseWheelEvent* event)
{
	m_scale = clamp(m_scale + event->getRotation() * 0.1f, 0.1f, 10.0f);
	m_shapeCache.clear();
	update();
}

	}
}
