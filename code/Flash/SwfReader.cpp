#include <cstdlib>
#include <cstring>
#include "Compress/Zip/InflateStreamZip.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Memory/PoolAllocator.h"
#include "Core/Misc/Endian.h"
#include "Flash/SwfReader.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const uint32_t c_allocSize = 256UL * 1024UL;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SwfReader", SwfReader, Object)

SwfReader::SwfReader(IStream* stream)
:	m_stream(stream)
,	m_bs(new BitReader(stream))
,	m_pool(new PoolAllocator(c_allocSize))
{
}

void SwfReader::enterScope()
{
	m_pool->enter();
}

void SwfReader::leaveScope()
{
	m_pool->leave();
}

SwfHeader* SwfReader::readHeader()
{
	SwfHeader* header = m_pool->alloc< SwfHeader >();

	header->signature[0] = m_bs->readUInt8();
	header->signature[1] = m_bs->readUInt8();
	header->signature[2] = m_bs->readUInt8();
	header->version = m_bs->readUInt8();
	header->fileSize = m_bs->readUInt32();

	if (std::memcmp(header->signature, "FWS", 3) != 0 && std::memcmp(header->signature, "CWS", 3) != 0)
	{
		log::error << L"Unknown signature; invalid SWF" << Endl;
		return 0;
	}
	//if (header->version > 15)
	//{
	//	log::error << L"Unsupported version (" << int32_t(header->version) << L")" << Endl;
	//	return 0;
	//}

	if (header->signature[0] == 'C')
	{
		m_stream = new compress::InflateStreamZip(m_stream);
		m_bs = new BitReader(m_stream);
	}

	header->frameRect = readRect();
	header->frameRate = m_bs->readUInt16();
	header->frameCount = m_bs->readUInt16();

	return header;
}

SwfTag* SwfReader::readTag()
{
	SwfTag* tag = m_pool->alloc< SwfTag >();

	uint16_t code = m_bs->readUInt16();
	tag->id = code >> 6;
	tag->length = code & 0x3f;
	if (tag->length >= 0x3f)
		tag->length = m_bs->readUInt32();

	return tag;
}

SwfGradientRecord* SwfReader::readGradientRecord(int shapeType)
{
	SwfGradientRecord* gradientRecord = m_pool->alloc< SwfGradientRecord >();

	gradientRecord->ratio = m_bs->readUInt8();
	if (shapeType == 1 || shapeType == 2)
		gradientRecord->color = readRgb();
	else if (shapeType == 3 || shapeType == 4)
		gradientRecord->color = readRgba();

	return gradientRecord;
}

SwfGradient* SwfReader::readGradient(int shapeType)
{
	SwfGradient* gradient = m_pool->alloc< SwfGradient >();

	m_bs->alignByte();

	/*uint32_t spreadMode = */m_bs->readUnsigned(2);
	/*uint32_t interpolationMode = */m_bs->readUnsigned(2);

	gradient->numGradientRecords = m_bs->readUnsigned(4);
	gradient->gradientRecords = m_pool->allocArray< SwfGradientRecord >(gradient->numGradientRecords);
	for (int i = 0; i < gradient->numGradientRecords; ++i)
		gradient->gradientRecords[i] = readGradientRecord(shapeType);

	gradient->focalPoint = 0.0f;
	return gradient;
}

SwfGradient* SwfReader::readFocalGradient(int shapeType)
{
	SwfGradient* gradient = m_pool->alloc< SwfGradient >();

	m_bs->alignByte();

	/*uint32_t spreadMode = */m_bs->readUnsigned(2);
	/*uint32_t interpolationMode = */m_bs->readUnsigned(2);

	gradient->numGradientRecords = m_bs->readUnsigned(4);
	gradient->gradientRecords = m_pool->allocArray< SwfGradientRecord >(gradient->numGradientRecords);
	for (int i = 0; i < gradient->numGradientRecords; ++i)
		gradient->gradientRecords[i] = readGradientRecord(shapeType);

	gradient->focalPoint = m_bs->readUInt16() / 256.0f;
	return gradient;
}

