#include <cstring>
#include <limits>
#include "Compress/Zip/InflateStreamZip.h"
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
#include "Flash/FlashMovieFactoryTags.h"
#include "Flash/FlashCharacterInstance.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSound.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashMorphShape.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashText.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashBitmapData.h"
#include "Flash/SwfReader.h"
#include "Flash/Action/IActionVM.h"

namespace traktor
{
	namespace flash
	{

// ============================================================================
// Set background color

bool FlashTagSetBackgroundColor::read(SwfReader* swf, ReadContext& context)
{
	SwfColor color = swf->readRgb();
	context.frame->changeBackgroundColor(color);
	return true;
}

// ============================================================================
// Define shape

FlashTagDefineShape::FlashTagDefineShape(int shapeType)
:	m_shapeType(shapeType)
{
}

bool FlashTagDefineShape::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t shapeId = bs.readUInt16();
	Aabb2 shapeBounds = swf->readRect();

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

	Ref< FlashShape > shapeCharacter = new FlashShape(shapeId);
	if (!shapeCharacter->create(shapeBounds, shape, styles))
		return false;

	context.movie->defineCharacter(shapeId, shapeCharacter);
	return true;
}

// ============================================================================
// Define morph shape

FlashTagDefineMorphShape::FlashTagDefineMorphShape(int shapeType)
:	m_shapeType(shapeType)
{
}

bool FlashTagDefineMorphShape::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t shapeId = swf->getBitReader().readUInt16();
	Aabb2 startBounds = swf->readRect();
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

	SwfShape* startShape = swf->readShape(m_shapeType);
	if (!startShape)
		return false;

	SwfShape* endShape = swf->readShape(m_shapeType);
	if (!endShape)
		return false;

	Ref< FlashMorphShape > shape = new FlashMorphShape(shapeId);
	if (!shape->create(startBounds, startShape, endShape, startStyles, endStyles))
		return false;

	context.movie->defineCharacter(shapeId, shape);
	return true;
}

// ============================================================================
// Define font

FlashTagDefineFont::FlashTagDefineFont(int fontType)
:	m_fontType(fontType)
{
}

bool FlashTagDefineFont::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t fontId = bs.readUInt16();

	if (m_fontType == 1)
	{
		uint32_t offsetBase = bs.getStream()->tell();
		uint16_t firstOffset = bs.readUInt16();
		uint16_t glyphCount = firstOffset >> 1;

		AlignedVector< uint16_t > offsetTable(glyphCount);
		offsetTable[0] = firstOffset;
		for (uint16_t i = 1; i < glyphCount; ++i)
			offsetTable[i] = bs.readUInt16();

		AlignedVector< SwfShape* > shapeTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
		{
			T_ASSERT (offsetBase + offsetTable[i] < context.tagEndPosition);
			bs.getStream()->seek(IStream::SeekSet, offsetBase + offsetTable[i]);
			bs.alignByte();
			shapeTable[i] = swf->readShape(0);
		}

		Ref< FlashFont > font = new FlashFont();
		if (!font->create(shapeTable))
			return false;

		context.movie->defineFont(fontId, font);
	}
	else if (m_fontType == 2 || m_fontType == 3)
	{
		bool hasLayout = bs.readBit();
		
		/*
		bool shiftJIS = bs.readBit();
		bool smallText = bs.readBit();		// SWF 7.0+
		bool ansi = bs.readBit();
		*/
		bs.skip(3);
		
		bool wideOffsets = bs.readBit();
		bool wideCodes = bs.readBit();
		
		/*
		bool italic = bs.readBit();
		bool bold = bs.readBit();
		*/
		bs.skip(2);
		
		/*uint8_t languageCode = */bs.readUInt8();	// SWF 6.0+
		std::string name = swf->readStringU8();
		
		uint16_t glyphCount = bs.readUInt16();
		if (!glyphCount)
			log::warning << L"Device fonts not supported; must embed fonts if used in dynamic fields" << Endl;

		uint32_t offsetBase = bs.getStream()->tell();

		std::vector< uint32_t > offsetTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
			offsetTable[i] = wideOffsets ? bs.readUInt32() : bs.readUInt16();

		uint32_t codeOffset = wideOffsets ? bs.readUInt32() : bs.readUInt16();

		AlignedVector< SwfShape* > shapeTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
		{
			T_ASSERT (offsetBase + offsetTable[i] < context.tagEndPosition);
			bs.getStream()->seek(IStream::SeekSet, offsetBase + offsetTable[i]);
			bs.alignByte();
			shapeTable[i] = swf->readShape(0);
		}

		uint32_t currentPosition = bs.getStream()->tell();
		T_ASSERT (offsetBase + codeOffset == currentPosition);

		AlignedVector< uint16_t > codeTable(glyphCount);
		for (uint16_t i = 0; i < glyphCount; ++i)
			codeTable[i] = wideCodes ? bs.readUInt16() : bs.readUInt8();

		int16_t ascent = 0, descent = 0, leading = 0;
		AlignedVector< int16_t > advanceTable;
		AlignedVector< Aabb2 > boundsTable;
		AlignedVector< SwfKerningRecord > kerningTable;

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

			uint16_t kerningCount = bs.readUInt16();
			kerningTable.resize(kerningCount);
			for (uint16_t i = 0; i < kerningCount; ++i)
				kerningTable[i] = swf->readKerningRecord(wideCodes);
		}

		Ref< FlashFont > font = new FlashFont();
		if (!font->create(
			shapeTable,
			ascent,
			descent,
			leading,
			advanceTable,
			boundsTable,
			kerningTable,
			codeTable,
			m_fontType == 3 ? FlashFont::CtEMSquare : FlashFont::CtTwips
		))
			return false;

		context.movie->defineFont(fontId, font);
	}

	return true;
}

