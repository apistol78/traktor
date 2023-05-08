/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Drawing/Filters/EncodeRGBM.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/NoiseFilter.h"
#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Filters/PremultiplyAlphaFilter.h"
#include "Drawing/Filters/SphereMapFilter.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Render/Editor/Texture/TextureControl.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor::render
{
	namespace
	{

ui::Size operator * (const ui::Size& sz, float scale)
{
	return ui::Size(
		(int32_t)(sz.cx * scale),
		(int32_t)(sz.cy * scale)
	);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureControl", TextureControl, ui::Widget)

bool TextureControl::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsAccelerated))
		return false;

	addEventHandler< ui::MouseButtonDownEvent >(this, &TextureControl::eventMouseDown);
	addEventHandler< ui::MouseButtonUpEvent >(this, &TextureControl::eventMouseUp);
	addEventHandler< ui::MouseMoveEvent >(this, &TextureControl::eventMouseMove);
	addEventHandler< ui::MouseWheelEvent >(this, &TextureControl::eventMouseWheel);
	addEventHandler< ui::PaintEvent >(this, &TextureControl::eventPaint);
	return true;
}

ui::Size TextureControl::getMinimumSize() const
{
	return m_imageSource ? m_imageSource->getSize() : ui::Size(0, 0);
}

ui::Size TextureControl::getPreferredSize(const ui::Size& hint) const
{
	return m_imageSource ? m_imageSource->getSize() : ui::Size(0, 0);
}

bool TextureControl::setImage(drawing::Image* image, const TextureOutput& output)
{
	if (image != nullptr)
	{
		float gamma = 2.2f;
		bool sRGB = true;

		if (image->getImageInfo() != nullptr)
		{
			gamma = image->getImageInfo()->getGamma();
			sRGB = (bool)(std::abs(gamma - 2.2f) <= 0.1f);
		}

		// Create source image.
		Ref< drawing::Image > imageSource = image->clone();
		if (!sRGB)
		{
			// Ensure image is in sRGB since that's what UI expects.
			const drawing::GammaFilter gammaFilter(gamma, 2.2f);
			imageSource->apply(&gammaFilter);
		}
		m_imageSource = new ui::Bitmap(imageSource);

		// Create output image.
		Ref< drawing::Image > imageOutput = image->clone();
		imageOutput->convert(drawing::PixelFormat::getR8G8B8A8());

		// Discard alpha.
		if (output.m_ignoreAlpha)
		{
			const drawing::SwizzleFilter swizzleFilter(L"RGB1");
			imageOutput->apply(&swizzleFilter);
		}

		// Generate alpha, maximum of color channels.
		if (output.m_generateAlpha)
		{
			Color4f tmp;
			for (int32_t y = 0; y < imageOutput->getHeight(); ++y)
			{
				for (int32_t x = 0; x < imageOutput->getWidth(); ++x)
				{
					imageOutput->getPixelUnsafe(x, y, tmp);

					Scalar alpha = 0.0_simd;
					alpha = max(alpha, tmp.getRed());
					alpha = max(alpha, tmp.getGreen());
					alpha = max(alpha, tmp.getBlue());
					tmp.setAlpha(alpha);

					imageOutput->setPixelUnsafe(x, y, tmp);
				}
			}
		}

		// Invert alpha channel.
		if (output.m_invertAlpha)
		{
			const drawing::TransformFilter invertAlphaFilter(Color4f(1.0f, 1.0f, 1.0f, -1.0f), Color4f(0.0f, 0.0f, 0.0f, 1.0f));
			imageOutput->apply(&invertAlphaFilter);
		}

		// Dilate image from alpha channel.
		if (output.m_dilateImage)
		{
			const drawing::DilateFilter dilateFilter(8);
			imageOutput->apply(&dilateFilter);
		}

		// Convert image into linear space to ensure all filters are applied in linear space.
		if (sRGB && !output.m_linearGamma)
		{
			const drawing::GammaFilter gammaFilter(gamma, 1.0f);
			imageOutput->apply(&gammaFilter);
		}

		if (output.m_flipX || output.m_flipY)
		{
			const drawing::MirrorFilter mirrorFilter(output.m_flipX, output.m_flipY);
			imageOutput->apply(&mirrorFilter);
		}
		if (output.m_generateSphereMap)
		{
			const drawing::SphereMapFilter sphereMapFilter;
			imageOutput->apply(&sphereMapFilter);
		}
		if (output.m_noiseStrength > 0.0f)
		{
			const drawing::NoiseFilter noiseFilter(output.m_noiseStrength);
			imageOutput->apply(&noiseFilter);
		}
		if (output.m_premultiplyAlpha)
		{
			const drawing::PremultiplyAlphaFilter preAlphaFilter;
			imageOutput->apply(&preAlphaFilter);
		}
		if (output.m_generateNormalMap)
		{
			const drawing::NormalMapFilter filter(output.m_scaleDepth);
			imageOutput->apply(&filter);
		}
		if (output.m_inverseNormalMapX || output.m_inverseNormalMapY)
		{
			const drawing::TransformFilter transformFilter(
				Color4f(
					output.m_inverseNormalMapX ? -1.0f : 1.0f,
					output.m_inverseNormalMapY ? -1.0f : 1.0f,
					1.0f,
					1.0f
				),
				Color4f(
					output.m_inverseNormalMapX ? 1.0f : 0.0f,
					output.m_inverseNormalMapY ? 1.0f : 0.0f,
					0.0f,
					0.0f
				)
			);
			imageOutput->apply(&transformFilter);
		}

		// Convert to sRGB last since that's what UI expect.
		{
			const drawing::GammaFilter gammaFilter(1.0f, 2.2f);
			imageOutput->apply(&gammaFilter);
		}

		for (int32_t y = 0; y < imageOutput->getHeight(); ++y)
		{
			for (int32_t x = 0; x < imageOutput->getWidth(); ++x)
			{
				Color4f clr;
				imageOutput->getPixelUnsafe(x, y, clr);

				Color4f ptrn = (((x / 16) ^ (y / 16)) & 1) ? Color4f(1.0f, 1.0f, 1.0f, 1.0f) : Color4f(0.2f, 0.2f, 0.2f, 1.0f);

				clr = clr * clr.getAlpha() + ptrn * (1.0_simd - clr.getAlpha());
				imageOutput->setPixelUnsafe(x, y, clr);
			}
		}

		m_imageOutput = new ui::Bitmap(imageOutput);
	}
	else
	{
		m_imageSource = nullptr;
		m_imageOutput = nullptr;
	}

	// Update view.
	update();
	return true;
}