SwfFillStyle* SwfReader::readFillStyle(int shapeType)
{
	SwfFillStyle* fillStyle = m_pool->alloc< SwfFillStyle >();

	fillStyle->type = m_bs->readUInt8();
	if (fillStyle->type == FstSolid)
	{
		if (shapeType == 1 || shapeType == 2)
			readRgb().storeUnaligned(fillStyle->solid.color);
		else if (shapeType == 3 || shapeType == 4)
			readRgba().storeUnaligned(fillStyle->solid.color);
	}
	else if (fillStyle->type == FstLinearGradient || fillStyle->type == FstRadialGradient)
	{
		fillStyle->gradient.gradientMatrix = readMatrix();
		fillStyle->gradient.gradient = readGradient(shapeType);
	}
	else if (fillStyle->type == FstFocalGradient)
	{
		fillStyle->gradient.gradientMatrix = readMatrix();
		fillStyle->gradient.gradient = readFocalGradient(shapeType);
	}
	else if (
		fillStyle->type == FstTiledBitmap || fillStyle->type == FstClippedBitmap ||
		fillStyle->type == FstTiledBitmapHard || fillStyle->type == FstClippedBitmapHard
	)
	{
		fillStyle->bitmap.bitmapId = m_bs->readUInt16();
		fillStyle->bitmap.bitmapMatrix = readMatrix();
	}
	else
	{
		log::error << L"Invalid fill type (" << int32_t(fillStyle->type) << L")" << Endl;
		return 0;
	}

	return fillStyle;
}

SwfLineStyle* SwfReader::readLineStyle(int shapeType)
{
	SwfLineStyle* lineStyle = m_pool->alloc< SwfLineStyle >();

	lineStyle->width = m_bs->readUInt16();
	if (shapeType == 1 || shapeType == 2)
		lineStyle->color = readRgb();
	else if (shapeType == 3)
		lineStyle->color = readRgba();
	else if (shapeType == 4)
	{
		/*uint16_t startCapStyle = */m_bs->readUnsigned(2);
		uint16_t joinStyle = m_bs->readUnsigned(2);
		bool hasFillFlag = m_bs->readBit();
		/*bool noHScaleFlag = */m_bs->readBit();
		/*bool noVScaleFlag = */m_bs->readBit();
		/*bool pixelHintingFlag = */m_bs->readBit();

		m_bs->skip(5);

		/*bool noClose = */m_bs->readBit();
		/*uint16_t endCapStyle = */m_bs->readUnsigned(2);

		m_bs->alignByte();

		if (joinStyle == 2)
		{
			/*uint16_t miterLimitFactor = */m_bs->readUInt16();
		}

		if (!hasFillFlag)
			lineStyle->color = readRgba();
		else
		{
			SwfFillStyle* fill = readFillStyle(shapeType);
			lineStyle->color = Color4f::loadUnaligned(fill->solid.color);
		}
	}

	return lineStyle;
}

SwfStyles* SwfReader::readStyles(int shapeType)
{
	SwfStyles* styles = m_pool->alloc< SwfStyles >();

	styles->numFillStyles = m_bs->readUInt8();
	if ((shapeType == 3 || shapeType == 4) && styles->numFillStyles == 0xff)
		styles->numFillStyles = m_bs->readUInt16();

	styles->fillStyles = m_pool->allocArray< SwfFillStyle >(styles->numFillStyles);
	for (uint16_t i = 0; i < styles->numFillStyles; ++i)
		styles->fillStyles[i] = readFillStyle(shapeType);

	styles->numLineStyles = m_bs->readUInt8();
	if (styles->numLineStyles == 0xff)
		styles->numLineStyles = m_bs->readUInt16();

	styles->lineStyles = m_pool->allocArray< SwfLineStyle >(styles->numLineStyles);
	for (uint16_t i = 0; i < styles->numLineStyles; ++i)
		styles->lineStyles[i] = readLineStyle(shapeType);

	m_bs->alignByte();
	styles->numFillBits = m_bs->readUnsigned(4);
	styles->numLineBits = m_bs->readUnsigned(4);

	return styles;
}

bool SwfReader::readMorphGradientRecord(SwfGradientRecord*& outStartGradientRecord, SwfGradientRecord*& outEndGradientRecord)
{
	outStartGradientRecord = m_pool->alloc< SwfGradientRecord >();
	outStartGradientRecord->ratio = m_bs->readUInt8();
	outStartGradientRecord->color = readRgba();

	outEndGradientRecord = m_pool->alloc< SwfGradientRecord >();
	outEndGradientRecord->ratio = m_bs->readUInt8();
	outEndGradientRecord->color = readRgba();

	return true;
}