// ============================================================================
// Define scaling grid

bool FlashTagDefineScalingGrid::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t characterId = bs.readUInt16();
	Aabb2 splitter = swf->readRect();

	const SmallMap< uint16_t, Ref< FlashCharacter > >& characters = context.movie->getCharacters();
	
	SmallMap< uint16_t, Ref< FlashCharacter > >::const_iterator i = characters.find(characterId);
	if (i == characters.end())
		return false;

	if (FlashSprite* spriteCharacter = dynamic_type_cast< FlashSprite* >(i->second))
		spriteCharacter->setScalingGrid(splitter);
	else
		log::warning << L"Only sprite characters can have 9-grid scaling; scaling grid ignored." << Endl;

	return true;
}

// ============================================================================
// Define text

FlashTagDefineText::FlashTagDefineText(int textType)
:	m_textType(textType)
{
}

bool FlashTagDefineText::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t textId = bs.readUInt16();
	Aabb2 textBounds = swf->readRect();
	SwfMatrix textMatrix = swf->readMatrix();

	uint8_t numGlyphBits = bs.readUInt8();
	uint8_t numAdvanceBits = bs.readUInt8();

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
	
	Ref< FlashText > text = new FlashText(textId, textBounds, Matrix33(textMatrix.m));
	if (!text->create(textRecords))
		return false;

	context.movie->defineCharacter(textId, text);
	return true;
}

// ============================================================================
// Define edit text

bool FlashTagDefineEditText::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t textId = bs.readUInt16();
	Aabb2 textBounds = swf->readRect();
	SwfColor textColor = { 255, 255, 255, 255 };

	bs.alignByte();

	bool hasText = bs.readBit();
	bool wordWrap = bs.readBit();
	bool multiLine = bs.readBit();

	/*
	bool password = bs.readBit();
	*/
	bs.skip(1);

	bool readOnly = bs.readBit();
	bool hasColor = bs.readBit();
	bool hasMaxLength = bs.readBit();
	bool hasFont = bs.readBit();
	bool autoSize = false;
	if (context.version >= 6)
	{
		bs.skip(1);
		autoSize = bs.readBit();
	}
	else
		bs.skip(2);
	bool hasLayout = bs.readBit();
	
	/*
	bool noSelect = bs.readBit();
	bool border = bs.readBit();
	*/
	bs.skip(2);
	
	bs.skip(1);
	bool html = bs.readBit();
	
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

	std::wstring variableName = mbstows(swf->readString());
	std::wstring initialText;
	if (hasText)
	{
		std::string it = swf->readString();
		initialText = mbstows(Utf8Encoding(), it);
	}

	Ref< FlashEdit > edit = new FlashEdit(
		textId,
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
		html
	);

	context.movie->defineCharacter(textId, edit);
	return true;
}

// ============================================================================
// Define button

FlashTagDefineButton::FlashTagDefineButton(int buttonType)
:	m_buttonType(buttonType)
{
}

