#include "Core/Io/BitWriter.h"
#include "Core/Io/Stream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.BitWriter", BitWriter, Object)

BitWriter::BitWriter(Stream* stream)
:	m_stream(stream)
,	m_data(0)
,	m_cnt(0)
{
}

void BitWriter::writeBit(bool bit)
{
	m_data |= bit ? (1 << m_cnt) : 0;
	if (++m_cnt >= 8)
		flush();
}

void BitWriter::writeUnsigned(int nbits, uint32_t value)
{
	for (int i = 0; i < nbits; ++i)
	{
		uint32_t bit = 1 << (nbits - i - 1);
		writeBit((value & bit) == bit);
	}
}

void BitWriter::flush()
{
	if (m_cnt <= 0)
		return;

	m_stream->write(&m_data, 1);
	m_cnt = 0;
}

uint32_t BitWriter::tell() const
{
	return (m_stream->tell() << 3) + m_cnt;
}

Ref< Stream > BitWriter::getStream()
{
	return m_stream;
}

}
