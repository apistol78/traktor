/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Editor/App/ThumbnailGenerator.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ThumbnailGenerator", ThumbnailGenerator, IThumbnailGenerator)

ThumbnailGenerator::ThumbnailGenerator(const Path& thumbsPath)
:	m_thumbsPath(thumbsPath)
{
}

Ref< drawing::Image > ThumbnailGenerator::get(const Path& fileName, int32_t width, int32_t height, Alpha alpha, Gamma gamma)
{
	const wchar_t* alphaPrefix[] = { L"o", L"a", L"ao" };
	const wchar_t* gammaPrefix[] = { L"a", L"l", L"r" };

	const std::wstring pathName = fileName.getPathName();

	// Stat source file.
	Ref< File > file = FileSystem::getInstance().get(fileName);
	if (!file)
		return nullptr;

	// Generate checksum of full path to source image.
	Adler32 adler;
	adler.begin();
	adler.feed(pathName.c_str(), pathName.length());
	adler.end();

	const Path thumbFileName =
		m_thumbsPath.getPathName() + L"/" +
		fileName.getFileNameNoExtension() + L"_" +
		toString(adler.get()) + L"_" +
		toString(width) + L"x" + toString(height) + L"_" +
		alphaPrefix[(int32_t)alpha] +
		gammaPrefix[(int32_t)gamma] +
		toString(file->getLastWriteTime().getSecondsSinceEpoch()) +
		L".png";

	if (FileSystem::getInstance().exist(thumbFileName))
	{
		// Cached thumb exist; load thumb and assume it's up to date.
		return drawing::Image::load(thumbFileName);
	}

	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (!image)
		return nullptr;

	const drawing::ScaleFilter scale(
		width,
		height,
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgNearest
	);
	image->apply(&scale);

	if (image->getPixelFormat().getAlphaBits() > 0 && alpha == Alpha::WithAlpha)
	{
		Color4f pixel;
		for (int32_t y = 0; y < height; ++y)
		{
			for (int32_t x = 0; x < width; ++x)
			{
				const Color4f alpha =
					((x >> 2) & 1) ^ ((y >> 2) & 1) ?
					Color4f(0.4f, 0.4f, 0.4f) :
					Color4f(0.6f, 0.6f, 0.6f);

				image->getPixelUnsafe(x, y, pixel);

				pixel = pixel * pixel.getAlpha() + alpha * (Scalar(1.0f) - pixel.getAlpha());
				pixel.setAlpha(Scalar(1.0f));

				image->setPixelUnsafe(x, y, pixel);
			}
		}
	}
	else if (alpha == Alpha::AlphaOnly)
	{
		const drawing::SwizzleFilter swizzleFilter(L"aaa1");
		image->apply(&swizzleFilter);
	}
	else	// Create solid alpha channel.
	{
		image->convert(drawing::PixelFormat::getR8G8B8A8());
		image->clearAlpha(1.0f);
	}

	// Convert to sRGB if image is expected to be linear.
	if (gamma == Gamma::Auto)
	{
		const auto imageInfo = image->getImageInfo();
		if (imageInfo != nullptr && abs(imageInfo->getGamma() - 1.0f) < 0.01f)
			gamma = Gamma::Linear;
	}
	if (gamma == Gamma::Linear)
	{
		const drawing::GammaFilter gammaFilter(1.0f, 2.2f);
		image->apply(&gammaFilter);
	}

	// Ensure thumb path exist; then save thumb image.
	if (FileSystem::getInstance().makeAllDirectories(m_thumbsPath))
		image->save(thumbFileName);

	return image;
}

}