bool FlashTagDefineButton::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t buttonId = bs.readUInt16();
	Ref< FlashButton > button = new FlashButton(buttonId);

	if (m_buttonType == 1)
	{
		// \fixme Not implemented.
		log::error << L"FlashTagDefineButton : Not implemented" << Endl;
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

			FlashButton::ButtonLayer layer;

			layer.state |= bs.readBit() ? FlashButton::SmHitTest : 0;
			layer.state |= bs.readBit() ? FlashButton::SmDown : 0;
			layer.state |= bs.readBit() ? FlashButton::SmOver: 0;
			layer.state |= bs.readBit() ? FlashButton::SmUp : 0;

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
		if (context.avm1)
		{
			bs.alignByte();
			while (uint32_t(bs.getStream()->tell()) < context.tagEndPosition)
			{
				uint16_t conditionLength = bs.readUInt16();

				FlashButton::ButtonCondition condition;

				condition.mask |= bs.readBit() ? FlashButton::CmIdleToOverDown : 0;
				condition.mask |= bs.readBit() ? FlashButton::CmOutDownToIdle : 0;
				condition.mask |= bs.readBit() ? FlashButton::CmOutDownToOverDown : 0;
				condition.mask |= bs.readBit() ? FlashButton::CmOverDownToOutDown : 0;
				condition.mask |= bs.readBit() ? FlashButton::CmOverDownToOverUp : 0;
				condition.mask |= bs.readBit() ? FlashButton::CmOverUpToOverDown : 0;
				condition.mask |= bs.readBit() ? FlashButton::CmOverUpToIdle : 0;
				condition.mask |= bs.readBit() ? FlashButton::CmIdleToOverUp : 0;

				condition.key = bs.readUnsigned(7);

				condition.mask |= bs.readBit() ? FlashButton::CmOverDownToIdle : 0;

				condition.script = context.avm1->load(bs);
				bs.alignByte();

				button->addButtonCondition(condition);

				if (!conditionLength)
					break;
			}
		}
	}

	context.movie->defineCharacter(buttonId, button);
	return true;
}

// ============================================================================
// JPEGTables

bool FlashTagJpegTables::read(SwfReader* swf, ReadContext& context)
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
	T_ASSERT (buffer[0] == 0xff && buffer[1] == 0xd8);

	MemoryStream bufferStream(buffer.ptr(), int(context.tagSize), true, false);
	context.jpegFormat->readJpegHeader(&bufferStream);

	return true;
}

// ============================================================================
// Define bits jpeg

FlashTagDefineBitsJpeg::FlashTagDefineBitsJpeg(int bitsType)
:	m_bitsType(bitsType)
{
}

bool FlashTagDefineBitsJpeg::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t bitmapId = bs.readUInt16();

	uint32_t tagSize = context.tagSize - sizeof(uint16_t) - sizeof(uint32_t);
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
	T_ASSERT (buffer[0] == 0xff && buffer[1] == 0xd8);
	MemoryStream bufferStream(buffer.ptr(), tagSize, true, false);

	// Decode JPEG image, either by using previously defined encoding tables
	// or, new method, with own encoding tables.
	if (m_bitsType == 1)
	{
		T_ASSERT (context.jpegFormat);

		Ref< drawing::Image > image = context.jpegFormat->readJpegImage(&bufferStream);
		T_ASSERT (image);

		Ref< FlashBitmapData > bitmap = new FlashBitmapData();
		if (!bitmap->create(image))
			return false;

		context.movie->defineBitmap(bitmapId, bitmap);
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
			uint32_t inflateSize = image->getWidth() * image->getHeight();

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
		{
			Ref< FlashBitmapData > bitmap = new FlashBitmapData();
			if (!bitmap->create(image))
				return false;

			context.movie->defineBitmap(bitmapId, bitmap);
		}
	}

	return true;
}

// ============================================================================
// Define bits loss less

FlashTagDefineBitsLossLess::FlashTagDefineBitsLossLess(int bitsType)
:	m_bitsType(bitsType)
{
}

bool FlashTagDefineBitsLossLess::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t bitmapId = bs.readUInt16();
	uint8_t format = bs.readUInt8();
	uint16_t width = bs.readUInt16();
	uint16_t height = bs.readUInt16();

	if (format == 3)	// Palette
	{
		uint8_t colorCount = bs.readUInt8() + 1;
		uint8_t colorSize = (m_bitsType == 1) ? 3 : 4;
		uint32_t colorTableSize = colorSize * colorCount;
		uint32_t imagePitch = (width + 3UL) & ~3UL;

		uint32_t bufferSize = colorTableSize + imagePitch * height;
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

		Ref< FlashBitmapData > bitmap = new FlashBitmapData();
		if (!bitmap->create(image))
			return false;

		context.movie->defineBitmap(bitmapId, bitmap);
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

		uint32_t bufferSize = imagePitch * height;
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
					image->setPixelUnsafe(x, y, Color4f(
						src[x * 4 + 1] / 255.0f,
						src[x * 4 + 2] / 255.0f,
						src[x * 4 + 3] / 255.0f,
						(m_bitsType > 1) ? src[x * 4 + 0] / 255.0f : 1.0f
					));
				}
			}
		}

		Ref< FlashBitmapData > bitmap = new FlashBitmapData();
		if (!bitmap->create(image))
			return false;

		context.movie->defineBitmap(bitmapId, bitmap);
	}

	return true;
}