bool SwfReader::readMorphGradient(SwfGradient*& outStartGradient, SwfGradient*& outEndGradient)
{
	uint8_t numGradientRecords = m_bs->readUInt8();
	
	outStartGradient = m_pool->alloc< SwfGradient >();
	outStartGradient->numGradientRecords = numGradientRecords;
	outStartGradient->gradientRecords = m_pool->allocArray< SwfGradientRecord >(numGradientRecords);

	outEndGradient = m_pool->alloc< SwfGradient >();
	outEndGradient->numGradientRecords = numGradientRecords;
	outEndGradient->gradientRecords = m_pool->allocArray< SwfGradientRecord >(numGradientRecords);

	for (int i = 0; i < numGradientRecords; ++i)
	{
		if (!readMorphGradientRecord(outStartGradient->gradientRecords[i], outEndGradient->gradientRecords[i]))
			return false;
	}

	return true;
}

bool SwfReader::readMorphFillStyle(SwfFillStyle*& outStartFillStyle, SwfFillStyle*& outEndFillStyle, int shapeType)
{
	outStartFillStyle = m_pool->alloc< SwfFillStyle >();
	outEndFillStyle = m_pool->alloc< SwfFillStyle >();

	outStartFillStyle->type =
	outEndFillStyle->type = m_bs->readUInt8();

	if (outStartFillStyle->type == FstSolid)
	{
		readRgba().storeUnaligned(outStartFillStyle->solid.color);
		readRgba().storeUnaligned(outEndFillStyle->solid.color);
	}
	else if (outStartFillStyle->type == FstLinearGradient || outStartFillStyle->type == FstRadialGradient)
	{
		outStartFillStyle->gradient.gradientMatrix = readMatrix();
		outEndFillStyle->gradient.gradientMatrix = readMatrix();
		if (!readMorphGradient(outStartFillStyle->gradient.gradient, outEndFillStyle->gradient.gradient))
			return false;
	}
	//else if (outStartFillStyle->type == FstFocalGradient)
	//{
	//	readGradient(fillStyle->gradient.gradient, shapeType);
	//}
	else if (
		outStartFillStyle->type == FstTiledBitmap || outStartFillStyle->type == FstClippedBitmap ||
		outStartFillStyle->type == FstTiledBitmapHard || outStartFillStyle->type == FstClippedBitmapHard
	)
	{
		outStartFillStyle->bitmap.bitmapId =
		outEndFillStyle->bitmap.bitmapId = m_bs->readUInt16();
		outStartFillStyle->bitmap.bitmapMatrix = readMatrix();
		outEndFillStyle->bitmap.bitmapMatrix = readMatrix();
	}
	else
	{
		log::error << L"Invalid fill type (" << outStartFillStyle->type << L")" << Endl;
		return false;
	}

	return true;
}

bool SwfReader::readMorphLineStyle(SwfLineStyle*& outStartLineStyle, SwfLineStyle*& outEndLineStyle, int shapeType)
{
	outStartLineStyle = m_pool->alloc< SwfLineStyle >();
	outEndLineStyle = m_pool->alloc< SwfLineStyle >();

	outStartLineStyle->width = m_bs->readUInt16();
	outEndLineStyle->width = m_bs->readUInt16();

	bool hasFillFlag = false;

	if (shapeType == 2)
	{
		/*uint8_t startCapStyle = */m_bs->readUnsigned(2);
		uint8_t joinStyle = m_bs->readUnsigned(2);
		hasFillFlag = m_bs->readBit();
		/*bool hasHScaleFlag = */m_bs->readBit();
		/*bool hasVScaleFlag = */m_bs->readBit();
		/*bool pixelHintingFlag = */m_bs->readBit();
		m_bs->skip(5);
		/*bool noClose = */m_bs->readBit();
		/*uint8_t endCapStyle = */m_bs->readUnsigned(2);

		if (joinStyle == 2)
			/*uint16_t miterLimitFactor = */m_bs->readUInt16();
	}

	if (!hasFillFlag)
	{
		outStartLineStyle->color = readRgba();
		outEndLineStyle->color = readRgba();
	}
	else
	{
		SwfFillStyle *startFillStyle, *endFillStyle;
		if (!readMorphFillStyle(startFillStyle, endFillStyle, shapeType))
			return false;

		// @fixme As for now we just assume solid fill style and grab the color.
		outStartLineStyle->color = Color4f::loadUnaligned(startFillStyle->solid.color);
		outEndLineStyle->color = Color4f::loadUnaligned(endFillStyle->solid.color);
	}

	return true;
}

