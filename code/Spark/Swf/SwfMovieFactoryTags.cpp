/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Compress/Zip/InflateStreamZip.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/Endian.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Formats/ImageFormatJpeg.h"
#include "Spark/BitmapImage.h"
#include "Spark/Button.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Edit.h"
#include "Spark/Font.h"
#include "Spark/Frame.h"
#include "Spark/MorphShape.h"
#include "Spark/Movie.h"
#include "Spark/Shape.h"
#include "Spark/Sound.h"
#include "Spark/Sprite.h"
#include "Spark/Text.h"
#include "Spark/Swf/SwfMovieFactoryTags.h"
#include "Spark/Swf/SwfReader.h"

namespace traktor::spark
{

// ============================================================================
// Set background color

bool TagSetBackgroundColor::read(SwfReader* swf, ReadContext& context)
{
	const Color4f color = swf->readRgb();
	context.frame->changeBackgroundColor(color);
	return true;
}

// ============================================================================
// Define shape

TagDefineShape::TagDefineShape(int shapeType)
:	m_shapeType(shapeType)
{
}

bool TagDefineShape::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t shapeId = bs.readUInt16();
	const Aabb2 shapeBounds = swf->readRect();

	if (m_shapeType == 4)
	{
		/*Aabb2 edgeBounds = */swf->readRect();

		bs.skip(5);

		/*
		bool useFillWindingRule = bs.readBit();
		bool usesNonScalingStrokes = bs.readBit();
		bool usesScalingStrokes = bs.readBit();
		*/
		bs.skip(3);
	}

	SwfShape* shape;
	SwfStyles* styles;
	if (!swf->readShapeWithStyle(shape, styles, m_shapeType))
		return false;

	Ref< Shape > shapeCharacter = new Shape();
	if (!shapeCharacter->create(shapeBounds, shape, styles))
		return false;

	context.movie->defineCharacter(shapeId, shapeCharacter);
	return true;
}

// ============================================================================
// Define morph shape

TagDefineMorphShape::TagDefineMorphShape(int shapeType)
:	m_shapeType(shapeType)
{
}

bool TagDefineMorphShape::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t shapeId = swf->getBitReader().readUInt16();
	const Aabb2 startBounds = swf->readRect();
	/*Aabb2 endBounds = */swf->readRect();

	if (m_shapeType == 2)
	{
		/*Aabb2 startEdgeBounds = */swf->readRect();
		/*Aabb2 endEdgeBounds = */swf->readRect();

		bs.skip(6);
		/*bool nonScalingStrokes = */bs.readBit();
		/*bool scalingStrokes = */bs.readBit();
	}

	/*uint32_t offsetMorph = */bs.readUInt32();

	SwfStyles *startStyles, *endStyles;
	if (!swf->readMorphStyles(startStyles, endStyles, m_shapeType))
		return false;

	const SwfShape* startShape = swf->readShape(m_shapeType);
	if (!startShape)
		return false;

	const SwfShape* endShape = swf->readShape(m_shapeType);
	if (!endShape)
		return false;

	Ref< MorphShape > shape = new MorphShape();
	if (!shape->create(startBounds, startShape, endShape, startStyles, endStyles))
		return false;

	context.movie->defineCharacter(shapeId, shape);
	return true;
}

// ============================================================================
// Define font

TagDefineFont::TagDefineFont(int fontType)
:	m_fontType(fontType)
{
}

