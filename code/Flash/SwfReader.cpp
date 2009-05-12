#include <cstdlib>
#include "Flash/SwfReader.h"
#include "Core/Io/Stream.h"
#include "Zip/InflateStream.h"
#include "Core/Heap/HeapNew.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const uint32_t c_allocSize = 1UL * 1024UL * 1024UL;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SwfReader", SwfReader, Object)

SwfReader::SwfReader(Stream* stream)
:	m_stream(stream)
,	m_bs(gc_new< BitReader >(stream))
{
	m_allocHead = static_cast< uint8_t* >(malloc(c_allocSize));
	T_ASSERT (m_allocHead);

	m_allocTail = m_allocHead;

	std::memset(m_allocHead, 0, c_allocSize);
}

SwfReader::~SwfReader()
{
	T_EXCEPTION_GUARD_BEGIN

	free(m_allocHead);

	T_EXCEPTION_GUARD_END
}

void SwfReader::enterScope()
{
	m_scope.push(m_allocTail);
}

void SwfReader::leaveScope()
{
	m_allocTail = m_scope.top();
	m_scope.pop();
}

SwfHeader* SwfReader::readHeader()
{
	SwfHeader* header = alloc< SwfHeader >();

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
	if (header->version > 10)
	{
		log::error << L"Unsupported version (" << int32_t(header->version) << L")" << Endl;
		return 0;
	}

	if (header->signature[0] == 'C')
	{
		m_stream = gc_new< zip::InflateStream >(m_stream);
		m_bs = gc_new< BitReader >(m_stream);
	}

	header->frameRect = readRect();
	header->frameRate = m_bs->readUInt16();
	header->frameCount = m_bs->readUInt16();

	return header;
}

SwfTag* SwfReader::readTag()
{
	SwfTag* tag = alloc< SwfTag >();

	uint16_t code = m_bs->readUInt16();
	tag->id = code >> 6;
	tag->length = code & 0x3f;
	if (tag->length >= 0x3f)
		tag->length = m_bs->readUInt32();

	return tag;
}

SwfGradientRecord* SwfReader::readGradientRecord(int shapeType)
{
	SwfGradientRecord* gradientRecord = alloc< SwfGradientRecord >();

	gradientRecord->ratio = m_bs->readUInt8();
	if (shapeType == 1 || shapeType == 2)
		gradientRecord->color = readRgb();
	else if (shapeType == 3 || shapeType == 4)
		gradientRecord->color = readRgba();

	return gradientRecord;
}

SwfGradient* SwfReader::readGradient(int shapeType)
{
	SwfGradient* gradient = alloc< SwfGradient >();

	m_bs->alignByte();

	uint32_t spreadMode = m_bs->readUnsigned(2);
	uint32_t interpolationMode = m_bs->readUnsigned(2);

	gradient->numGradientRecords = m_bs->readUnsigned(4);
	gradient->gradientRecords = allocArray< SwfGradientRecord >(gradient->numGradientRecords);
	for (int i = 0; i < gradient->numGradientRecords; ++i)
		gradient->gradientRecords[i] = readGradientRecord(shapeType);

	gradient->focalPoint = 0.0f;
	return gradient;
}

SwfGradient* SwfReader::readFocalGradient(int shapeType)
{
	SwfGradient* gradient = alloc< SwfGradient >();

	m_bs->alignByte();

	uint32_t spreadMode = m_bs->readUnsigned(2);
	uint32_t interpolationMode = m_bs->readUnsigned(2);

	gradient->numGradientRecords = m_bs->readUnsigned(4);
	gradient->gradientRecords = allocArray< SwfGradientRecord >(gradient->numGradientRecords);
	for (int i = 0; i < gradient->numGradientRecords; ++i)
		gradient->gradientRecords[i] = readGradientRecord(shapeType);

	gradient->focalPoint = m_bs->readUInt16() / 256.0f;
	return gradient;
}