// ============================================================================
// Define sprite

bool FlashTagDefineSprite::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t spriteId = bs.readUInt16();
	uint16_t frameRate = bs.readUInt16();

	Ref< FlashSprite > sprite = new FlashSprite(spriteId, frameRate);

	// Setup readers for supported sprite tags.
	std::map< uint16_t, Ref< FlashTag > > tagReaders;

	if (context.avm1 || context.avm2)
		tagReaders[TiDoAction] = new FlashTagDoAction();

	tagReaders[TiPlaceObject] = new FlashTagPlaceObject(1);
	tagReaders[TiPlaceObject2] = new FlashTagPlaceObject(2);
	tagReaders[TiPlaceObject3] = new FlashTagPlaceObject(3);
	tagReaders[TiRemoveObject] = new FlashTagRemoveObject(1);
	tagReaders[TiRemoveObject2] = new FlashTagRemoveObject(2);
	tagReaders[TiStartSound] = new FlashTagStartSound(1);
	tagReaders[TiStartSound2] = new FlashTagStartSound(2);
	tagReaders[TiShowFrame] = new FlashTagShowFrame();
	tagReaders[TiFrameLabel] = new FlashTagFrameLabel();
	tagReaders[TiDefineScalingGrid] = new FlashTagDefineScalingGrid();

	// Define readers for tags which isn't planed to be implemented.
	tagReaders[TiDefineFontInfo] = new FlashTagUnsupported(TiDefineFontInfo);
	tagReaders[TiDefineSound] = new FlashTagUnsupported(TiDefineSound);
	tagReaders[TiSoundStreamHead] = new FlashTagUnsupported(TiSoundStreamHead);
	tagReaders[TiSoundStreamBlock] = new FlashTagUnsupported(TiSoundStreamBlock);
	tagReaders[TiSoundStreamHead2] = new FlashTagUnsupported(TiSoundStreamHead2);
	tagReaders[TiDebugID] = new FlashTagUnsupported(TiDebugID);

	// Decode tags.
	FlashTag::ReadContext spriteContext;
	spriteContext.version = context.version;
	spriteContext.avm1 = context.avm1;
	spriteContext.avm2 = context.avm2;
	spriteContext.movie = context.movie;
	spriteContext.sprite = sprite;
	spriteContext.frame = new FlashFrame();
	for (;;)
	{
		swf->enterScope();

		SwfTag* tag = swf->readTag();
		if (!tag || tag->id == TiEnd)
			break;

		Ref< FlashTag > tagReader = tagReaders[tag->id];
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
		sprite->addFrame(new FlashFrame());

	context.movie->defineCharacter(spriteId, sprite);
	return true;
}

// ============================================================================
// Place object

FlashTagPlaceObject::FlashTagPlaceObject(int placeType)
:	m_placeType(placeType)
{
}