bool TagDefineFont::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t fontId = bs.readUInt16();

	if (m_fontType == 1)
	{
		const int64_t offsetBase = bs.getStream()->tell();
		const uint16_t firstOffset = bs.readUInt16();
		const uint16_t glyphCount = firstOffset >> 1;

		AlignedVector< uint16_t > offsetTable(glyphCount);
		offsetTable[0] = firstOffset;
		for (uint16_t i = 1; i < glyphCount; ++i)
			offsetTable[i] = bs.readUInt16();

		RefArray< Shape > shapeTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
		{
			T_ASSERT(offsetBase + offsetTable[i] < context.tagEndPosition);
			bs.getStream()->seek(IStream::SeekSet, offsetBase + offsetTable[i]);
			bs.alignByte();

			const SwfShape* swfShape = swf->readShape(0);

			Ref< Shape > shape = new Shape();
			if (!shape->create(swfShape))
				return false;

			shapeTable[i] = shape;
		}

		Ref< Font > font = new Font();
		if (!font->initializeFromShapes(shapeTable))
			return false;

		context.movie->defineFont(fontId, font);
	}
	else if (m_fontType == 2 || m_fontType == 3)
	{
		const bool hasLayout = bs.readBit();

		/*
		const bool shiftJIS = bs.readBit();
		const bool smallText = bs.readBit();		// SWF 7.0+
		const bool ansi = bs.readBit();
		*/
		bs.skip(3);

		const bool wideOffsets = bs.readBit();
		const bool wideCodes = bs.readBit();
		const bool italic = bs.readBit();
		const bool bold = bs.readBit();

		/*const uint8_t languageCode = */bs.readUInt8();	// SWF 6.0+
		const std::string fontName = swf->readStringU8();
		const uint16_t glyphCount = bs.readUInt16();
		const int64_t offsetBase = bs.getStream()->tell();

		AlignedVector< uint32_t > offsetTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
			offsetTable[i] = wideOffsets ? bs.readUInt32() : bs.readUInt16();

		const uint32_t codeOffset = wideOffsets ? bs.readUInt32() : bs.readUInt16();

		RefArray< Shape > shapeTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
		{
			T_ASSERT(offsetBase + offsetTable[i] < context.tagEndPosition);
			bs.getStream()->seek(IStream::SeekSet, offsetBase + offsetTable[i]);
			bs.alignByte();

			const SwfShape* swfShape = swf->readShape(0);

			Ref< Shape > shape = new Shape();
			if (!shape->create(swfShape))
				return false;

			shapeTable[i] = shape;
		}

		const int64_t currentPosition = bs.getStream()->tell();
		T_ASSERT(offsetBase + codeOffset == currentPosition);

		AlignedVector< uint16_t > codeTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
			codeTable[i] = wideCodes ? bs.readUInt16() : bs.readUInt8();

		int16_t ascent = 0, descent = 0, leading = 0;
		AlignedVector< int16_t > advanceTable;
		AlignedVector< Aabb2 > boundsTable;
		SmallMap< uint32_t, int16_t > kerningLookup;

		if (hasLayout)
		{
			ascent = bs.readInt16();
			descent = bs.readInt16();
			leading = bs.readInt16();

			advanceTable.resize(glyphCount);
			for (uint16_t i = 0; i < glyphCount; ++i)
				advanceTable[i] = bs.readInt16();

			boundsTable.resize(glyphCount);
			for (uint16_t i = 0; i < glyphCount; ++i)
				boundsTable[i] = swf->readRect();

			const uint16_t kerningCount = bs.readUInt16();
			for (uint16_t i = 0; i < kerningCount; ++i)
			{
				const SwfKerningRecord kerning = swf->readKerningRecord(wideCodes);
				const uint32_t codePair = (uint32_t(kerning.code1) << 16) | kerning.code2;
				kerningLookup[codePair] = kerning.adjustment;
			}
		}

		Ref< Font > font = new Font();
		if (!font->initialize(
			fontName,
			italic,
			bold,
			shapeTable,
			ascent,
			descent,
			leading,
			advanceTable,
			boundsTable,
			kerningLookup,
			codeTable,
			m_fontType == 3 ? Font::CtEMSquare : Font::CtTwips
		))
			return false;

		context.movie->defineFont(fontId, font);
	}

	return true;
}

// ============================================================================
// Define scaling grid

bool TagDefineScalingGrid::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t characterId = bs.readUInt16();
	const Aabb2 splitter = swf->readRect();

	const SmallMap< uint16_t, Ref< Character > >& characters = context.movie->getCharacters();

	SmallMap< uint16_t, Ref< Character > >::const_iterator i = characters.find(characterId);
	if (i == characters.end())
		return false;

	if (Sprite* spriteCharacter = dynamic_type_cast< Sprite* >(i->second))
		spriteCharacter->setScalingGrid(splitter);
	else
		log::warning << L"Only sprite characters can have 9-grid scaling; scaling grid ignored." << Endl;

	return true;
}

// ============================================================================
// Define text

TagDefineText::TagDefineText(int textType)
:	m_textType(textType)
{
}

bool TagDefineText::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t textId = bs.readUInt16();
	const Aabb2 textBounds = swf->readRect();
	const SwfMatrix textMatrix = swf->readMatrix();

	const uint8_t numGlyphBits = bs.readUInt8();
	const uint8_t numAdvanceBits = bs.readUInt8();

	AlignedVector< SwfTextRecord* > textRecords;
	for (;;)
	{
		SwfTextRecord* textRecord = swf->readTextRecord(numGlyphBits, numAdvanceBits, m_textType);
		if (!textRecord)
			return false;

		if (!textRecord->styleFlag && textRecord->glyph.glyphCount == 0)
			break;

		textRecords.push_back(textRecord);
	}

	Ref< Text > text = new Text(textBounds, Matrix33(textMatrix.m));
	if (!text->create(textRecords))
		return false;

	context.movie->defineCharacter(textId, text);
	return true;
}

// ============================================================================
// Define edit text

bool TagDefineEditText::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t textId = bs.readUInt16();
	const Aabb2 textBounds = swf->readRect();
	Color4f textColor(1.0f, 1.0f, 1.0f, 1.0f);

	bs.alignByte();

	const bool hasText = bs.readBit();
	const bool wordWrap = bs.readBit();
	const bool multiLine = bs.readBit();
	const bool password = bs.readBit();
	const bool readOnly = bs.readBit();
	const bool hasColor = bs.readBit();
	const bool hasMaxLength = bs.readBit();
	const bool hasFont = bs.readBit();
	bool autoSize = false;
	if (context.version >= 6)
	{
		bs.skip(1);
		autoSize = bs.readBit();
	}
	else
		bs.skip(2);
	const bool hasLayout = bs.readBit();

	/*
	bool noSelect = bs.readBit();
	bool border = bs.readBit();
	*/
	bs.skip(2);

	bs.skip(1);
	const bool html = bs.readBit();

	/*
	bool useOutlines = bs.readBit();
	*/
	bs.skip(1);

	uint16_t fontId = 0;
	uint16_t fontHeight = 10;
	uint16_t maxLength = std::numeric_limits< uint16_t >::max();
	if (hasFont)
	{
		fontId = bs.readUInt16();
		fontHeight = bs.readUInt16();
	}
	if (hasColor)
		textColor = swf->readRgba();
	if (hasMaxLength)
		maxLength = bs.readUInt16();

	uint8_t align = 0;
	uint16_t leftMargin = 0, rightMargin = 0;
	int16_t indent = 0, leading = 0;

	if (hasLayout)
	{
		align = bs.readUInt8();
		leftMargin = bs.readUInt16();
		rightMargin = bs.readUInt16();
		indent = bs.readInt16();
		leading = bs.readInt16();
	}

	const std::wstring variableName = mbstows(swf->readString());
	std::wstring initialText;
	if (hasText)
	{
		std::string it = swf->readString();
		initialText = mbstows(Utf8Encoding(), it);
	}

	Ref< Edit > edit = new Edit(
		fontId,
		fontHeight,
		textBounds,
		textColor,
		maxLength,
		initialText,
		(SwfTextAlignType)align,
		leftMargin,
		rightMargin,
		indent,
		leading,
		readOnly,
		wordWrap,
		multiLine,
		password,
		html
	);

	context.movie->defineCharacter(textId, edit);
	return true;
}