bool SwfReader::readMorphStyles(SwfStyles*& outStartStyles, SwfStyles*& outEndStyles, int shapeType)
{
	uint16_t numFillStyles = m_bs->readUInt8();
	if (numFillStyles == 0xff)
		numFillStyles = m_bs->readUInt16();

	outStartStyles = m_pool->alloc< SwfStyles >();
	outStartStyles->numFillStyles = numFillStyles;
	outStartStyles->fillStyles = m_pool->allocArray< SwfFillStyle >(numFillStyles);

	outEndStyles = m_pool->alloc< SwfStyles >();
	outEndStyles->numFillStyles = numFillStyles;
	outEndStyles->fillStyles = m_pool->allocArray< SwfFillStyle >(numFillStyles);

	for (uint16_t i = 0; i < numFillStyles; ++i)
	{
		if (!readMorphFillStyle(outStartStyles->fillStyles[i], outEndStyles->fillStyles[i], shapeType))
			return false;
	}
	
	uint16_t numLineStyles = m_bs->readUInt8();
	if (numLineStyles == 0xff)
		numLineStyles = m_bs->readUInt16();

	outStartStyles->numLineStyles = numLineStyles;
	outStartStyles->lineStyles = m_pool->allocArray< SwfLineStyle >(numLineStyles);
	
	outEndStyles->numLineStyles = numLineStyles;
	outEndStyles->lineStyles = m_pool->allocArray< SwfLineStyle >(numLineStyles);

	for (uint16_t i = 0; i < numLineStyles; ++i)
	{
		if (!readMorphLineStyle(outStartStyles->lineStyles[i], outEndStyles->lineStyles[i], shapeType))
			return false;
	}

	return true;
}

SwfShapeRecord* SwfReader::readShapeRecord(uint32_t numFillBits, uint32_t numLineBits, int shapeType)
{
	SwfShapeRecord* shapeRecord = m_pool->alloc< SwfShapeRecord >();
	std::memset(shapeRecord, 0, sizeof(SwfShapeRecord));

	shapeRecord->edgeFlag = m_bs->readBit();
	if (!shapeRecord->edgeFlag)
	{
		shapeRecord->style.stateNewStyles = m_bs->readBit();
		shapeRecord->style.stateLineStyle = m_bs->readBit();
		shapeRecord->style.stateFillStyle1 = m_bs->readBit();
		shapeRecord->style.stateFillStyle0 = m_bs->readBit();
		shapeRecord->style.stateMoveTo = m_bs->readBit();

		if (shapeRecord->style.stateMoveTo)
		{
			uint32_t nbits = m_bs->readUnsigned(5);
			shapeRecord->style.moveDeltaX = m_bs->readSigned(nbits);
			shapeRecord->style.moveDeltaY = m_bs->readSigned(nbits);
		}

		if (shapeRecord->style.stateFillStyle0 && numFillBits > 0)
			shapeRecord->style.fillStyle0 = m_bs->readUnsigned(numFillBits);
		if (shapeRecord->style.stateFillStyle1 && numFillBits > 0)
			shapeRecord->style.fillStyle1 = m_bs->readUnsigned(numFillBits);
		if (shapeRecord->style.stateLineStyle && numLineBits > 0)
			shapeRecord->style.lineStyle = m_bs->readUnsigned(numLineBits);

		if (shapeRecord->style.stateNewStyles)
		{
			T_ASSERT (shapeType != 1);
			shapeRecord->style.newStyles = readStyles(shapeType);
		}
	}
	else
	{
		shapeRecord->edge.straightFlag = m_bs->readBit();
		if (shapeRecord->edge.straightFlag)
		{
			uint32_t nbits = m_bs->readUnsigned(4) + 2;

			shapeRecord->edge.straightEdge.generalLineFlag = m_bs->readBit();
			if (shapeRecord->edge.straightEdge.generalLineFlag)
			{
				shapeRecord->edge.straightEdge.deltaX = m_bs->readSigned(nbits);
				shapeRecord->edge.straightEdge.deltaY = m_bs->readSigned(nbits);
			}
			else
			{
				shapeRecord->edge.straightEdge.vertLineFlag = m_bs->readBit();
				if (!shapeRecord->edge.straightEdge.vertLineFlag)
					shapeRecord->edge.straightEdge.deltaX = m_bs->readSigned(nbits);
				else
					shapeRecord->edge.straightEdge.deltaY = m_bs->readSigned(nbits);
			}
		}
		else
		{
			uint32_t nbits = m_bs->readUnsigned(4) + 2;

			shapeRecord->edge.curvedEdge.controlDeltaX = m_bs->readSigned(nbits);
			shapeRecord->edge.curvedEdge.controlDeltaY = m_bs->readSigned(nbits);
			shapeRecord->edge.curvedEdge.anchorDeltaX = m_bs->readSigned(nbits);
			shapeRecord->edge.curvedEdge.anchorDeltaY = m_bs->readSigned(nbits);
		}
	}

	return shapeRecord;
}

