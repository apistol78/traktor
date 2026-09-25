/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetComposer.h"

#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Image.h"
#include "Drawing/ImageInfo.h"
#include "Drawing/PixelFormat.h"
#include "Render/Editor/Texture/TrimSheetSlab.h"

#include <algorithm>
#include <cmath>

namespace traktor::render
{
namespace
{

//! Largest sheet dimension, also largest dimension of a scaled source image.
const int32_t c_maxSize = 8192;

int32_t wrap(int32_t v, int32_t n)
{
	const int32_t r = v % n;
	return (r >= 0) ? r : r + n;
}

float normalized(uint8_t v)
{
	return v * (1.0f / 255.0f);
}

float normalized(float v)
{
	return v;
}

/*! Load source image and convert into a format which is fast to sample.
 *
 * Sources with at most 8 bits per channel are kept as 8 bit RGBA, others
 * as RGBA F32; both have channels in RGBA order in memory.
 */
Ref< drawing::Image > loadSource(const Path& filePath, float scale, bool sRGB)
{
	Ref< drawing::Image > image = drawing::Image::load(filePath);
	if (!image)
		return nullptr;

	const drawing::PixelFormat pf = image->getPixelFormat();
	const bool highPrecision = pf.isFloatPoint() || pf.getRedBits() > 8 || pf.getGreenBits() > 8 || pf.getBlueBits() > 8 || pf.getAlphaBits() > 8;
	const bool luminance = pf.getRedBits() > 0 && pf.getGreenBits() == 0 && pf.getBlueBits() == 0;
	const bool alpha = pf.getAlphaBits() > 0;
	const float gamma = image->getImageInfo() ? image->getImageInfo()->getGamma() : 2.2f;

	image->convert(highPrecision ? drawing::PixelFormat::getRGBAF32() : drawing::PixelFormat::getA8B8G8R8());

	// Expand luminance into all color channels; also ensure alpha is opaque if source has none.
	if (luminance || !alpha)
	{
		const int32_t count = image->getWidth() * image->getHeight();
		if (highPrecision)
		{
			float* p = static_cast< float* >(image->getData());
			for (int32_t i = 0; i < count; ++i, p += 4)
			{
				if (luminance)
					p[1] = p[2] = p[0];
				if (!alpha)
					p[3] = 1.0f;
			}
		}
		else
		{
			uint8_t* p = static_cast< uint8_t* >(image->getData());
			for (int32_t i = 0; i < count; ++i, p += 4)
			{
				if (luminance)
					p[1] = p[2] = p[0];
				if (!alpha)
					p[3] = 255;
			}
		}
	}

	// Albedo is composed in sRGB, other layers contain data which is copied as-is.
	if (sRGB && gamma > FUZZY_EPSILON && std::abs(gamma - 2.2f) > 0.1f)
	{
		const drawing::GammaFilter gammaFilter(gamma, 2.2f);
		image->apply(&gammaFilter);
	}

	if (std::abs(scale - 1.0f) > FUZZY_EPSILON)
	{
		// Never scale beyond largest sheet dimension; it would be clipped anyway.
		const float maxScale = float(c_maxSize) / std::max(image->getWidth(), image->getHeight());
		if (scale > maxScale)
		{
			log::warning << L"Trim sheet source image \"" << filePath.getPathName() << L"\" scale clamped to " << maxScale << L"." << Endl;
			scale = maxScale;
		}

		const int32_t width = std::max((int32_t)(image->getWidth() * scale + 0.5f), 1);
		const int32_t height = std::max((int32_t)(image->getHeight() * scale + 0.5f), 1);
		const drawing::ScaleFilter scaleFilter(width, height, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
		image->apply(&scaleFilter);
	}

	return image;
}

/*! Place image into region of sheet, using region's placement.
 *
 * \param normalSign Non-zero if image is a normal map which XY need to be rotated
 *                   along with the image; +1 if green points down in image, -1 if up.
 */
template < typename SourceType >
void placeImage(const drawing::Image* source, const TrimSheetRegion* region, const TrimSheetRect& rect, float normalSign, drawing::Image* sheet)
{
	const SourceType* src = static_cast< const SourceType* >(source->getData());
	float* dst = static_cast< float* >(sheet->getData());

	const TrimSheetRegion::Rotation rotation = region->getRotation();
	const int32_t offsetX = region->getOffsetX();
	const int32_t offsetY = region->getOffsetY();
	const int32_t sw = source->getWidth();
	const int32_t sh = source->getHeight();
	const int32_t pw = region->rotationSwapsAxes() ? sh : sw;
	const int32_t ph = region->rotationSwapsAxes() ? sw : sh;
	const bool tileH = region->tileHorizontal();
	const bool tileV = region->tileVertical();
	const bool rotateNormals = (normalSign != 0.0f && rotation != TrimSheetRegion::Rotation::Deg0);

	const int32_t x0 = std::max(rect.x, 0);
	const int32_t y0 = std::max(rect.y, 0);
	const int32_t x1 = std::min(rect.x + rect.width, sheet->getWidth());
	const int32_t y1 = std::min(rect.y + rect.height, sheet->getHeight());

	for (int32_t y = y0; y < y1; ++y)
	{
		int32_t v = y - rect.y - offsetY;
		if (tileV)
			v = wrap(v, ph);
		else if (v < 0 || v >= ph)
			continue;

		for (int32_t x = x0; x < x1; ++x)
		{
			int32_t u = x - rect.x - offsetX;
			if (tileH)
				u = wrap(u, pw);
			else if (u < 0 || u >= pw)
				continue;

			// Map from placed, rotated clockwise, image into source image.
			int32_t sx, sy;
			switch (rotation)
			{
			default:
			case TrimSheetRegion::Rotation::Deg0:
				sx = u;
				sy = v;
				break;

			case TrimSheetRegion::Rotation::Deg90:
				sx = v;
				sy = sh - 1 - u;
				break;

			case TrimSheetRegion::Rotation::Deg180:
				sx = sw - 1 - u;
				sy = sh - 1 - v;
				break;

			case TrimSheetRegion::Rotation::Deg270:
				sx = sw - 1 - v;
				sy = u;
				break;
			}

			const SourceType* s = &src[(sx + sy * sw) * 4];
			float* d = &dst[(x + y * sheet->getWidth()) * 4];

			d[0] = normalized(s[0]);
			d[1] = normalized(s[1]);
			d[2] = normalized(s[2]);
			d[3] = normalized(s[3]);

			// Rotate tangent space XY of normal; image space is Y down so a clockwise
			// rotation is (x, y) -> (-y, x) where normal's Y is scaled by normalSign.
			if (rotateNormals)
			{
				const float nx = d[0] * 2.0f - 1.0f;
				const float ny = d[1] * 2.0f - 1.0f;
				float rx = nx, ry = ny;
				switch (rotation)
				{
				default:
					break;

				case TrimSheetRegion::Rotation::Deg90:
					rx = -normalSign * ny;
					ry = normalSign * nx;
					break;

				case TrimSheetRegion::Rotation::Deg180:
					rx = -nx;
					ry = -ny;
					break;

				case TrimSheetRegion::Rotation::Deg270:
					rx = normalSign * ny;
					ry = -normalSign * nx;
					break;
				}
				d[0] = rx * 0.5f + 0.5f;
				d[1] = ry * 0.5f + 0.5f;
			}
		}
	}
}

void fillRect(drawing::Image* sheet, const TrimSheetRect& rect, const Color4f& color)
{
	float T_MATH_ALIGN16 c[4];
	color.storeAligned(c);

	float* dst = static_cast< float* >(sheet->getData());

	const int32_t x0 = std::max(rect.x, 0);
	const int32_t y0 = std::max(rect.y, 0);
	const int32_t x1 = std::min(rect.x + rect.width, sheet->getWidth());
	const int32_t y1 = std::min(rect.y + rect.height, sheet->getHeight());

	for (int32_t y = y0; y < y1; ++y)
	{
		for (int32_t x = x0; x < x1; ++x)
		{
			float* d = &dst[(x + y * sheet->getWidth()) * 4];
			d[0] = c[0];
			d[1] = c[1];
			d[2] = c[2];
			d[3] = c[3];
		}
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TrimSheetComposer", TrimSheetComposer, Object)

TrimSheetComposer::TrimSheetComposer(const Path& assetPath)
:	m_assetPath(assetPath)
{
}

Ref< drawing::Image > TrimSheetComposer::compose(const TrimSheetSetupAsset* setup, TrimSheetLayer layer)
{
	const int32_t width = setup->getWidth();
	const int32_t height = setup->getHeight();
	if (width <= 0 || height <= 0 || width > c_maxSize || height > c_maxSize)
	{
		log::error << L"Trim sheet size " << width << L" * " << height << L" not supported; must be 1 to " << c_maxSize << L" pixels." << Endl;
		return nullptr;
	}

	Ref< drawing::Image > sheet = new drawing::Image(drawing::PixelFormat::getRGBAF32(), width, height);
	fillRect(sheet, { 0, 0, width, height }, setup->getBackground(layer));

	AlignedVector< TrimSheetRect > slabs;
	AlignedVector< TrimSheetSetupAsset::RegionLayout > regions;
	setup->calculateLayout(slabs, regions);

	for (const auto& regionLayout : regions)
		placeRegion(setup, layer, regionLayout, sheet);

	Ref< drawing::ImageInfo > imageInfo = new drawing::ImageInfo();
	imageInfo->setGamma(layer == TrimSheetLayer::Albedo ? 2.2f : 1.0f);
	sheet->setImageInfo(imageInfo);

	return sheet;
}

Ref< drawing::Image > TrimSheetComposer::composeThumbnail(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, int32_t width, int32_t height)
{
	Ref< drawing::Image > sheet = compose(setup, layer);
	if (!sheet)
		return nullptr;

	const drawing::ScaleFilter scaleFilter(width, height, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
	sheet->apply(&scaleFilter);
	return sheet;
}

void TrimSheetComposer::composeRegion(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, const TrimSheetSetupAsset::RegionLayout& regionLayout, drawing::Image* sheet)
{
	if (!sheet || sheet->getWidth() != setup->getWidth() || sheet->getHeight() != setup->getHeight())
		return;

	fillRect(sheet, regionLayout.rect, setup->getBackground(layer));
	placeRegion(setup, layer, regionLayout, sheet);
}

bool TrimSheetComposer::getPlacedSize(const TrimSheetRegion* region, TrimSheetLayer layer, int32_t& outWidth, int32_t& outHeight)
{
	const Path& fileName = region->getFileName(layer);
	if (fileName.empty())
		return false;

	const drawing::Image* source = getSource(fileName, region->getScale(), layer);
	if (!source)
		return false;

	outWidth = region->rotationSwapsAxes() ? source->getHeight() : source->getWidth();
	outHeight = region->rotationSwapsAxes() ? source->getWidth() : source->getHeight();
	return true;
}

void TrimSheetComposer::flush()
{
	m_sources.clear();
}

void TrimSheetComposer::collectFiles(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, std::set< std::wstring >& outFiles)
{
	for (auto slab : setup->getSlabs())
	{
		for (auto region : slab->getRegions())
		{
			const Path& fileName = region->getFileName(layer);
			if (!fileName.empty())
				outFiles.insert(fileName.getOriginal());
		}
	}
}

const drawing::Image* TrimSheetComposer::getSource(const Path& fileName, float scale, TrimSheetLayer layer)
{
	const Path filePath = FileSystem::getInstance().getAbsolutePath(m_assetPath, fileName);
	const bool sRGB = (layer == TrimSheetLayer::Albedo);

	StringOutputStream ss;
	ss << filePath.getPathName() << L"|" << scale << L"|" << (sRGB ? L"sRGB" : L"linear");
	Source& source = m_sources[ss.str()];

	Ref< File > file = FileSystem::getInstance().get(filePath);
	if (!file)
	{
		// Only report once until file appear.
		if (!source.missing)
			log::warning << L"Trim sheet source image \"" << filePath.getPathName() << L"\" not found." << Endl;
		source.missing = true;
		source.image = nullptr;
		return nullptr;
	}

	const uint64_t lastWriteTime = file->getLastWriteTime().getSecondsSinceEpoch();
	const uint64_t size = file->getSize();

	if (!source.missing && source.lastWriteTime == lastWriteTime && source.size == size)
		return source.image;

	source.lastWriteTime = lastWriteTime;
	source.size = size;
	source.missing = false;
	source.image = loadSource(filePath, scale, sRGB);
	if (!source.image)
		log::error << L"Unable to load trim sheet source image \"" << filePath.getPathName() << L"\"." << Endl;

	return source.image;
}

void TrimSheetComposer::placeRegion(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, const TrimSheetSetupAsset::RegionLayout& regionLayout, drawing::Image* sheet)
{
	const TrimSheetRegion* region = setup->getRegion(regionLayout.slab, regionLayout.region);
	if (!region || regionLayout.rect.empty())
		return;

	const Path& fileName = region->getFileName(layer);
	if (fileName.empty())
		return;

	const drawing::Image* source = getSource(fileName, region->getScale(), layer);
	if (!source)
		return;

	float normalSign = 0.0f;
	if (layer == TrimSheetLayer::Normal)
		normalSign = (setup->getNormalConvention() == TrimSheetSetupAsset::NormalConvention::DirectX) ? 1.0f : -1.0f;

	if (source->getPixelFormat().isFloatPoint())
		placeImage< float >(source, region, regionLayout.rect, normalSign, sheet);
	else
		placeImage< uint8_t >(source, region, regionLayout.rect, normalSign, sheet);
}

}