// ============================================================================
// Define button

TagDefineButton::TagDefineButton(int buttonType)
:	m_buttonType(buttonType)
{
}

bool TagDefineButton::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t buttonId = bs.readUInt16();
	Ref< Button > button = new Button();

	if (m_buttonType == 1)
	{
		// \fixme Not implemented.
		log::error << L"TagDefineButton : Not implemented" << Endl;
		return false;
	}
	else if (m_buttonType == 2)
	{
		bs.skip(7);

		/*bool trackAsMenu = */bs.readBit();
		/*uint16_t actionOffset = */bs.readUInt16();

		// Read button characters.
		for (;;)
		{
			bs.alignByte();
			bs.skip(2);

			bool hasBlendMode = false;
			bool hasFilterList = false;

			if (context.version >= 8)
			{
				hasBlendMode = bs.readBit();
				hasFilterList = bs.readBit();
			}
			else
				bs.skip(2);

			Button::ButtonLayer layer;

			layer.state |= bs.readBit() ? Button::SmHitTest : 0;
			layer.state |= bs.readBit() ? Button::SmDown : 0;
			layer.state |= bs.readBit() ? Button::SmOver: 0;
			layer.state |= bs.readBit() ? Button::SmUp : 0;

			if (!layer.state)
				break;

			layer.characterId = bs.readUInt16();
			layer.placeDepth = bs.readUInt16();
			layer.placeMatrix = Matrix33(swf->readMatrix().m);

			if (m_buttonType == 2)
				layer.cxform = swf->readCxTransform(true);

			if (hasFilterList)
			{
				AlignedVector< SwfFilter* > filterList;
				if (!swf->readFilterList(filterList))
					return false;
			}
			if (hasBlendMode)
			{
				/*uint8_t blendMode = */bs.readUInt8();
			}

			button->addButtonLayer(layer);
		}

		// Read conditions.
		//if (context.avm1)
		//{
		//	bs.alignByte();
		//	while (uint32_t(bs.getStream()->tell()) < context.tagEndPosition)
		//	{
		//		uint16_t conditionLength = bs.readUInt16();

		//		Button::ButtonCondition condition;

		//		condition.mask |= bs.readBit() ? Button::CmIdleToOverDown : 0;
		//		condition.mask |= bs.readBit() ? Button::CmOutDownToIdle : 0;
		//		condition.mask |= bs.readBit() ? Button::CmOutDownToOverDown : 0;
		//		condition.mask |= bs.readBit() ? Button::CmOverDownToOutDown : 0;
		//		condition.mask |= bs.readBit() ? Button::CmOverDownToOverUp : 0;
		//		condition.mask |= bs.readBit() ? Button::CmOverUpToOverDown : 0;
		//		condition.mask |= bs.readBit() ? Button::CmOverUpToIdle : 0;
		//		condition.mask |= bs.readBit() ? Button::CmIdleToOverUp : 0;

		//		condition.key = bs.readUnsigned(7);

		//		condition.mask |= bs.readBit() ? Button::CmOverDownToIdle : 0;

		//		condition.script = context.avm1->load(*swf);
		//		bs.alignByte();

		//		button->addButtonCondition(condition);

		//		if (!conditionLength)
		//			break;
		//	}
		//}
	}

	context.movie->defineCharacter(buttonId, button);
	return true;
}

// ============================================================================
// JPEGTables

bool TagJpegTables::read(SwfReader* swf, ReadContext& context)
{
	context.jpegFormat = new drawing::ImageFormatJpeg();

	// Strange, seems Flash CS3 emits a 0 length tag, assuming
	// it's valid and there is only a single JPEG in the SWF which
	// will contain the JPEG header locally.
	if (context.tagSize == 0)
		return true;

	BitReader& bs = swf->getBitReader();

	// Read entire tag's content into memory buffer first, need to correct SWF
	// bugs in data.
	AutoArrayPtr< uint8_t > buffer(new uint8_t [context.tagSize]);
	bs.getStream()->read(buffer.ptr(), context.tagSize);

	// Prior to SWF 8.0 there could be a problem with incorrect JFIF start tag->
	if (buffer[0] == 0xff && buffer[1] == 0xd9 && buffer[2] == 0xff && buffer[3] == 0xd8)
	{
		buffer[1] = 0xd8;
		buffer[3] = 0xd9;
	}

	// Ensure data appears to be correct(ed).
	T_ASSERT(buffer[0] == 0xff && buffer[1] == 0xd8);

	MemoryStream bufferStream(buffer.ptr(), int(context.tagSize), true, false);
	context.jpegFormat->readJpegHeader(&bufferStream);

	return true;
}

