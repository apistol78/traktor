#ifndef traktor_BitWriter_H
#define traktor_BitWriter_H

#include "Core/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! \brief Bit stream writer.
 * \ingroup Core
 */
class T_DLLCLASS BitWriter : public Object
{
	T_RTTI_CLASS;

public:
	BitWriter(IStream* stream);

	/*! \brief Write single bit to stream. */
	void writeBit(bool bit);

	/*! \brief Read variable bit-length unsigned number from stream. */
	void writeUnsigned(int32_t nbits, uint32_t value);

	/*! \brief Flush bits. */
	void flush();

	/*! \brief Return current bit position. */
	uint32_t tell() const;

	/*! \brief Get reference to underlying stream. */
	Ref< IStream > getStream();

private:
	Ref< IStream > m_stream;
	uint8_t m_data;
	int8_t m_cnt;
};

}

#endif	// traktor_BitWriter_H