void TextureControl::eventMouseDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtLeft)
		return;

	m_moveOrigin = event->getPosition();
	m_moveOriginOffset = m_offset;

	setCapture();
}

void TextureControl::eventMouseUp(ui::MouseButtonUpEvent* event)
{
	releaseCapture();
}

void TextureControl::eventMouseMove(ui::MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	ui::Size offset = event->getPosition() - m_moveOrigin;
	m_offset = m_moveOriginOffset + offset;

	update();
}

void TextureControl::eventMouseWheel(ui::MouseWheelEvent* event)
{
	const int32_t rotation = event->getRotation();

	m_scale += rotation * 0.2f;
	m_scale = std::max(m_scale, 0.2f);

	update();
}

void TextureControl::eventPaint(ui::PaintEvent* event)
{
	const ui::StyleSheet* ss = getStyleSheet();
	ui::Canvas& canvas = event->getCanvas();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(getInnerRect());

	if (m_imageSource)
	{
		const ui::Size clientSize = getInnerRect().getSize();
		const ui::Size imageSize = m_imageSource->getSize();

		const ui::Point center =
		{
			(clientSize.cx - imageSize.cx) / 2,
			(clientSize.cy - imageSize.cy) / 2
		};

		canvas.setClipRect(ui::Rect(0, 0, clientSize.cx / 2, clientSize.cy));
		canvas.drawBitmap(
			center + m_offset,
			m_imageSource->getSize() * m_scale,
			ui::Point(0, 0),
			m_imageSource->getSize(),
			m_imageSource,
			ui::BlendMode::Opaque,
			ui::Filter::Nearest
		);

		canvas.setClipRect(ui::Rect(clientSize.cx / 2, 0, clientSize.cx, clientSize.cy));
		canvas.drawBitmap(
			center + m_offset,
			m_imageOutput->getSize() * m_scale,
			ui::Point(0, 0),
			m_imageOutput->getSize(),
			m_imageOutput,
			ui::BlendMode::Opaque,
			ui::Filter::Nearest
		);

		canvas.resetClipRect();
	}

	event->consume();
}

}