// ============================================================================
// Define bits jpeg

TagDefineBitsJpeg::TagDefineBitsJpeg(int bitsType)
:	m_bitsType(bitsType)
{
}

bool TagDefineBitsJpeg::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t bitmapId = bs.readUInt16();

	const uint32_t tagSize = context.tagSize - sizeof(uint16_t) - sizeof(uint32_t);
	uint32_t offsetToAlpha = tagSize;
	if (m_bitsType == 3)
		offsetToAlpha = bs.readUInt32();

	// Read entire tag's content into memory buffer first, need to correct SWF
	// bugs in data.
	AutoArrayPtr< uint8_t > buffer(new uint8_t [tagSize]);
	if (bs.getStream()->read(buffer.ptr(), tagSize) != tagSize)
		return false;

	// Prior to SWF 8.0 there could be a problem with incorrect JFIF start tag->
	if (buffer[0] == 0xff && buffer[1] == 0xd9 && buffer[2] == 0xff && buffer[3] == 0xd8)
	{
		buffer[1] = 0xd8;
		buffer[3] = 0xd9;
	}

	// Ensure data appears to be correct(ed).
	T_ASSERT(buffer[0] == 0xff && buffer[1] == 0xd8);
	MemoryStream bufferStream(buffer.ptr(), tagSize, true, false);

	// Decode JPEG image, either by using previously defined encoding tables
	// or, new method, with own encoding tables.
	if (m_bitsType == 1)
	{
		T_ASSERT(context.jpegFormat);

		Ref< drawing::Image > image = context.jpegFormat->readJpegImage(&bufferStream);
		T_ASSERT(image);

		context.movie->defineBitmap(bitmapId, new BitmapImage(image));
	}
	else
	{
		Ref< drawing::ImageFormatJpeg > jpegFormat = new drawing::ImageFormatJpeg();
		if (!jpegFormat->readJpegHeader(&bufferStream))
			return false;

		Ref< drawing::Image > image;

		if (bufferStream.available() > 0)
		{
			image = jpegFormat->readJpegImage(&bufferStream);
			if (!image)
				return false;
		}

		if (m_bitsType == 3 && offsetToAlpha < tagSize)
		{
			const uint32_t inflateSize = image->getWidth() * image->getHeight();

			bufferStream.seek(IStream::SeekSet, offsetToAlpha);
			compress::InflateStreamZip inf(&bufferStream);

			AutoArrayPtr< uint8_t > alphaBuffer(new uint8_t [inflateSize]);
			if (inf.read(alphaBuffer.ptr(), inflateSize) != inflateSize)
				return false;

#if defined(T_LITTLE_ENDIAN)
			image->convert(drawing::PixelFormat::getA8B8G8R8());
#else	// T_BIG_ENDIAN
			image->convert(drawing::PixelFormat::getR8G8B8A8());
#endif

			uint8_t* bits = static_cast< uint8_t* >(image->getData());
			for (uint32_t i = 0; i < inflateSize; ++i)
				bits[i * 4 + 3] = alphaBuffer[i];
		}

		if (image)
			context.movie->defineBitmap(bitmapId, new BitmapImage(image));
	}

	return true;
}

// ============================================================================
// Define bits loss less

TagDefineBitsLossLess::TagDefineBitsLossLess(int bitsType)
:	m_bitsType(bitsType)
{
}

