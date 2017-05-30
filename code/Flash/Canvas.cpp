/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Bitmap.h"
#include "Flash/Canvas.h"
#include "Flash/Types.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Canvas", Canvas, Object)

Canvas::Canvas()
:	m_cacheTag(allocateCacheTag())
,	m_dirtyTag(0)
,	m_drawing(false)
{
	clear();
}

int32_t Canvas::getCacheTag() const
{
	return m_cacheTag;
}

int32_t Canvas::getDirtyTag() const
{
	return m_dirtyTag;
}

void Canvas::clear()
{
	m_dictionary = Dictionary();
	m_paths.clear();
	m_bounds = Aabb2();
	m_fillStyles.resize(0);
	m_lineStyles.resize(0);
	++m_dirtyTag;
}

void Canvas::beginFill(const Color4f& color)
{
	FillStyle style;
	style.create(color);
	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void Canvas::beginGradientFill(FillStyle::GradientType gradientType, const AlignedVector< FillStyle::ColorRecord >& colorRecords, const Matrix33& gradientMatrix)
{
	FillStyle style;
	style.create(gradientType, colorRecords, gradientMatrix);
	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void Canvas::beginBitmapFill(Bitmap* image, const Matrix33& bitmapMatrix, bool repeat)
{
	uint16_t bitmapId = m_dictionary.addBitmap(image);

	FillStyle style;
	style.create(bitmapId, bitmapMatrix, repeat);

	m_fillStyles.push_back(style);
	m_paths.push_back(Path());
	m_drawing = true;
}

void Canvas::endFill()
{
	if (!m_drawing)
		return;

	uint16_t fillStyle = uint16_t(m_fillStyles.size());

	Path& p = m_paths.back();
	p.end(fillStyle, 0, 0);

	m_drawing = false;
	++m_dirtyTag;
}

void Canvas::moveTo(float x, float y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.moveTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void Canvas::lineTo(float x, float y)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.lineTo(int32_t(x), int32_t(y), Path::CmAbsolute);
	m_bounds.contain(Vector2(x, y));
}

void Canvas::curveTo(float controlX, float controlY, float anchorX, float anchorY)
{
	if (!m_drawing)
		return;

	Path& p = m_paths.back();
	p.quadraticTo(int32_t(controlX), int32_t(controlY), int32_t(anchorX), int32_t(anchorY), Path::CmAbsolute);
	m_bounds.contain(Vector2(controlX, controlY));
	m_bounds.contain(Vector2(anchorX, anchorY));
}

	}
}