bool SwfReader::readShapeWithStyle(SwfShape*& outShape, SwfStyles*& outStyles, int shapeType)
{
	outStyles = readStyles(shapeType);
	if (!outStyles)
		return false;

	outShape = m_pool->alloc< SwfShape >();
	outShape->shapeRecords = 0;

	SwfShapeRecord* p = 0;
	for (;;)
	{
		SwfShapeRecord* shapeRecord = readShapeRecord(
			outStyles->numFillBits,
			outStyles->numLineBits,
			shapeType
		);
		if (!shapeRecord)
			return false;

		if (
			!shapeRecord->edgeFlag &&
			!shapeRecord->style.stateNewStyles &&
			!shapeRecord->style.stateLineStyle &&
			!shapeRecord->style.stateFillStyle1 && 
			!shapeRecord->style.stateFillStyle0 &&
			!shapeRecord->style.stateMoveTo
		)
			break;

		if (p)
			p->next = shapeRecord;
		else
			outShape->shapeRecords = shapeRecord;

		p = shapeRecord;

		if (!shapeRecord->edgeFlag && shapeRecord->style.stateNewStyles)
		{
			outStyles->numFillBits = shapeRecord->style.newStyles->numFillBits;
			outStyles->numLineBits = shapeRecord->style.newStyles->numLineBits;
		}
	}

	return true;
}

SwfShape* SwfReader::readShape(int shapeType)
{
	uint32_t numFillBits = m_bs->readUnsigned(4);
	uint32_t numLineBits = m_bs->readUnsigned(4);

	SwfShape* shape = m_pool->alloc< SwfShape >();
	shape->shapeRecords = 0;

	SwfShapeRecord* p = 0;
	for (;;)
	{
		SwfShapeRecord* shapeRecord = readShapeRecord(numFillBits, numLineBits, shapeType);
		if (!shapeRecord)
			return 0;

		if (
			!shapeRecord->edgeFlag &&
			!shapeRecord->style.stateNewStyles &&
			!shapeRecord->style.stateLineStyle &&
			!shapeRecord->style.stateFillStyle1 && 
			!shapeRecord->style.stateFillStyle0 &&
			!shapeRecord->style.stateMoveTo
		)
			break;

		if (p)
			p->next = shapeRecord;
		else
			shape->shapeRecords = shapeRecord;

		p = shapeRecord;
	}

	return shape;
}