bool TagDefineBitsLossLess::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t bitmapId = bs.readUInt16();
	const uint8_t format = bs.readUInt8();
	const uint16_t width = bs.readUInt16();
	const uint16_t height = bs.readUInt16();

	if (format == 3)	// Palette
	{
		const uint8_t colorCount = bs.readUInt8() + 1;
		const uint8_t colorSize = (m_bitsType == 1) ? 3 : 4;
		const uint32_t colorTableSize = colorSize * colorCount;
		const uint32_t imagePitch = (width + 3UL) & ~3UL;

		const uint32_t bufferSize = colorTableSize + imagePitch * height;
		AutoArrayPtr< uint8_t > buffer(new uint8_t [bufferSize]);
#if defined(_DEBUG)
		std::memset(buffer.ptr(), 0, bufferSize);
#endif

		compress::InflateStreamZip inflateStream(bs.getStream());
		inflateStream.read(&buffer[0], int(bufferSize));

		bs.getStream()->seek(IStream::SeekSet, context.tagEndPosition);
		bs.alignByte();

		Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), width, height);
		if (!image)
			return false;

		for (int32_t y = 0; y < height; ++y)
		{
			uint8_t* src = &buffer[colorTableSize + y * imagePitch];
			for (int32_t x = 0; x < width; ++x)
			{
				image->setPixelUnsafe(x, y, Color4f(
					buffer[src[x] * colorSize + 0] / 255.0f,
					buffer[src[x] * colorSize + 1] / 255.0f,
					buffer[src[x] * colorSize + 2] / 255.0f,
					(colorSize == 4) ? buffer[src[x] * colorSize + 3] / 255.0f : 1.0f
				));
			}
		}

		context.movie->defineBitmap(bitmapId, new BitmapImage(image));
	}
	else if (format == 4 || format == 5)	// RGB/RGBA
	{
		uint8_t colorSize;
		if (m_bitsType == 1)
		{
			if (format == 4)
				colorSize = 2;	// 1555
			else
				colorSize = 4;	// 888
		}
		else
			colorSize = 4;	// 8888

		uint32_t imagePitch = width * colorSize;
		imagePitch = (imagePitch + 3UL) & ~3UL;

		const uint32_t bufferSize = imagePitch * height;
		AutoArrayPtr< uint8_t > buffer(new uint8_t [bufferSize]);
#if defined(_DEBUG)
		std::memset(buffer.ptr(), 0, bufferSize);
#endif

		compress::InflateStreamZip inflateStream(bs.getStream());
		inflateStream.read(&buffer[0], int(bufferSize));

		bs.getStream()->seek(IStream::SeekSet, context.tagEndPosition);
		bs.alignByte();

		Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), width, height);
		if (!image)
			return false;

		for (int32_t y = 0; y < height; ++y)
		{
			uint8_t* src = &buffer[y * imagePitch];
			for (int32_t x = 0; x < width; ++x)
			{
				if (colorSize == 2)
				{
					uint16_t& c = reinterpret_cast< uint16_t& >(src[x * 2]);
#if defined(T_BIG_ENDIAN)
					swap8in16(c);
#endif
					image->setPixelUnsafe(x, y, Color4f(
						((c >> 7) & 0xf8) / 255.0f,
						((c >> 2) & 0xf8) / 255.0f,
						((c << 3) & 0xf8) / 255.0f,
						1.0f
					));
				}
				else
				{
					const Color4f clr(
						src[x * 4 + 1] / 255.0f,
						src[x * 4 + 2] / 255.0f,
						src[x * 4 + 3] / 255.0f,
						(m_bitsType > 1) ? src[x * 4 + 0] / 255.0f : 1.0f
					);
					image->setPixelUnsafe(x, y, clr);
				}
			}
		}

		context.movie->defineBitmap(bitmapId, new BitmapImage(image));
	}

	return true;
}

// ============================================================================
// Define sprite

bool TagDefineSprite::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t spriteId = bs.readUInt16();
	const uint16_t frameRate = bs.readUInt16();

	Ref< Sprite > sprite = new Sprite(frameRate);

	// Setup readers for supported sprite tags.
	SmallMap< uint16_t, Ref< Tag > > tagReaders;

	tagReaders[TiPlaceObject] = new TagPlaceObject(1);
	tagReaders[TiPlaceObject2] = new TagPlaceObject(2);
	tagReaders[TiPlaceObject3] = new TagPlaceObject(3);
	tagReaders[TiRemoveObject] = new TagRemoveObject(1);
	tagReaders[TiRemoveObject2] = new TagRemoveObject(2);
	tagReaders[TiStartSound] = new TagStartSound(1);
	tagReaders[TiStartSound2] = new TagStartSound(2);
	tagReaders[TiShowFrame] = new TagShowFrame();
	tagReaders[TiFrameLabel] = new TagFrameLabel();
	tagReaders[TiDefineScalingGrid] = new TagDefineScalingGrid();

	// Define readers for tags which isn't planed to be implemented.
	tagReaders[TiDefineFontInfo] = new TagUnsupported(TiDefineFontInfo);
	tagReaders[TiDefineSound] = new TagUnsupported(TiDefineSound);
	tagReaders[TiSoundStreamHead] = new TagUnsupported(TiSoundStreamHead);
	tagReaders[TiSoundStreamBlock] = new TagUnsupported(TiSoundStreamBlock);
	tagReaders[TiSoundStreamHead2] = new TagUnsupported(TiSoundStreamHead2);
	tagReaders[TiDebugID] = new TagUnsupported(TiDebugID);

	// Decode tags.
	Tag::ReadContext spriteContext;
	spriteContext.version = context.version;
	spriteContext.movie = context.movie;
	spriteContext.sprite = sprite;
	spriteContext.frame = new Frame();
	for (;;)
	{
		swf->enterScope();

		SwfTag* tag = swf->readTag();
		if (!tag || tag->id == TiEnd)
			break;

		Ref< Tag > tagReader = tagReaders[tag->id];
		if (tagReader)
		{
			spriteContext.tagSize = tag->length;
			spriteContext.tagEndPosition = swf->getBitReader().getStream()->tell() + tag->length;
			if (!tagReader->read(swf, spriteContext))
			{
				log::error << L"Unable to read flash, error when reading tag " << int32_t(tag->id) << Endl;
				return false;
			}
			if (uint32_t(swf->getBitReader().getStream()->tell()) < spriteContext.tagEndPosition)
			{
				log::warning << L"Read too few bytes (" << spriteContext.tagEndPosition - swf->getBitReader().getStream()->tell() << L" left) in tag " << int32_t(tag->id) << Endl;
				swf->getBitReader().getStream()->seek(IStream::SeekSet, spriteContext.tagEndPosition);
			}
			else if (uint32_t(swf->getBitReader().getStream()->tell()) > spriteContext.tagEndPosition)
			{
				log::error << L"Read too many bytes (" << swf->getBitReader().getStream()->tell() - spriteContext.tagEndPosition << L") in tag " << int32_t(tag->id) << Endl;
				swf->getBitReader().getStream()->seek(IStream::SeekSet, spriteContext.tagEndPosition);
			}
		}
		else
		{
			log::warning << L"Invalid sprite tag " << tag->id << Endl;
			swf->getBitReader().skip(tag->length * 8);
		}

		swf->leaveScope();
	}

	if (sprite->getFrameCount() == 0)
		sprite->addFrame(new Frame());

	context.movie->defineCharacter(spriteId, sprite);
	return true;
}