SwfFillStyle* SwfReader::readFillStyle(int shapeType)
{
	SwfFillStyle* fillStyle = alloc< SwfFillStyle >();

	fillStyle->type = m_bs->readUInt8();
	if (fillStyle->type == FstSolid)
	{
		if (shapeType == 1 || shapeType == 2)
			fillStyle->solid.color = readRgb();
		else if (shapeType == 3 || shapeType == 4)
			fillStyle->solid.color = readRgba();
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
	SwfLineStyle* lineStyle = alloc< SwfLineStyle >();

	lineStyle->width = m_bs->readUInt16();
	if (shapeType == 1 || shapeType == 2)
		lineStyle->color = readRgb();
	else if (shapeType == 3)
		lineStyle->color = readRgba();
	else if (shapeType == 4)
	{
		uint16_t startCapStyle = m_bs->readUnsigned(2);
		uint16_t joinStyle = m_bs->readUnsigned(2);
		bool hasFillFlag = m_bs->readBit();
		bool noHScaleFlag = m_bs->readBit();
		bool noVScaleFlag = m_bs->readBit();
		bool pixelHintingFlag = m_bs->readBit();

		m_bs->skip(5);

		bool noClose = m_bs->readBit();
		uint16_t endCapStyle = m_bs->readUnsigned(2);

		m_bs->alignByte();

		if (joinStyle == 2)
		{
			uint16_t miterLimitFactor = m_bs->readUInt16();
		}

		if (!hasFillFlag)
			lineStyle->color = readRgba();
		else
		{
			SwfFillStyle* fill = readFillStyle(shapeType);
			lineStyle->color = fill->solid.color;
		}
	}

	return lineStyle;
}

SwfStyles* SwfReader::readStyles(int shapeType)
{
	SwfStyles* styles = alloc< SwfStyles >();

	styles->numFillStyles = m_bs->readUInt8();
	if ((shapeType == 3 || shapeType == 4) && styles->numFillStyles == 0xff)
		styles->numFillStyles = m_bs->readUInt16();

	styles->fillStyles = allocArray< SwfFillStyle >(styles->numFillStyles);
	for (uint16_t i = 0; i < styles->numFillStyles; ++i)
		styles->fillStyles[i] = readFillStyle(shapeType);

	styles->numLineStyles = m_bs->readUInt8();
	if (styles->numLineStyles == 0xff)
		styles->numLineStyles = m_bs->readUInt16();

	styles->lineStyles = allocArray< SwfLineStyle >(styles->numLineStyles);
	for (uint16_t i = 0; i < styles->numLineStyles; ++i)
		styles->lineStyles[i] = readLineStyle(shapeType);

	m_bs->alignByte();
	styles->numFillBits = m_bs->readUnsigned(4);
	styles->numLineBits = m_bs->readUnsigned(4);

	return styles;
}

bool SwfReader::readMorphGradientRecord(SwfGradientRecord*& outStartGradientRecord, SwfGradientRecord*& outEndGradientRecord)
{
	outStartGradientRecord = alloc< SwfGradientRecord >();
	outStartGradientRecord->ratio = m_bs->readUInt8();
	outStartGradientRecord->color = readRgba();

	outEndGradientRecord = alloc< SwfGradientRecord >();
	outEndGradientRecord->ratio = m_bs->readUInt8();
	outEndGradientRecord->color = readRgba();

	return true;
}

bool SwfReader::readMorphGradient(SwfGradient*& outStartGradient, SwfGradient*& outEndGradient)
{
	uint8_t numGradientRecords = m_bs->readUInt8();
	
	outStartGradient = alloc< SwfGradient >();
	outStartGradient->numGradientRecords = numGradientRecords;
	outStartGradient->gradientRecords = allocArray< SwfGradientRecord >(numGradientRecords);

	outEndGradient = alloc< SwfGradient >();
	outEndGradient->numGradientRecords = numGradientRecords;
	outEndGradient->gradientRecords = allocArray< SwfGradientRecord >(numGradientRecords);

	for (int i = 0; i < numGradientRecords; ++i)
	{
		if (!readMorphGradientRecord(outStartGradient->gradientRecords[i], outEndGradient->gradientRecords[i]))
			return false;
	}

	return true;
}

bool SwfReader::readMorphFillStyle(SwfFillStyle*& outStartFillStyle, SwfFillStyle*& outEndFillStyle, int shapeType)
{
	outStartFillStyle = alloc< SwfFillStyle >();
	outEndFillStyle = alloc< SwfFillStyle >();

	outStartFillStyle->type =
	outEndFillStyle->type = m_bs->readUInt8();

	if (outStartFillStyle->type == FstSolid)
	{
		outStartFillStyle->solid.color = readRgba();
		outEndFillStyle->solid.color = readRgba();
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
	outStartLineStyle = alloc< SwfLineStyle >();
	outEndLineStyle = alloc< SwfLineStyle >();

	outStartLineStyle->width = m_bs->readUInt16();
	outEndLineStyle->width = m_bs->readUInt16();

	bool hasFillFlag = false;

	if (shapeType == 2)
	{
		uint8_t startCapStyle = m_bs->readUnsigned(2);
		uint8_t joinStyle = m_bs->readUnsigned(2);
		hasFillFlag = m_bs->readBit();
		bool hasHScaleFlag = m_bs->readBit();
		bool hasVScaleFlag = m_bs->readBit();
		bool pixelHintingFlag = m_bs->readBit();
		m_bs->skip(5);
		bool noClose = m_bs->readBit();
		uint8_t endCapStyle = m_bs->readUnsigned(2);

		if (joinStyle == 2)
			uint16_t miterLimitFactor = m_bs->readUInt16();
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
		outStartLineStyle->color = startFillStyle->solid.color;
		outEndLineStyle->color = endFillStyle->solid.color;
	}

	return true;
}

bool SwfReader::readMorphStyles(SwfStyles*& outStartStyles, SwfStyles*& outEndStyles, int shapeType)
{
	uint16_t numFillStyles = m_bs->readUInt8();
	if (numFillStyles == 0xff)
		numFillStyles = m_bs->readUInt16();

	outStartStyles = alloc< SwfStyles >();
	outStartStyles->numFillStyles = numFillStyles;
	outStartStyles->fillStyles = allocArray< SwfFillStyle >(numFillStyles);

	outEndStyles = alloc< SwfStyles >();
	outEndStyles->numFillStyles = numFillStyles;
	outEndStyles->fillStyles = allocArray< SwfFillStyle >(numFillStyles);

	for (uint16_t i = 0; i < numFillStyles; ++i)
	{
		if (!readMorphFillStyle(outStartStyles->fillStyles[i], outEndStyles->fillStyles[i], shapeType))
			return false;
	}
	
	uint16_t numLineStyles = m_bs->readUInt8();
	if (numLineStyles == 0xff)
		numLineStyles = m_bs->readUInt16();

	outStartStyles->numLineStyles = numLineStyles;
	outStartStyles->lineStyles = allocArray< SwfLineStyle >(numLineStyles);
	
	outEndStyles->numLineStyles = numLineStyles;
	outEndStyles->lineStyles = allocArray< SwfLineStyle >(numLineStyles);

	for (uint16_t i = 0; i < numLineStyles; ++i)
	{
		if (!readMorphLineStyle(outStartStyles->lineStyles[i], outEndStyles->lineStyles[i], shapeType))
			return false;
	}

	return true;
}

SwfShapeRecord* SwfReader::readShapeRecord(uint32_t numFillBits, uint32_t numLineBits, int shapeType)
{
	SwfShapeRecord* shapeRecord = alloc< SwfShapeRecord >();

	memset(shapeRecord, 0, sizeof(SwfShapeRecord));

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
	const uint32_t c_maxRecordCount = 2 * 4096UL;

	outStyles = readStyles(shapeType);
	if (!outStyles)
		return false;

	outShape = alloc< SwfShape >();
	outShape->numShapeRecords = 0;
	outShape->shapeRecords = allocArray< SwfShapeRecord >(c_maxRecordCount);

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

		T_ASSERT (outShape->numShapeRecords < c_maxRecordCount);
		outShape->shapeRecords[outShape->numShapeRecords++] = shapeRecord;

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
	SwfShape* shape = alloc< SwfShape >();
	shape->numShapeRecords = 0;
	shape->shapeRecords = allocArray< SwfShapeRecord >(256);

	uint32_t numFillBits = m_bs->readUnsigned(4);
	uint32_t numLineBits = m_bs->readUnsigned(4);

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

		shape->shapeRecords[shape->numShapeRecords++] = shapeRecord;
	}

	return shape;
}

SwfTextRecord* SwfReader::readTextRecord(uint8_t numGlyphBits, uint8_t numAdvanceBits, int textType)
{
	SwfTextRecord* textRecord = alloc< SwfTextRecord >();

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
				textRecord->style.color = readRgba();
			else
				textRecord->style.color = readRgb();
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
		textRecord->glyph.glyphEntries = allocArray< SwfGlyphEntry >(textRecord->glyph.glyphCount);
		for (uint8_t i = 0; i < textRecord->glyph.glyphCount; ++i)
		{
			textRecord->glyph.glyphEntries[i] = alloc< SwfGlyphEntry >();
			textRecord->glyph.glyphEntries[i]->glyphIndex = m_bs->readUnsigned(numGlyphBits);
			textRecord->glyph.glyphEntries[i]->glyphAdvance = m_bs->readSigned(numAdvanceBits);
		}
	}

	m_bs->alignByte();
	return textRecord;
}

