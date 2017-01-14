#include <cstring>
#include <sstream>
#include <iomanip>
#include "Core/Misc/SHA1.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace
	{

const uint32_t SHA1_K0 = 0x5a827999;
const uint32_t SHA1_K20 = 0x6ed9eba1;
const uint32_t SHA1_K40 = 0x8f1bbcdc;
const uint32_t SHA1_K60 = 0xca62c1d6;
const uint8_t HASH_LENGTH = 20;
const uint8_t BLOCK_LENGTH = 64;

struct sha1info
{
	uint32_t buffer[BLOCK_LENGTH / 4];
	uint32_t state[HASH_LENGTH / 4];
	uint32_t byteCount;
	uint8_t bufferOffset;
	uint8_t keyBuffer[BLOCK_LENGTH];
	uint8_t innerHash[HASH_LENGTH];
};

uint32_t sha1_rol32(uint32_t number, uint8_t bits)
{
	return ((number << bits) | (number >> (32 - bits)));
}

void sha1_init(sha1info* s)
{
	s->state[0] = 0x67452301;
	s->state[1] = 0xefcdab89;
	s->state[2] = 0x98badcfe;
	s->state[3] = 0x10325476;
	s->state[4] = 0xc3d2e1f0;
	s->byteCount = 0;
	s->bufferOffset = 0;
}

void sha1_hashBlock(sha1info* s)
{
	uint32_t a = s->state[0];
	uint32_t b = s->state[1];
	uint32_t c = s->state[2];
	uint32_t d = s->state[3];
	uint32_t e = s->state[4];
	uint32_t t;

	for (uint8_t i = 0; i < 80; ++i)
	{
		if (i >= 16)
		{
			t = s->buffer[(i + 13) & 15] ^ s->buffer[(i + 8) & 15] ^ s->buffer[(i + 2) & 15] ^ s->buffer[i & 15];
			s->buffer[i & 15] = sha1_rol32(t, 1);
		}

		if (i < 20)
			t = (d ^ (b & (c ^ d))) + SHA1_K0;
		else if (i < 40)
			t = (b ^ c ^ d) + SHA1_K20;
		else if (i < 60)
			t = ((b & c) | (d & (b | c))) + SHA1_K40;
		else
			t = (b ^ c ^ d) + SHA1_K60;
		
		t += sha1_rol32(a, 5) + e + s->buffer[i & 15];

		e = d;
		d = c;
		c = sha1_rol32(b, 30);
		b = a;
		a = t;
	}

	s->state[0] += a;
	s->state[1] += b;
	s->state[2] += c;
	s->state[3] += d;
	s->state[4] += e;
}

void sha1_addUncounted(sha1info* s, uint8_t data)
{
	uint8_t* const b = (uint8_t*)s->buffer;
#ifdef T_BIG_ENDIAN
	b[s->bufferOffset] = data;
#else
	b[s->bufferOffset ^ 3] = data;
#endif
	s->bufferOffset++;
	if (s->bufferOffset == BLOCK_LENGTH)
	{
		sha1_hashBlock(s);
		s->bufferOffset = 0;
	}
}

void sha1_writebyte(sha1info* s, uint8_t data)
{
	++s->byteCount;
	sha1_addUncounted(s, data);
}

void sha1_pad(sha1info* s)
{
	// Implement SHA-1 padding (fips180-2 §5.1.1).

	// Pad with 0x80 followed by 0x00 until the end of the block.
	sha1_addUncounted(s, 0x80);
	while (s->bufferOffset != 56)
		sha1_addUncounted(s, 0x00);

	// Append length in the last 8 bytes
	sha1_addUncounted(s, 0); // We're only using 32 bit lengths
	sha1_addUncounted(s, 0); // But SHA-1 supports 64 bit lengths
	sha1_addUncounted(s, 0); // So zero pad the top bits
	sha1_addUncounted(s, s->byteCount >> 29); // Shifting to multiply by 8
	sha1_addUncounted(s, s->byteCount >> 21); // as SHA-1 supports bitstreams as well as
	sha1_addUncounted(s, s->byteCount >> 13); // byte.
	sha1_addUncounted(s, s->byteCount >> 5);
	sha1_addUncounted(s, s->byteCount << 3);
}

uint8_t* sha1_result(sha1info* s)
{
	// Pad to complete the last block.
	sha1_pad(s);

#ifndef T_BIG_ENDIAN
	// Swap byte order back.
	for (int32_t i = 0; i < 5; ++i)
	{
		s->state[i]= (((s->state[i])<<24)& 0xff000000) |
			(((s->state[i])<<8) & 0x00ff0000) |
			(((s->state[i])>>8) & 0x0000ff00) |
			(((s->state[i])>>24)& 0x000000ff);
	}
#endif

	// Return pointer to hash (20 characters).
	return (uint8_t*)s->state;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.SHA1", SHA1, Object)

SHA1::SHA1()
:	m_sha1nfo(new sha1info())
{
	begin();
}

SHA1::~SHA1()
{
	delete (sha1info*)m_sha1nfo;
}

void SHA1::begin()
{
	sha1_init((sha1info*)m_sha1nfo);
}

bool SHA1::createFromString(const std::wstring& str)
{
	std::string s = wstombs(str);
	feed(s.c_str(), uint32_t(s.length()));
	end();
	return true;
}

void SHA1::feed(const void* buffer, uint32_t bufferSize)
{
	const uint8_t* u8buffer = static_cast< const uint8_t* const >(buffer);
	for (uint32_t i = 0; i < bufferSize; ++i)
		sha1_writebyte((sha1info*)m_sha1nfo, u8buffer[i]);
}

void SHA1::end()
{

}

std::wstring SHA1::format() const
{
	const uint8_t* r = sha1_result((sha1info*)m_sha1nfo);
	std::wstringstream ss;
	for (int i = 0; i < 20; ++i)
		ss << std::hex << std::setfill(L'0') << std::setw(2)<< r[i];
	return ss.str();    
}

}