// ============================================================================
// Place object

TagPlaceObject::TagPlaceObject(int placeType)
:	m_placeType(placeType)
{
}

bool TagPlaceObject::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	if (m_placeType == 1)
	{
		Frame::PlaceObject placeObject;

		placeObject.hasFlags |= Frame::PfHasCharacterId | Frame::PfHasMatrix;
		placeObject.characterId = bs.readUInt16();
		placeObject.depth = bs.readUInt16();
		placeObject.matrix = Matrix33(swf->readMatrix().m);

		if (uint32_t(bs.getStream()->tell()) < context.tagEndPosition)
		{
			placeObject.hasFlags |= Frame::PfHasCxTransform;
			placeObject.cxTransform = swf->readCxTransform(false);
		}

		context.frame->placeObject(placeObject);
	}
	else if (m_placeType == 2 || m_placeType == 3)
	{
		Frame::PlaceObject placeObject;

		if (context.version >= 5)
			placeObject.hasFlags |= bs.readBit() ? Frame::PfHasActions : 0;
		else
			bs.skip(1);

		placeObject.hasFlags |= bs.readBit() ? Frame::PfHasClipDepth : 0;
		placeObject.hasFlags |= bs.readBit() ? Frame::PfHasName: 0;
		placeObject.hasFlags |= bs.readBit() ? Frame::PfHasRatio : 0;
		placeObject.hasFlags |= bs.readBit() ? Frame::PfHasCxTransform : 0;
		placeObject.hasFlags |= bs.readBit() ? Frame::PfHasMatrix : 0;
		placeObject.hasFlags |= bs.readBit() ? Frame::PfHasCharacterId : 0;
		placeObject.hasFlags |= bs.readBit() ? Frame::PfHasMove : 0;

		if (m_placeType == 2)
		{
			placeObject.depth = bs.readUInt16();
		}
		else if (m_placeType == 3)
		{
			bs.skip(1);

			placeObject.hasFlags |= bs.readBit() ? Frame::PfHasOpaqueBackground : 0;
			placeObject.hasFlags |= bs.readBit() ? Frame::PfHasVisible : 0;

			const bool hasImage = bs.readBit();
			const bool hasClassName = bs.readBit();

			placeObject.hasFlags |= bs.readBit() ? Frame::PfHasBitmapCaching : 0;
			placeObject.hasFlags |= bs.readBit() ? Frame::PfHasBlendMode : 0;
			placeObject.hasFlags |= bs.readBit() ? Frame::PfHasFilters : 0;

			placeObject.depth = bs.readUInt16();

			if (hasClassName || (hasImage && placeObject.has(Frame::PfHasCharacterId)))
			{
				const std::string className = swf->readString();
				log::warning << L"Unused class name " << mbstows(className) << L" in PlaceObject" << Endl;
			}
		}

		if (placeObject.has(Frame::PfHasCharacterId))
			placeObject.characterId = bs.readUInt16();

		if (placeObject.has(Frame::PfHasMatrix))
			placeObject.matrix = Matrix33(swf->readMatrix().m);

		if (placeObject.has(Frame::PfHasCxTransform))
			placeObject.cxTransform = swf->readCxTransform(true);

		if (placeObject.has(Frame::PfHasRatio))
			placeObject.ratio = bs.readUInt16();

		if (placeObject.has(Frame::PfHasName))
			placeObject.name = swf->readString();

		if (placeObject.has(Frame::PfHasClipDepth))
			placeObject.clipDepth = bs.readUInt16();

		if (m_placeType == 3)
		{
			if (placeObject.has(Frame::PfHasFilters))
			{
				AlignedVector< SwfFilter* > filterList;
				if (!swf->readFilterList(filterList))
					return false;

				if (!filterList.empty())
				{
					placeObject.filter = filterList.front()->filterId + 1;
					if (placeObject.filter == 1)	// Drop shadow
						placeObject.filterColor = Color4f::loadUnaligned(filterList.front()->dropShadow.dropShadowColor);
					if (placeObject.filter == 3)	// Glow
						placeObject.filterColor = Color4f::loadUnaligned(filterList.front()->glow.glowColor);
				}
			}

			if (placeObject.has(Frame::PfHasBlendMode))
				placeObject.blendMode = bs.readUInt8();

			if (placeObject.has(Frame::PfHasBitmapCaching))
				placeObject.bitmapCaching = bs.readUInt8();

			if (placeObject.has(Frame::PfHasVisible))
				placeObject.visible = bs.readUInt8();
		}

		//if (placeObject.has(Frame::PfHasActions) && context.avm1)
		//{
		//	uint16_t reserved = bs.readUInt16();
		//	if (reserved)
		//		log::warning << L"Reserved field in PlaceObject actions not zero, " << reserved << Endl;

		//	uint32_t allFlags;
		//	if (context.version >= 6)
		//		allFlags = bs.readUInt32();
		//	else
		//		allFlags = (bs.readUInt16() << 16);

		//	while (uint32_t(bs.getStream()->tell()) < context.tagEndPosition)
		//	{
		//		uint32_t eventMask = context.version >= 6 ? bs.readUInt32() : (bs.readUInt16() << 16);
		//		if (!eventMask)
		//			break;

		//		uint32_t eventLength = bs.readUInt32();
		//		if (uint32_t(bs.getStream()->tell()) + eventLength >= context.tagEndPosition)
		//		{
		//			log::error << L"PlaceObject, incorrect number of bytes in event tag" << Endl;
		//			break;
		//		}

		//		// "Key Press" events.
		//		if (eventMask & EvtKeyPress)
		//		{
		//			/*uint8_t keyCode = */bs.readUInt8();
		//			T_DEBUG(L"PlaceObject, unused keycode in EvtKeyPress");
		//		}

		//		Ref< const IActionVMImage > image = context.avm1->load(*swf);
		//		bs.alignByte();

		//		placeObject.events.insert(std::make_pair(eventMask, image));
		//	}
		//}

		context.frame->placeObject(placeObject);
	}

	return true;
}

