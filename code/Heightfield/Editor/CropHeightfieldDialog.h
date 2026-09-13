/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Ui/ConfigDialog.h"

namespace traktor::drawing
{

class Image;

}

namespace traktor::ui
{

class Bitmap;
class ContentChangeEvent;
class Edit;
class PaintEvent;
class Static;
class Widget;

}

namespace traktor::hf
{

/*! Crop heightfield dialog.
 *
 * Enter the crop rectangle along with a preview of the
 * result of the crop operation.
 */
class CropHeightfieldDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, int32_t sourceSize, const drawing::Image* sourceImage);

	int32_t getCropLeft() const;

	int32_t getCropTop() const;

	int32_t getCropSize() const;

private:
	int32_t m_sourceSize = 0;
	Ref< ui::Bitmap > m_bitmapSource;
	Ref< ui::Bitmap > m_bitmapSourceMasked;
	Ref< ui::Edit > m_editLeft;
	Ref< ui::Edit > m_editTop;
	Ref< ui::Edit > m_editSize;
	Ref< ui::Widget > m_previewControl;
	Ref< ui::Static > m_staticInfo;

	void updateInfo();

	void eventEditChange(ui::ContentChangeEvent* event);

	void eventPreviewPaint(ui::PaintEvent* event);
};

}