SwfTextRecord* SwfReader::readTextRecord(uint8_t numGlyphBits, uint8_t numAdvanceBits, int textType)
{
	SwfTextRecord* textRecord = m_pool->alloc< SwfTextRecord >();

	textRecord->styleFlag = m_bs->readBit();
	if (textRecord->styleFlag)
	{
		m_bs->skip(3);

		textRecord->style.hasFont = m_bs->readBit();
		textRecord->style.hasColor = m_bs->readBit();
		textRecord->style.hasYOffset = m_bs->readBit();
		textRecord->style.hasXOffset = m_bs->readBit();

		if (textRecord->style.hasFont)
			textRecord->style.fontId = m_bs->readUInt16();
		if (textRecord->style.hasColor)
		{
			if (textType == 2)
				readRgba().storeUnaligned(textRecord->style.color);
			else
				readRgb().storeUnaligned(textRecord->style.color);
		}
		if (textRecord->style.hasXOffset)
			textRecord->style.XOffset = m_bs->readInt16();
		if (textRecord->style.hasYOffset)
			textRecord->style.YOffset = m_bs->readInt16();
		if (textRecord->style.hasFont)
			textRecord->style.height = m_bs->readUInt16();
	}
	else
	{
		textRecord->glyph.glyphCount = m_bs->readUnsigned(7);
		textRecord->glyph.glyphEntries = m_pool->allocArray< SwfGlyphEntry >(textRecord->glyph.glyphCount);
		for (uint8_t i = 0; i < textRecord->glyph.glyphCount; ++i)
		{
			textRecord->glyph.glyphEntries[i] = m_pool->alloc< SwfGlyphEntry >();
			textRecord->glyph.glyphEntries[i]->glyphIndex = m_bs->readUnsigned(numGlyphBits);
			textRecord->glyph.glyphEntries[i]->glyphAdvance = m_bs->readSigned(numAdvanceBits);
		}
	}

	m_bs->alignByte();
	return textRecord;
}

SwfFilter* SwfReader::readFilter()
{
	SwfFilter* filter = m_pool->alloc< SwfFilter >();

	m_bs->alignByte();

	filter->filterId = m_bs->readUInt8();
	switch (filter->filterId)
	{
	case 0:
		{
			readRgba().storeUnaligned(filter->dropShadow.dropShadowColor);
			filter->dropShadow.blurX = readFixed();
			filter->dropShadow.blurY = readFixed();
			filter->dropShadow.angle = readFixed();
			filter->dropShadow.distance = readFixed();
			filter->dropShadow.strength = readFixed8();
			filter->dropShadow.innerShadow = m_bs->readBit();
			filter->dropShadow.knockOut = m_bs->readBit();
			filter->dropShadow.compositeSource = m_bs->readBit();
			filter->dropShadow.passes = m_bs->readUnsigned(5);
		}
		break;
	case 1:
		{
			filter->blur.blurX = readFixed();
			filter->blur.blurY = readFixed();
			filter->blur.passes = m_bs->readUnsigned(5);
			m_bs->skip(3);
		}
		break;
	case 2:
		{
			readRgba().storeUnaligned(filter->glow.glowColor);
			filter->glow.blurX = readFixed();
			filter->glow.blurY = readFixed();
			filter->glow.strength = readFixed8();
			filter->glow.innerGlow = m_bs->readBit();
			filter->glow.knockOut = m_bs->readBit();
			filter->glow.compositeSource = m_bs->readBit();
			filter->glow.passes = m_bs->readUnsigned(5);
		}
		break;
	case 3:
		{
			readRgba().storeUnaligned(filter->bevel.shadowColor);
			readRgba().storeUnaligned(filter->bevel.highlightColor);
			filter->bevel.blurX = readFixed();
			filter->bevel.blurY = readFixed();
			filter->bevel.angle = readFixed();
			filter->bevel.distance = readFixed();
			filter->bevel.strength = readFixed8();
			filter->bevel.innerShadow = m_bs->readBit();
			filter->bevel.knockOut = m_bs->readBit();
			filter->bevel.compositeSource = m_bs->readBit();
			filter->bevel.onTop = m_bs->readBit();
			filter->bevel.passes = m_bs->readUnsigned(4);
		}
		break;
		//case 4:
		//	break;
	case 5:
		{
			filter->convolution.matrixX = m_bs->readUInt8();
			filter->convolution.matrixY = m_bs->readUInt8();
			filter->convolution.divisor = readFloat32();
			filter->convolution.bias = readFloat32();
			filter->convolution.matrix = m_pool->alloc< float >(filter->convolution.matrixX * filter->convolution.matrixY);
			for (int i = 0; i < filter->convolution.matrixX * filter->convolution.matrixY; ++i)
				filter->convolution.matrix[i] = readFloat32();
			readRgba().storeUnaligned(filter->convolution.defaultColor);
			m_bs->skip(6);
			filter->convolution.clamp = m_bs->readBit();
			filter->convolution.preserveAlpha = m_bs->readBit();
		}
		break;
	case 6:
		{
			for (int i = 0; i < 20; ++i)
				filter->colorMatrix.matrix[i] = readFloat32();
		}
		break;
		//case 7:
		//	break;
	default:
		log::error << L"Unknown filter type " << filter->filterId << Endl;
		return 0;
	}
	return filter;
}

