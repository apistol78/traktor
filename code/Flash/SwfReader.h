#ifndef traktor_flash_SwfReader_H
#define traktor_flash_SwfReader_H

#include "Core/Object.h"
#include "Core/Io/BitReader.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;
class PoolAllocator;

	namespace flash
	{

/*! \brief SWF type reader.
 * \ingroup Flash
 *
 * This class holds a temporary pool of allocated
 * types; ie. all returned instances will be invalid
 * as soon as the SwfReader instance is destroyed.
 */
class T_DLLCLASS SwfReader : public Object
{
	T_RTTI_CLASS;

public:
	SwfReader(IStream* stream);

	/*! \brief Enter allocation scope. */
	void enterScope();

	/*! \brief Leave allocation scope. */
	void leaveScope();

	/*! \brief Read header.
	 *
	 * \return SWF header, null if failed.
	 */
	SwfHeader* readHeader();

	/*! \brief Read tag.
	 *
	 * \return SWF tag, null if failed.
	 */
	SwfTag* readTag();

	/*! \brief Read gradient record.
	 *
	 * \param shapeType Type of shape record.
	 * \return Gradient record, null if failed.
	 */
	SwfGradientRecord* readGradientRecord(int shapeType);

	/*! \brief Read gradient.
	 *
	 * \param shapeType Type of shape record.
	 * \return Gradient, null if failed.
	 */
	SwfGradient* readGradient(int shapeType);

	/*! \brief Read focal gradient.
	 *
	 * \param shapeType Type of shape record.
	 * \return Gradient, null if failed.
	 */
	SwfGradient* readFocalGradient(int shapeType);

	/*! \brief Read fill style.
	 *
	 * \param shapeType Type of shape record.
	 * \return Fill style, null if failed.
	 */
	SwfFillStyle* readFillStyle(int shapeType);

	/*! \brief Read line style.
	 *
	 * \param shapeType Type of shape record.
	 * \return Line style, null if failed.
	 */
	SwfLineStyle* readLineStyle(int shapeType);

	/*! \brief Read styles.
	 *
	 * \param shapeType Type of shape record.
	 * \return Styles, null if failed.
	 */
	SwfStyles* readStyles(int shapeType);

	/*! \brief Read morph gradient record.
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

	/*! \brief Read sound.
	 */
	SwfSoundInfo* readSoundInfo();

	/*! \name Simple types */
	//@{

	SwfCxTransform readCxTransform(bool withAlpha);

	SwfKerningRecord readKerningRecord(bool wideCodes);

	SwfRect readRect();

	SwfColor readRgb();

	SwfColor readRgba();

	SwfMatrix readMatrix();

	std::string readStringU8();

	std::string readString();

	float readFloat32();

	float readFixed();

	float readFixed8();

	//@}

	BitReader& getBitReader();

private:
	Ref< IStream > m_stream;
	Ref< BitReader > m_bs;
	Ref< PoolAllocator > m_pool;
};

	}
}

#endif	// traktor_flash_SwfReader_H
