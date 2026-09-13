/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/CropHeightfieldDialog.h"

#include "Core/Containers/StaticVector.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Filters/GrayscaleFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Image.h"
#include "I18N/Text.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/StyleSheet.h"
#include "Ui/TableLayout.h"

namespace traktor::hf
{
namespace
{

//! Largest preview image kept in the dialog; it's resampled for each repaint.
const int32_t c_maxPreviewSize = 512;

//! Limit of the crop rectangle, same as the largest heightfield which can be created.
const int32_t c_maxGridSize = 16 * 1024;

//! Span of the crop rectangle, in grid units, along with the source it's sampled from.
struct Span
{
	int32_t destination0;
	int32_t destination1;
	int32_t source0;
	int32_t source1;
};

/*! Split a crop range into spans of the source range.
 *
 * The crop operation clamp samples coordinates, thus parts of the crop rectangle
 * outside the source repeat the outermost row or column of the source.
 */
void buildSpans(int32_t cropBegin, int32_t cropEnd, int32_t sourceSize, StaticVector< Span, 3 >& outSpans)
{
	if (cropBegin < 0)
		outSpans.push_back({ cropBegin, min(cropEnd, 0), 0, 1 });

	const int32_t inside0 = max(cropBegin, 0);
	const int32_t inside1 = min(cropEnd, sourceSize);
	if (inside1 > inside0)
		outSpans.push_back({ inside0, inside1, inside0, inside1 });

	if (cropEnd > sourceSize)
		outSpans.push_back({ max(cropBegin, sourceSize), cropEnd, sourceSize - 1, sourceSize });
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.CropHeightfieldDialog", CropHeightfieldDialog, ui::ConfigDialog)

bool CropHeightfieldDialog::create(ui::Widget* parent, int32_t sourceSize, const drawing::Image* sourceImage)
{
	if (sourceSize <= 0 || sourceImage == nullptr)
		return false;

	if (!ui::ConfigDialog::create(
			parent,
			i18n::Text(L"CROP_HEIGHTFIELD_WIZARDTOOL_TITLE"),
			520_ut,
			600_ut,
			ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
			new ui::FloodLayout()))
		return false;

	m_sourceSize = sourceSize;

	Ref< drawing::Image > previewImage = sourceImage->clone();
	if (previewImage->getWidth() > c_maxPreviewSize)
	{
		const drawing::ScaleFilter scaleFilter(
			c_maxPreviewSize,
			c_maxPreviewSize,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear);
		previewImage->apply(&scaleFilter);
	}
	m_bitmapSource = new ui::Bitmap(previewImage);

	// Desaturated and darkened copy, drawn outside of the crop rectangle so
	// the cropped region stands out.
	Ref< drawing::Image > maskedImage = previewImage->clone();
	const drawing::GrayscaleFilter grayscaleFilter;
	const drawing::BrightnessContrastFilter darkenFilter(0.0f, 0.4f);
	maskedImage->apply(&grayscaleFilter);
	maskedImage->apply(&darkenFilter);
	m_bitmapSourceMasked = new ui::Bitmap(maskedImage);

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(this, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*", 8_ut, 8_ut)))
		return false;

	Ref< ui::Container > containerFields = new ui::Container();
	if (!containerFields->create(containerInner, ui::WsNone, new ui::TableLayout(L"*,100%,*,100%,*,100%", L"*", 0_ut, 4_ut)))
		return false;

	Ref< ui::Static > staticLeft = new ui::Static();
	staticLeft->create(containerFields, L"Left");

	m_editLeft = new ui::Edit();
	m_editLeft->create(containerFields, L"0", ui::WsNone, new ui::NumericEditValidator(false, -c_maxGridSize, c_maxGridSize));
	m_editLeft->addEventHandler< ui::ContentChangeEvent >(this, &CropHeightfieldDialog::eventEditChange);

	Ref< ui::Static > staticTop = new ui::Static();
	staticTop->create(containerFields, L"Top");

	m_editTop = new ui::Edit();
	m_editTop->create(containerFields, L"0", ui::WsNone, new ui::NumericEditValidator(false, -c_maxGridSize, c_maxGridSize));
	m_editTop->addEventHandler< ui::ContentChangeEvent >(this, &CropHeightfieldDialog::eventEditChange);

	Ref< ui::Static > staticSize = new ui::Static();
	staticSize->create(containerFields, L"Size");

	m_editSize = new ui::Edit();
	m_editSize->create(containerFields, toString(m_sourceSize), ui::WsNone, new ui::NumericEditValidator(false, 1, c_maxGridSize));
	m_editSize->addEventHandler< ui::ContentChangeEvent >(this, &CropHeightfieldDialog::eventEditChange);

	m_previewControl = new ui::Widget();
	if (!m_previewControl->create(containerInner, ui::WsDoubleBuffer))
		return false;

	m_previewControl->addEventHandler< ui::PaintEvent >(this, &CropHeightfieldDialog::eventPreviewPaint);

	m_staticInfo = new ui::Static();
	m_staticInfo->create(containerInner, L"");

	updateInfo();
	return true;
}

int32_t CropHeightfieldDialog::getCropLeft() const
{
	return parseString< int32_t >(m_editLeft->getText(), 0);
}

int32_t CropHeightfieldDialog::getCropTop() const
{
	return parseString< int32_t >(m_editTop->getText(), 0);
}

int32_t CropHeightfieldDialog::getCropSize() const
{
	return parseString< int32_t >(m_editSize->getText(), 0);
}

void CropHeightfieldDialog::updateInfo()
{
	const int32_t cropLeft = getCropLeft();
	const int32_t cropTop = getCropTop();
	const int32_t cropSize = max(getCropSize(), 1);

	StringOutputStream ss;
	ss << L"Source " << m_sourceSize << L" x " << m_sourceSize << L", cropped " << cropSize << L" x " << cropSize;

	if (cropLeft < 0 || cropTop < 0 || cropLeft + cropSize > m_sourceSize || cropTop + cropSize > m_sourceSize)
		ss << L"; outside of source, edge is repeated.";

	m_staticInfo->setText(ss.str());
	m_staticInfo->update();
}

void CropHeightfieldDialog::eventEditChange(ui::ContentChangeEvent* event)
{
	updateInfo();
	m_previewControl->update();
}

void CropHeightfieldDialog::eventPreviewPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	const ui::StyleSheet* ss = m_previewControl->getStyleSheet();
	const ui::Rect rcInner = m_previewControl->getInnerRect();

	canvas.setBackground(ss->getColor(m_previewControl, L"background-color"));
	canvas.fillRect(rcInner);

	if (rcInner.getWidth() <= 0 || rcInner.getHeight() <= 0)
		return;

	const int32_t cropLeft = getCropLeft();
	const int32_t cropTop = getCropTop();
	const int32_t cropSize = max(getCropSize(), 1);

	// Region of the grid shown in the preview; both the source and the crop rectangle
	// are visible, and it's kept square as to not distort the heightfield.
	const float viewLeft = (float)min(cropLeft, 0);
	const float viewTop = (float)min(cropTop, 0);
	const float viewRight = (float)max(cropLeft + cropSize, m_sourceSize);
	const float viewBottom = (float)max(cropTop + cropSize, m_sourceSize);
	const float viewSize = max(viewRight - viewLeft, viewBottom - viewTop) * 1.05f;

	const float scale = min(rcInner.getWidth(), rcInner.getHeight()) / viewSize;
	const float offsetX = rcInner.left + rcInner.getWidth() / 2.0f - (viewLeft + viewRight) * scale / 2.0f;
	const float offsetY = rcInner.top + rcInner.getHeight() / 2.0f - (viewTop + viewBottom) * scale / 2.0f;

	const auto toClient = [&](int32_t gridX, int32_t gridY) {
		return ui::Point(
			(int32_t)(offsetX + gridX * scale + 0.5f),
			(int32_t)(offsetY + gridY * scale + 0.5f));
	};

	// Image pixels per grid unit; the preview image is scaled down for large heightfields.
	const ui::Size imageSize = m_bitmapSource->getSize(m_previewControl);
	const float imageScale = (float)imageSize.cx / (float)m_sourceSize;

	const auto drawBlock = [&](ui::Bitmap* bitmap, const Span& spanX, const Span& spanY) {
		const ui::Point dstTopLeft = toClient(spanX.destination0, spanY.destination0);
		const ui::Point dstBottomRight = toClient(spanX.destination1, spanY.destination1);

		// Spans thinner than a pixel are dropped; scaling to nothing puts the canvas in an error state.
		const ui::Size dstSize(dstBottomRight.x - dstTopLeft.x, dstBottomRight.y - dstTopLeft.y);
		if (dstSize.cx <= 0 || dstSize.cy <= 0)
			return;

		const ui::Point srcTopLeft(
			clamp((int32_t)(spanX.source0 * imageScale), 0, imageSize.cx - 1),
			clamp((int32_t)(spanY.source0 * imageScale), 0, imageSize.cy - 1));
		const ui::Size srcSize(
			clamp((int32_t)((spanX.source1 - spanX.source0) * imageScale), 1, imageSize.cx - srcTopLeft.x),
			clamp((int32_t)((spanY.source1 - spanY.source0) * imageScale), 1, imageSize.cy - srcTopLeft.y));
		canvas.drawBitmap(
			dstTopLeft,
			dstSize,
			srcTopLeft,
			srcSize,
			bitmap,
			ui::BlendMode::Opaque,
			(srcSize.cx > 1 && srcSize.cy > 1) ? ui::Filter::Linear : ui::Filter::Nearest);
	};

	// Entire source, masked since only the crop rectangle is kept.
	const Span sourceSpan = { 0, m_sourceSize, 0, m_sourceSize };
	drawBlock(m_bitmapSourceMasked, sourceSpan, sourceSpan);

	// Cropped region, as it will be sampled from the source.
	StaticVector< Span, 3 > spansX, spansY;
	buildSpans(cropLeft, cropLeft + cropSize, m_sourceSize, spansX);
	buildSpans(cropTop, cropTop + cropSize, m_sourceSize, spansY);

	for (const auto& spanY : spansY)
		for (const auto& spanX : spansX)
			drawBlock(m_bitmapSource, spanX, spanY);

	const ui::Rect rcSource(toClient(0, 0), toClient(m_sourceSize, m_sourceSize));
	canvas.setForeground(ss->getColor(m_previewControl, L"border-color"));
	canvas.drawRect(rcSource);

	// Crop rectangle; outlined twice as to be visible on both bright and dark terrain.
	const ui::Rect rcCrop(toClient(cropLeft, cropTop), toClient(cropLeft + cropSize, cropTop + cropSize));
	canvas.setForeground(ss->getColor(m_previewControl, L"background-color"));
	canvas.drawRect(rcCrop.inflate(1, 1));
	canvas.setForeground(ss->getColor(m_previewControl, L"color"));
	canvas.drawRect(rcCrop);

	event->consume();
}

}