bool SwfReader::readFilterList(AlignedVector< SwfFilter* >& outFilterList)
{
	uint8_t filterCount = m_bs->readUInt8();

	outFilterList.resize(filterCount);
	for (int i = 0; i < filterCount; ++i)
	{
		outFilterList[i] = readFilter();
		if (!outFilterList[i])
			return false;
	}

	return true;
}

SwfSoundInfo* SwfReader::readSoundInfo()
{
	SwfSoundInfo* info = m_pool->alloc< SwfSoundInfo >();

	m_bs->skip(2);

	info->syncStop = m_bs->readBit();
	info->syncNoMultiple = m_bs->readBit();

	bool hasEnvelope = m_bs->readBit();
	bool hasLoops = m_bs->readBit();
	bool hasOutPoint = m_bs->readBit();
	bool hasInPoint = m_bs->readBit();
	
	if (hasInPoint)
		{ uint32_t inPoint = m_bs->readUInt32(); }

	if (hasOutPoint)
		{ uint32_t outPoint = m_bs->readUInt32(); }

	if (hasLoops)
		{ uint16_t loops = m_bs->readUInt16(); }

	if (hasEnvelope)
	{
		uint8_t envPoints = m_bs->readUInt8();
		for (int i = 0; i < envPoints; ++i)
		{
			uint32_t pos44 = m_bs->readUInt32();
			uint16_t leftLevel = m_bs->readUInt16();
			uint16_t rightLevel = m_bs->readUInt16();
		}
	}

	return info;
}

ColorTransform SwfReader::readCxTransform(bool withAlpha)
{
	ColorTransform cxTransform;

	cxTransform.mul = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	cxTransform.add = Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	m_bs->alignByte();

	bool hasAddTerms = m_bs->readBit();
	bool hasMulTerms = m_bs->readBit();
	uint32_t nbits = m_bs->readUnsigned(4);

	if (hasMulTerms)
	{
		int16_t redMul = m_bs->readSigned(nbits);
		int16_t greenMul = m_bs->readSigned(nbits);
		int16_t blueMul = m_bs->readSigned(nbits);
		int16_t alphaMul = withAlpha ? m_bs->readSigned(nbits) : (1 << 8);

		cxTransform.mul = Color4f(
			redMul / 255.0f,
			greenMul / 255.0f,
			blueMul / 255.0f,
			alphaMul / 255.0f
		);
	}

	if (hasAddTerms)
	{
		int16_t redAdd = m_bs->readSigned(nbits);
		int16_t greenAdd = m_bs->readSigned(nbits);
		int16_t blueAdd = m_bs->readSigned(nbits);
		int16_t alphaAdd = withAlpha ? m_bs->readSigned(nbits) : 0;

		cxTransform.add = Color4f(
			redAdd / 255.0f,
			greenAdd / 255.0f,
			blueAdd / 255.0f,
			alphaAdd / 255.0f
		);
	}

	return cxTransform;
}

SwfKerningRecord SwfReader::readKerningRecord(bool wideCodes)
{
	SwfKerningRecord kerningRecord;

	kerningRecord.code1 = wideCodes ? m_bs->readUInt16() : m_bs->readUInt8();
	kerningRecord.code2 = wideCodes ? m_bs->readUInt16() : m_bs->readUInt8();
	kerningRecord.adjustment = m_bs->readInt16();

	return kerningRecord;
}

Aabb2 SwfReader::readRect()
{
	Aabb2 rect;

	m_bs->alignByte();

	uint32_t nbits = m_bs->readUnsigned(5);
	rect.mn.x = float(m_bs->readSigned(nbits));
	rect.mx.x = float(m_bs->readSigned(nbits));
	rect.mn.y = float(m_bs->readSigned(nbits));
	rect.mx.y = float(m_bs->readSigned(nbits));

	m_bs->alignByte();

	return rect;
}

Color4f SwfReader::readRgb()
{
	uint8_t r = m_bs->readUInt8();
	uint8_t g = m_bs->readUInt8();
	uint8_t b = m_bs->readUInt8();
	uint8_t a = 255;
	return Color4f(r, g, b, a) / Scalar(255.0f);
}