bool FlashTagPlaceObject::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	if (m_placeType == 1)
	{
		FlashFrame::PlaceObject placeObject;
		
		placeObject.hasFlags |= FlashFrame::PfHasCharacterId | FlashFrame::PfHasMatrix;
		placeObject.characterId = bs.readUInt16();
		placeObject.depth = bs.readUInt16();
		placeObject.matrix = Matrix33(swf->readMatrix().m);

		if (uint32_t(bs.getStream()->tell()) < context.tagEndPosition)
		{
			placeObject.hasFlags |= FlashFrame::PfHasCxTransform;
			placeObject.cxTransform = swf->readCxTransform(false);
		}

		context.frame->placeObject(placeObject);
	}
	else if (m_placeType == 2 || m_placeType == 3)
	{
		FlashFrame::PlaceObject placeObject;

		if (context.version >= 5)
			placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasActions : 0;
		else
			bs.skip(1);

		placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasClipDepth : 0;
		placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasName: 0;
		placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasRatio : 0;
		placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasCxTransform : 0;
		placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasMatrix : 0;
		placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasCharacterId : 0;
		placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasMove : 0;

		if (m_placeType == 2)
		{
			placeObject.depth = bs.readUInt16();
		}
		else if (m_placeType == 3)
		{
			bs.skip(1);

			placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasOpaqueBackground : 0;
			placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasVisible : 0;

			bool hasImage = bs.readBit();
			bool hasClassName = bs.readBit();

			placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasBitmapCaching : 0;
			placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasBlendMode : 0;
			placeObject.hasFlags |= bs.readBit() ? FlashFrame::PfHasFilters : 0;

			placeObject.depth = bs.readUInt16();

			if (hasClassName || (hasImage && placeObject.has(FlashFrame::PfHasCharacterId)))
			{
				std::string className = swf->readString();
				log::warning << L"Unused class name " << mbstows(className) << L" in PlaceObject" << Endl;
			}
		}
	
		if (placeObject.has(FlashFrame::PfHasCharacterId))
			placeObject.characterId = bs.readUInt16();

		if (placeObject.has(FlashFrame::PfHasMatrix))
			placeObject.matrix = Matrix33(swf->readMatrix().m);

		if (placeObject.has(FlashFrame::PfHasCxTransform))
			placeObject.cxTransform = swf->readCxTransform(true);

		if (placeObject.has(FlashFrame::PfHasRatio))
			placeObject.ratio = bs.readUInt16();

		if (placeObject.has(FlashFrame::PfHasName))
			placeObject.name = swf->readString();

		if (placeObject.has(FlashFrame::PfHasClipDepth))
			placeObject.clipDepth = bs.readUInt16();

		if (m_placeType == 3)
		{
			if (placeObject.has(FlashFrame::PfHasFilters))
			{
				AlignedVector< SwfFilter* > filterList;
				if (!swf->readFilterList(filterList))
					return false;

				if (!filterList.empty())
				{
					placeObject.filter = filterList.front()->filterId + 1;
					if (placeObject.filter == 1)	// Drop shadow
						placeObject.filterColor = filterList.front()->dropShadow.dropShadowColor;
					if (placeObject.filter == 3)	// Glow
						placeObject.filterColor = filterList.front()->glow.glowColor;
				}
			}

			if (placeObject.has(FlashFrame::PfHasBlendMode))
				placeObject.blendMode = bs.readUInt8();

			if (placeObject.has(FlashFrame::PfHasBitmapCaching))
				placeObject.bitmapCaching = bs.readUInt8();

			if (placeObject.has(FlashFrame::PfHasVisible))
				placeObject.visible = bs.readUInt8();
		}

		if (placeObject.has(FlashFrame::PfHasActions) && context.avm1)
		{
			uint16_t reserved = bs.readUInt16();
			if (reserved)
				log::warning << L"Reserved field in PlaceObject actions not zero, " << reserved << Endl;

			uint32_t allFlags;
			if (context.version >= 6)
				allFlags = bs.readUInt32();
			else
				allFlags = (bs.readUInt16() << 16);

			while (uint32_t(bs.getStream()->tell()) < context.tagEndPosition)
			{
				uint32_t eventMask = context.version >= 6 ? bs.readUInt32() : (bs.readUInt16() << 16);
				if (!eventMask)
					break;

				uint32_t eventLength = bs.readUInt32();
				if (uint32_t(bs.getStream()->tell()) + eventLength >= context.tagEndPosition)
				{
					log::error << L"PlaceObject, incorrect number of bytes in event tag" << Endl;
					break;
				}

				// "Key Press" events.
				if (eventMask & EvtKeyPress)
				{
					/*uint8_t keyCode = */bs.readUInt8();
					T_DEBUG(L"PlaceObject, unused keycode in EvtKeyPress");
				}

				Ref< const IActionVMImage > image = context.avm1->load(bs);
				bs.alignByte();

				placeObject.events.insert(std::make_pair(eventMask, image));
			}
		}

		context.frame->placeObject(placeObject);
	}

	return true;
}

// ============================================================================
// Remove object

FlashTagRemoveObject::FlashTagRemoveObject(int removeType)
:	m_removeType(removeType)
{
}

