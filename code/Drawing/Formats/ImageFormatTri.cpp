/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Drawing/Image.h"
#include "Drawing/Formats/ImageFormatTri.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageFormatTri", ImageFormatTri, IImageFormat)

Ref< Image > ImageFormatTri::read(IStream* stream)
{
	Reader reader(stream);

	uint32_t magic;
	reader >> magic;
	if (magic != 'TRIF')
		return nullptr;

	uint8_t version;
	reader >> version;
	if (version != 2)
		return nullptr;

	return BinarySerializer(stream).readObject< Image >();
}

bool ImageFormatTri::write(IStream* stream, const Image* image)
{
	Writer writer(stream);
	writer << uint32_t('TRIF');
	writer << uint8_t(2);

	return BinarySerializer(stream).writeObject(image);
}

}