Color4f SwfReader::readRgba()
{
	uint8_t r = m_bs->readUInt8();
	uint8_t g = m_bs->readUInt8();
	uint8_t b = m_bs->readUInt8();
	uint8_t a = m_bs->readUInt8();
	return Color4f(r, g, b, a) / Scalar(255.0f);
}

SwfMatrix SwfReader::readMatrix()
{
	int scaleX, scaleY;
	int rotateSkew0, rotateSkew1;
	int translateX, translateY;

	m_bs->alignByte();

	bool hasScale = m_bs->readBit();
	if (hasScale)
	{
		uint32_t nbits = m_bs->readUnsigned(5);
		scaleX = m_bs->readSigned(nbits);
		scaleY = m_bs->readSigned(nbits);
	}
	else
		scaleX = scaleY = 1 << 16;

	bool hasRotate = m_bs->readBit();
	if (hasRotate)
	{
		uint32_t nbits = m_bs->readUnsigned(5);
		rotateSkew0 = m_bs->readSigned(nbits);
		rotateSkew1 = m_bs->readSigned(nbits);
	}
	else
		rotateSkew0 = rotateSkew1 = 0;

	uint32_t nbits = m_bs->readUnsigned(5);
	translateX = m_bs->readSigned(nbits);
	translateY = m_bs->readSigned(nbits);

	m_bs->alignByte();

	SwfMatrix matrix;
	matrix.m[0] = scaleX / 65536.0f;		matrix.m[1] = rotateSkew1 / 65536.0f;	matrix.m[2] = float(translateX);
	matrix.m[3] = rotateSkew0 / 65536.0f;	matrix.m[4] = scaleY / 65536.0f;		matrix.m[5] = float(translateY);
	matrix.m[6] = 0.0f;						matrix.m[7] = 0.0f;						matrix.m[8] = 1.0f;

	return matrix;
}

std::string SwfReader::readStringU8()
{
	uint8_t len = m_bs->readUInt8();

	std::vector< uint8_t > buf(len);
	for (uint8_t i = 0; i < len; ++i)
		buf[i] = m_bs->readUInt8();

	return std::string(buf.begin(), buf.end());
}

std::string SwfReader::readString()
{
	std::vector< uint8_t > buf;
	uint8_t ch;

	while ((ch = m_bs->readUInt8()) != 0)
		buf.push_back(ch);

	return std::string(buf.begin(), buf.end());
}

float SwfReader::readFloat32()
{
	uint32_t flt = m_bs->readUInt32();
	return *reinterpret_cast< float* >(&flt);
}

float SwfReader::readFixed()
{
	int32_t fixed = m_bs->readInt32();
	return float(fixed) / 65536.0f;
}

float SwfReader::readFixed8()
{
	int16_t fixed = m_bs->readInt16();
	return float(fixed) / 256.0f;
}

uint16_t SwfReader::readU16BE()
{
	uint16_t v = m_bs->readUnsigned(16);
#if defined(T_LITTLE_ENDIAN)
	swap8in32(v);
#endif
	return v;
}

uint32_t SwfReader::readEncodedU30()
{
	return readEncodedU32();
}

uint32_t SwfReader::readEncodedU32()
{
	uint32_t v = 0;
	uint32_t b;
	
	b = m_bs->readUInt8();
	v = b;
	if ((v & 0x00000080) == 0)
		return v;

	b = m_bs->readUInt8();
	v = (v & 0x0000007f) | (b << 7);
	if ((v & 0x00004000) == 0)
		return v;

	b = m_bs->readUInt8();
	v = (v & 0x00003fff) | (b << 14);
	if ((v & 0x00200000) == 0)
		return v;

	b = m_bs->readUInt8();
	v = (v & 0x001fffff) | (b << 21);
	if ((v & 0x10000000) == 0)
		return v;

	b = m_bs->readUInt8();
	v = (v & 0x0fffffff) | (b << 28);
	return v;
}

int32_t SwfReader::readEncodedS32()
{
	uint32_t v = readEncodedU32();
	return *(int32_t*)&v;
}

BitReader& SwfReader::getBitReader()
{
	return *m_bs;
}

	}
}