// ============================================================================
// Remove object

TagRemoveObject::TagRemoveObject(int removeType)
:	m_removeType(removeType)
{
}

bool TagRemoveObject::read(SwfReader* swf, ReadContext& context)
{
	Frame::RemoveObject removeObject;
	if (m_removeType == 1)
	{
		removeObject.hasCharacterId = true;
		removeObject.characterId = swf->getBitReader().readUInt16();
		removeObject.depth = swf->getBitReader().readUInt16();
	}
	else if (m_removeType == 2)
	{
		removeObject.hasCharacterId = false;
		removeObject.depth = swf->getBitReader().readUInt16();
	}
	context.frame->removeObject(removeObject);
	return true;
}

// ============================================================================
// Show frame

bool TagShowFrame::read(SwfReader* swf, ReadContext& context)
{
	T_ASSERT(context.frame);
	context.sprite->addFrame(context.frame);
	context.frame = new Frame();
	return true;
}

// ============================================================================
// Do action

//bool TagDoAction::read(SwfReader* swf, ReadContext& context)
//{
//	Ref< const IActionVMImage > image = context.avm1->load(*swf);
//	if (image)
//		context.frame->addActionScript(image);
//	return true;
//}

// ============================================================================
// Export assets

bool TagExportAssets::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t exportCount = bs.readUInt16();
	for (int i = 0; i < exportCount; ++i)
	{
		const uint16_t id = bs.readUInt16();
		const std::string symbol = swf->readString();
		context.movie->setExport(symbol, id);
	}

	return true;
}

// ============================================================================
// Import assets

TagImportAssets::TagImportAssets(int importType)
:	m_importType(importType)
{
}

bool TagImportAssets::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const std::string url = swf->readString();

	if (m_importType == 2)
	{
		/*uint8_t version = */bs.readUInt8();
		/*uint8_t reserved = */bs.readUInt8();
	}

	const uint16_t count = bs.readUInt16();
	for (int i = 0; i < count; ++i)
	{
		const uint16_t id = bs.readUInt16();
		const std::string symbol = swf->readString();

		log::info << L"Import symbol \"" << mbstows(symbol) << L"\" as " << id << L" from URL \"" << mbstows(url) << L"\"" << Endl;
	}

	return true;
}

// ============================================================================
// Init action

//bool TagInitAction::read(SwfReader* swf, ReadContext& context)
//{
//	BitReader& bs = swf->getBitReader();
//
//	/*uint16_t spriteId = */bs.readUInt16();
//
//	Ref< const IActionVMImage > image = context.avm1->load(*swf);
//	bs.alignByte();
//
//	context.sprite->addInitActionScript(image);
//	return true;
//}

// ============================================================================
// Protect

TagProtect::TagProtect(int protectType)
{
}

bool TagProtect::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();
	bs.getStream()->seek(IStream::SeekSet, context.tagEndPosition);
	bs.alignByte();
	return true;
}

// ============================================================================
// Frame label

bool TagFrameLabel::read(SwfReader* swf, ReadContext& context)
{
	const std::string label = swf->readString();
	if (context.frame)
		context.frame->setLabel(label);
	return true;
}

// ============================================================================
// ABC

//bool TagDoABC::read(SwfReader* swf, ReadContext& context)
//{
//	BitReader& bs = swf->getBitReader();
//
//	uint32_t flags = bs.readUInt32();
//	std::string name = swf->readString();
//
//	Ref< const IActionVMImage > image = context.avm2->load(*swf);
//	if (!image)
//		return false;
//
//	context.frame->addActionScript(image);
//	return true;
//}

// ============================================================================
// Define sound

