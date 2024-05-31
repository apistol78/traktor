/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Io/BitReader.h"
#include "Spark/ColorTransform.h"
#include "Spark/Swf/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;
class PoolAllocator;

}

namespace traktor::spark
{

/*! SWF type reader.
 * \ingroup Spark
 *
 * This class holds a temporary pool of allocated
 * types; ie. all returned instances will be invalid
 * as soon as the SwfReader instance is destroyed.
 */
class T_DLLCLASS SwfReader : public Object
{
	T_RTTI_CLASS;

public:
	explicit SwfReader(IStream* stream);

	/*! Enter allocation scope. */
	void enterScope();

	/*! Leave allocation scope. */
	void leaveScope();

	/*! Read header.
	 *
	 * \return SWF header, null if failed.
	 */
	SwfHeader* readHeader();

	/*! Read tag.
	 *
	 * \return SWF tag, null if failed.
	 */
	SwfTag* readTag();

	/*! Read gradient record.
	 *
	 * \param shapeType Type of shape record.
	 * \return Gradient record, null if failed.
	 */
	SwfGradientRecord* readGradientRecord(int shapeType);

	/*! Read gradient.
	 *
	 * \param shapeType Type of shape record.
	 * \return Gradient, null if failed.
	 */
	SwfGradient* readGradient(int shapeType);

	/*! Read focal gradient.
	 *
	 * \param shapeType Type of shape record.
	 * \return Gradient, null if failed.
	 */
	SwfGradient* readFocalGradient(int shapeType);

	/*! Read fill style.
	 *
	 * \param shapeType Type of shape record.
	 * \return Fill style, null if failed.
	 */
	SwfFillStyle* readFillStyle(int shapeType);

	/*! Read line style.
	 *
	 * \param shapeType Type of shape record.
	 * \return Line style, null if failed.
	 */
	SwfLineStyle* readLineStyle(int shapeType);

	/*! Read styles.
	 *
	 * \param shapeType Type of shape record.
	 * \return Styles, null if failed.
	 */
	SwfStyles* readStyles(int shapeType);

	/*! Read morph gradient record.
	 *
	 * \param outStartGradientRecord Start morph gradient record.
	 * \param outEndGradientRecord End morph gradient record.
	 * \return True if successful.
	 */
	bool readMorphGradientRecord(SwfGradientRecord*& outStartGradientRecord, SwfGradientRecord*& outEndGradientRecord);

	bool readMorphGradient(SwfGradient*& outStartGradient, SwfGradient*& outEndGradient);

	bool readMorphFillStyle(SwfFillStyle*& outStartFillStyle, SwfFillStyle*& outEndFillStyle, int shapeType);

	bool readMorphLineStyle(SwfLineStyle*& outStartLineStyle, SwfLineStyle*& outEndLineStyle, int shapeType);

	bool readMorphStyles(SwfStyles*& outStartStyles, SwfStyles*& outEndStyles, int shapeType);

	SwfShapeRecord* readShapeRecord(uint32_t numFillBits, uint32_t numLineBits, int shapeType);

	bool readShapeWithStyle(SwfShape*& outShape, SwfStyles*& outStyles, int shapeType);

	SwfShape* readShape(int shapeType);

	SwfTextRecord* readTextRecord(uint8_t numGlyphBits, uint8_t numAdvanceBits, int textType);

	SwfFilter* readFilter();

	bool readFilterList(AlignedVector< SwfFilter* >& outFilterList);

	/*! Read sound.
	 */
	SwfSoundInfo* readSoundInfo();

	/*! \name Simple types */
	//@{

	ColorTransform readCxTransform(bool withAlpha);

	SwfKerningRecord readKerningRecord(bool wideCodes);

	Aabb2 readRect();

	Color4f readRgb();

	Color4f readRgba();

	SwfMatrix readMatrix();

	std::string readStringU8();

	std::string readString();

	float readFloat32();

	float readFixed();

	float readFixed8();

	uint16_t readU16BE();

	uint32_t readEncodedU30();

	uint32_t readEncodedU32();

	int32_t readEncodedS32();

	//@}

	BitReader& getBitReader();

private:
	Ref< IStream > m_stream;
	Ref< BitReader > m_bs;
	Ref< PoolAllocator > m_pool;
};

}