SwfFilter* SwfReader::readFilter()
{
	SwfFilter* filter = alloc< SwfFilter >();

	m_bs->alignByte();

	filter->filterId = m_bs->readUInt8();
	switch (filter->filterId)
	{
	case 0:
		{
			filter->dropShadow.dropShadowColor = readRgba();
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
			filter->glow.glowColor = readRgba();
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
			filter->bevel.shadowColor = readRgba();
			filter->bevel.highlightColor = readRgba();
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
			filter->convolution.matrix = alloc< float >(filter->convolution.matrixX * filter->convolution.matrixY);
			for (int i = 0; i < filter->convolution.matrixX * filter->convolution.matrixY; ++i)
				filter->convolution.matrix[i] = readFloat32();
			filter->convolution.defaultColor = readRgba();
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

SwfCxTransform SwfReader::readCxTransform(bool withAlpha)
{
	SwfCxTransform cxTransform;

	cxTransform.red[0]   = 1.0f; cxTransform.red[1]   = 0.0f;
	cxTransform.green[0] = 1.0f; cxTransform.green[1] = 0.0f;
	cxTransform.blue[0]  = 1.0f; cxTransform.blue[1]  = 0.0f;
	cxTransform.alpha[0] = 1.0f; cxTransform.alpha[1] = 0.0f;

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

		cxTransform.red[0] = redMul / 256.0f;
		cxTransform.green[0] = greenMul / 256.0f;
		cxTransform.blue[0] = blueMul / 256.0f;
		cxTransform.alpha[0] = alphaMul / 256.0f;
	}

	if (hasAddTerms)
	{
		int16_t redAdd = m_bs->readSigned(nbits);
		int16_t greenAdd = m_bs->readSigned(nbits);
		int16_t blueAdd = m_bs->readSigned(nbits);
		int16_t alphaAdd = withAlpha ? m_bs->readSigned(nbits) : 0;

		cxTransform.red[1] = redAdd / 256.0f;
		cxTransform.green[1] = greenAdd / 256.0f;
		cxTransform.blue[1] = blueAdd / 256.0f;
		cxTransform.alpha[1] = alphaAdd / 256.0f;
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

SwfRect SwfReader::readRect()
{
	SwfRect rect;

	m_bs->alignByte();

	uint32_t nbits = m_bs->readUnsigned(5);
	rect.min.x = float(m_bs->readSigned(nbits));
	rect.max.x = float(m_bs->readSigned(nbits));
	rect.min.y = float(m_bs->readSigned(nbits));
	rect.max.y = float(m_bs->readSigned(nbits));

	m_bs->alignByte();

	return rect;
}

SwfColor SwfReader::readRgb()
{
	SwfColor color;

	color.red = m_bs->readUInt8();
	color.green = m_bs->readUInt8();
	color.blue = m_bs->readUInt8();
	color.alpha = 255;

	return color;
}

SwfColor SwfReader::readRgba()
{
	SwfColor color;

	color.red = m_bs->readUInt8();
	color.green = m_bs->readUInt8();
	color.blue = m_bs->readUInt8();
	color.alpha = m_bs->readUInt8();

	return color;
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
	matrix.m[0] = scaleX / 65536.0f;		matrix.m[1] = rotateSkew0 / 65536.0f;	matrix.m[2] = 0.0f;
	matrix.m[3] = rotateSkew1 / 65536.0f;	matrix.m[4] = scaleY / 65536.0f;		matrix.m[5] = 0.0f;
	matrix.m[6] = float(translateX);		matrix.m[7] = float(translateY);		matrix.m[8] = 1.0f;

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

BitReader& SwfReader::getBitReader()
{
	return *m_bs;
}

void* SwfReader::alloc(uint32_t size)
{
	T_ASSERT_M (uint32_t(m_allocTail - m_allocHead) + size < c_allocSize, L"Out of memory");
	void* ptr = m_allocTail; m_allocTail += size;
	return ptr;
}

	}
}