bool TagDefineSound::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t soundId = bs.readUInt16();

	const uint8_t soundFormat = bs.readUnsigned(4);
	if (soundFormat != 0 && soundFormat != 3)
	{
		log::error << L"Only uncompressed sounds are supported" << Endl;
		return false;
	}

	const uint8_t soundRate = bs.readUnsigned(2);
	const bool soundSize = bs.readBit();
	const bool soundType = bs.readBit();
	const uint32_t soundSampleCount = bs.readUInt32();

	const uint8_t soundChannels = soundType ? 2 : 1;
	const uint8_t soundSampleSize = soundSize ? 2 : 1;

	AutoArrayPtr< uint8_t > soundData(new uint8_t [soundSampleCount * soundSampleSize * soundChannels]);
	bs.getStream()->read(soundData.ptr(), soundSampleCount * soundSampleSize * soundChannels);
	bs.getStream()->seek(IStream::SeekSet, context.tagEndPosition);
	bs.alignByte();

	const uint32_t c_soundRates[] = { 5500, 11025, 22050, 44100 };
	T_ASSERT(soundRate < sizeof_array(c_soundRates));

	Ref< Sound > sound = new Sound();
	if (!sound->create(soundChannels, c_soundRates[soundRate], soundSampleCount))
		return false;

	if (soundSize)
	{
		// 16-bit samples
		const int16_t* ss = reinterpret_cast< const int16_t* >(soundData.c_ptr());

		int16_t* dsl = reinterpret_cast< int16_t* >(sound->getSamples(0));
		int16_t* dsr = reinterpret_cast< int16_t* >(sound->getSamples(1));

		if (soundType)
		{
			// Stereo
			for (uint32_t i = 0; i < soundSampleCount; ++i)
			{
				*dsl++ = *ss++;
				*dsr++ = *ss++;
			}
		}
		else
		{
			// Mono
			for (uint32_t i = 0; i < soundSampleCount; ++i)
				*dsl++ = *ss++;
		}
	}
	else
	{
		// 8-bit samples
		const int8_t* ss = reinterpret_cast< const int8_t* >(soundData.c_ptr());

		int16_t* dsl = reinterpret_cast< int16_t* >(sound->getSamples(0));
		int16_t* dsr = reinterpret_cast< int16_t* >(sound->getSamples(1));

		if (soundType)
		{
			// Stereo
			for (uint32_t i = 0; i < soundSampleCount; ++i)
			{
				*dsl++ = 256 * *ss++;
				*dsr++ = 256 * *ss++;
			}
		}
		else
		{
			// Mono
			for (uint32_t i = 0; i < soundSampleCount; ++i)
				*dsl++ = 256 * *ss++;
		}
	}

	context.movie->defineSound(soundId, sound);
	return true;
}

// ============================================================================
// Start sound

TagStartSound::TagStartSound(int32_t startType)
:	m_startType(startType)
{
}

bool TagStartSound::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	if (m_startType == 1)
	{
		const uint16_t soundId = bs.readUInt16();

		SwfSoundInfo* soundInfo = swf->readSoundInfo();
		if (!soundInfo)
			return false;

		context.frame->startSound(soundId);
	}
	else
		// #fixme
		return false;

	return true;
}

// ============================================================================
// Define scene and frame label data.

bool TagDefineSceneAndFrameLabelData::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint32_t sceneCount = swf->readEncodedU32();
	for (uint32_t i = 0; i < sceneCount; ++i)
	{
		const uint32_t offset = swf->readEncodedU32();
		const std::string name = swf->readString();

		log::info << i << L". offset = " << offset << L", name = " << mbstows(name) << Endl;
	}

	const uint32_t frameLabelCount = swf->readEncodedU32();
	for (uint32_t i = 0; i < frameLabelCount; ++i)
	{
		const uint32_t number = swf->readEncodedU32();
		const std::string label = swf->readString();

		log::info << i << L". number = " << number << L", label = " << mbstows(label) << Endl;
	}

	return true;
}

// ============================================================================
// Symbol class associations.

bool TagSymbolClass::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	const uint16_t numSymbols = bs.readUInt16();
	for (uint16_t i = 0; i < numSymbols; ++i)
	{
		const uint16_t tag = bs.readUInt16();
		const std::string name = swf->readString();
		context.movie->setExport(name, tag);
	}

	return true;
}

// ============================================================================
// Metadata

bool TagMetaData::read(SwfReader* swf, ReadContext& context)
{
	const std::string meta = swf->readString();
	log::info << mbstows(meta) << Endl;
	return true;
}

// ============================================================================
// FileAttributes

bool TagFileAttributes::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	bs.skip(1);
	const bool useDirectBlit = bs.readBit();
	const bool useGPU = bs.readBit();
	const bool hasMetadata = bs.readBit();
	const bool useAVM2 = bs.readBit();
	bs.skip(1);
	const bool useNetwork = bs.readBit();
	bs.skip(1);

	return true;
}

// ============================================================================
// Unsupported

TagUnsupported::TagUnsupported(int32_t tagId)
:	m_tagId(tagId)
,	m_visited(false)
{
}

bool TagUnsupported::read(SwfReader* swf, ReadContext& context)
{
	if (!m_visited)
	{
		T_DEBUG(L"Tag " << m_tagId << L" unsupported");
		m_visited = true;
	}

	BitReader& bs = swf->getBitReader();
	bs.getStream()->seek(IStream::SeekSet, context.tagEndPosition);
	bs.alignByte();

	return true;
}

}