bool FlashTagRemoveObject::read(SwfReader* swf, ReadContext& context)
{
	FlashFrame::RemoveObject removeObject;
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

bool FlashTagShowFrame::read(SwfReader* swf, ReadContext& context)
{
	T_ASSERT (context.frame);
	context.sprite->addFrame(context.frame);
	context.frame = new FlashFrame();
	return true;
}

// ============================================================================
// Do action

bool FlashTagDoAction::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	Ref< const IActionVMImage > image = context.avm1->load(bs);
	if (image)
		context.frame->addActionScript(image);

	return true;
}

// ============================================================================
// Export assets

bool FlashTagExportAssets::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t exportCount = bs.readUInt16();
	for (int i = 0; i < exportCount; ++i)
	{
		uint16_t id = bs.readUInt16();
		std::string symbol = swf->readString();
		context.movie->setExport(symbol, id);
	}

	return true;
}

// ============================================================================
// Import assets

FlashTagImportAssets::FlashTagImportAssets(int importType)
:	m_importType(importType)
{
}

bool FlashTagImportAssets::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	std::string url = swf->readString();

	if (m_importType == 2)
	{
		/*uint8_t version = */bs.readUInt8();
		/*uint8_t reserved = */bs.readUInt8();
	}

	uint16_t count = bs.readUInt16();
	for (int i = 0; i < count; ++i)
	{
		uint16_t id = bs.readUInt16();
		std::string symbol = swf->readString();

		log::info << L"Import symbol \"" << mbstows(symbol) << L"\" as " << id << L" from URL \"" << mbstows(url) << L"\"" << Endl;
	}

	return true;
}

// ============================================================================
// Init action

bool FlashTagInitAction::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	/*uint16_t spriteId = */bs.readUInt16();

	Ref< const IActionVMImage > image = context.avm1->load(bs);
	bs.alignByte();

	context.sprite->addInitActionScript(image);
	return true;
}

// ============================================================================
// Protect

FlashTagProtect::FlashTagProtect(int protectType)
{
}

bool FlashTagProtect::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();
	bs.getStream()->seek(IStream::SeekSet, context.tagEndPosition);
	bs.alignByte();
	return true;
}

// ============================================================================
// Frame label

bool FlashTagFrameLabel::read(SwfReader* swf, ReadContext& context)
{
	std::string label = swf->readString();
	if (context.frame)
		context.frame->setLabel(label);
	return true;
}

// ============================================================================
// ABC

bool FlashTagDoABC::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint32_t flags = bs.readUInt32();
	std::string name = swf->readString();

	Ref< const IActionVMImage > image = context.avm2->load(bs);
	if (!image)
		return false;

	context.frame->addActionScript(image);
	return true;
}

// ============================================================================
// Define sound

bool FlashTagDefineSound::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	uint16_t soundId = bs.readUInt16();

	uint8_t soundFormat = bs.readUnsigned(4);
	if (soundFormat != 0 && soundFormat != 3)
	{
		log::error << L"Only uncompressed sounds are supported" << Endl;
		return false;
	}

	uint8_t soundRate = bs.readUnsigned(2);
	bool soundSize = bs.readBit();
	bool soundType = bs.readBit();
	uint32_t soundSampleCount = bs.readUInt32();

	uint8_t soundChannels = soundType ? 2 : 1;
	uint8_t soundSampleSize = soundSize ? 2 : 1;

	AutoArrayPtr< uint8_t > soundData(new uint8_t [soundSampleCount * soundSampleSize * soundChannels]);
	bs.getStream()->read(soundData.ptr(), soundSampleCount * soundSampleSize * soundChannels);
	bs.getStream()->seek(IStream::SeekSet, context.tagEndPosition);
	bs.alignByte();

	const uint32_t c_soundRates[] = { 5500, 11025, 22050, 44100 };
	T_ASSERT (soundRate < sizeof_array(c_soundRates));

	Ref< FlashSound > sound = new FlashSound();
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

FlashTagStartSound::FlashTagStartSound(int32_t startType)
:	m_startType(startType)
{
}

bool FlashTagStartSound::read(SwfReader* swf, ReadContext& context)
{
	BitReader& bs = swf->getBitReader();

	if (m_startType == 1)
	{
		uint16_t soundId = bs.readUInt16();

		SwfSoundInfo* soundInfo = swf->readSoundInfo();
		if (!soundInfo)
			return false;

		context.frame->startSound(soundId);
	}
	else
		// \fixme
		return false;

	return true;
}

// ============================================================================
// Unsupported

FlashTagUnsupported::FlashTagUnsupported(int32_t tagId)
:	m_tagId(tagId)
,	m_visited(false)
{
}

bool FlashTagUnsupported::read(SwfReader* swf, ReadContext& context)
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
}
