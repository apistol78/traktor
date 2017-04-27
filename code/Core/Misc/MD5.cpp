/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <sstream>
#include "Core/Misc/MD5.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStaticArray.h"

#define MD5_INIT_STATE_0 0x67452301
#define MD5_INIT_STATE_1 0xefcdab89
#define MD5_INIT_STATE_2 0x98badcfe
#define MD5_INIT_STATE_3 0x10325476

#define MD5_S11  7
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
#define MD5_S21  5
#define MD5_S22  9
#define MD5_S23 14
#define MD5_S24 20
#define MD5_S31  4
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
#define MD5_S41  6
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21

#define MD5_T01  0xd76aa478
#define MD5_T02  0xe8c7b756
#define MD5_T03  0x242070db
#define MD5_T04  0xc1bdceee
#define MD5_T05  0xf57c0faf
#define MD5_T06  0x4787c62a
#define MD5_T07  0xa8304613
#define MD5_T08  0xfd469501
#define MD5_T09  0x698098d8
#define MD5_T10  0x8b44f7af
#define MD5_T11  0xffff5bb1
#define MD5_T12  0x895cd7be
#define MD5_T13  0x6b901122
#define MD5_T14  0xfd987193
#define MD5_T15  0xa679438e
#define MD5_T16  0x49b40821

#define MD5_T17  0xf61e2562
#define MD5_T18  0xc040b340
#define MD5_T19  0x265e5a51
#define MD5_T20  0xe9b6c7aa
#define MD5_T21  0xd62f105d
#define MD5_T22  0x02441453
#define MD5_T23  0xd8a1e681
#define MD5_T24  0xe7d3fbc8
#define MD5_T25  0x21e1cde6
#define MD5_T26  0xc33707d6
#define MD5_T27  0xf4d50d87
#define MD5_T28  0x455a14ed
#define MD5_T29  0xa9e3e905
#define MD5_T30  0xfcefa3f8
#define MD5_T31  0x676f02d9
#define MD5_T32  0x8d2a4c8a

#define MD5_T33  0xfffa3942
#define MD5_T34  0x8771f681
#define MD5_T35  0x6d9d6122
#define MD5_T36  0xfde5380c
#define MD5_T37  0xa4beea44
#define MD5_T38  0x4bdecfa9
#define MD5_T39  0xf6bb4b60
#define MD5_T40  0xbebfbc70
#define MD5_T41  0x289b7ec6
#define MD5_T42  0xeaa127fa
#define MD5_T43  0xd4ef3085
#define MD5_T44  0x04881d05
#define MD5_T45  0xd9d4d039
#define MD5_T46  0xe6db99e5
#define MD5_T47  0x1fa27cf8
#define MD5_T48  0xc4ac5665

#define MD5_T49  0xf4292244
#define MD5_T50  0x432aff97
#define MD5_T51  0xab9423a7
#define MD5_T52  0xfc93a039
#define MD5_T53  0x655b59c3
#define MD5_T54  0x8f0ccc92
#define MD5_T55  0xffeff47d
#define MD5_T56  0x85845dd1
#define MD5_T57  0x6fa87e4f
#define MD5_T58  0xfe2ce6e0
#define MD5_T59  0xa3014314
#define MD5_T60  0x4e0811a1
#define MD5_T61  0xf7537e82
#define MD5_T62  0xbd3af235
#define MD5_T63  0x2ad7d2bb
#define MD5_T64  0xeb86d391

namespace traktor
{
	namespace
	{

void byteToDword(uint32_t* output, const uint8_t* input, uint32_t length)
{
	for (uint32_t i = 0, j = 0; j < length; ++i, j += 4)
	{
		output[i] = 
			static_cast< uint32_t >(input[j]) |
			static_cast< uint32_t >(input[j + 1] << 8) |
			static_cast< uint32_t >(input[j + 2] << 16) |
			static_cast< uint32_t >(input[j + 3] << 24);
	}        
}

void dwordToByte(uint8_t* output, const uint32_t* input, uint32_t length)
{
	for (uint32_t i = 0, j = 0; j < length; ++i, j += 4)
	{
		output[j] = static_cast< uint8_t >(input[i] & 0xff);
		output[j + 1] = static_cast< uint8_t >((input[i] >> 8) & 0xff);
		output[j + 2] = static_cast< uint8_t >((input[i] >> 16) & 0xff);
		output[j + 3] = static_cast< uint8_t >((input[i] >> 24) & 0xff);
	}
}

uint32_t rotate(uint32_t x, int32_t n)
{
	return (x << n) | (x >> (32 - n));
}

uint8_t fromHex(wchar_t hc)
{
	if (hc >= L'0' && hc <= L'9')
		return hc - L'0';
#if !defined(_WIN32)
	hc = std::tolower(hc);
#else
	hc = tolower(hc);
#endif
	if (hc >= L'a' && hc <= L'f')
		return (hc - L'a') + 10;
	return 0;
}

void FF(
	uint32_t& a,
	uint32_t b,
	uint32_t c,
	uint32_t d,
	uint32_t x,
	uint32_t s,
	uint32_t t
)
{
	uint32_t f = (b & c) | (~b & d);
	a += f + x + t;
	a = rotate(a, s);
	a += b;
}

void GG(
	uint32_t& a,
	uint32_t b,
	uint32_t c,
	uint32_t d,
	uint32_t x,
	uint32_t s,
	uint32_t t
)
{
	uint32_t g = (b & d) | (c & ~d);
	a += g + x + t;
	a = rotate(a, s);
	a += b;
}

void HH(
	uint32_t& a,
	uint32_t b,
	uint32_t c,
	uint32_t d,
	uint32_t x,
	uint32_t s,
	uint32_t t
)
{
	uint32_t h = (b ^ c ^ d);
	a += h + x + t;
	a = rotate(a, s);
	a += b;
}

void II(
	uint32_t& a,
	uint32_t b,
	uint32_t c,
	uint32_t d,
	uint32_t x,
	uint32_t s,
	uint32_t t
)
{
	uint32_t i = (c ^ (b | ~d));
	a += i + x + t;
	a = rotate(a, s);
	a += b;
}

uint8_t PADDING[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.MD5", 0, MD5, ISerializable)

MD5::MD5()
{
	begin();
}

MD5::MD5(const void* md5)
{
	begin();

	m_md5[0] = *((const uint32_t*)md5 + 0);
	m_md5[1] = *((const uint32_t*)md5 + 1);
	m_md5[2] = *((const uint32_t*)md5 + 2);
	m_md5[3] = *((const uint32_t*)md5 + 3);
}

bool MD5::create(const std::wstring& md5)
{
	if (md5.length() != 32)
		return false;

	const int MD5_SIZE = 16;
	uint8_t md5b[MD5_SIZE];

	for (int i = 0; i < MD5_SIZE; ++i)
	{
		wchar_t hn = md5[i * 2];
		wchar_t ln = md5[i * 2 + 1];
		md5b[i] = (fromHex(hn) << 4) | fromHex(ln);
	}

	byteToDword(m_md5, md5b, MD5_SIZE);
	return true;
}

bool MD5::createFromString(const std::wstring& str)
{
	std::string s = wstombs(str);
	feed(s.c_str(), uint32_t(s.length()));
	end();
	return true;
}

void MD5::begin()
{
	std::memset(m_buffer, 0, sizeof(m_buffer));

	m_count[0] =
	m_count[1] = 0;

	m_md5[0] = MD5_INIT_STATE_0;
	m_md5[1] = MD5_INIT_STATE_1;
	m_md5[2] = MD5_INIT_STATE_2;
	m_md5[3] = MD5_INIT_STATE_3;
}

void MD5::feed(const void* buffer, uint32_t bufferSize)
{
	uint32_t index = (m_count[0] >> 3) & 0x3f;

	if ((m_count[0] += bufferSize << 3) < (bufferSize << 3))
		m_count[1]++;
	m_count[1] += (bufferSize >> 29);

	uint32_t i = 0;
	uint32_t partSize = 64 - index;

	if (bufferSize >= partSize)
	{
		memcpy(&m_buffer[index], buffer, partSize);
		transform(m_buffer);
		for (i = partSize; i + 63 < bufferSize; i += 64)
			transform(&reinterpret_cast< const uint8_t* >(buffer)[i]);
		index = 0;
	}

	memcpy(&m_buffer[index], &reinterpret_cast< const uint8_t* >(buffer)[i], bufferSize - i);
}

void MD5::end()
{
	uint8_t bits[8];
	dwordToByte(bits, m_count, 8);

	uint32_t index = (m_count[0] >> 3) & 0x3f;
	uint32_t padSize = (index < 56) ? (56 - index) : (120 - index);
	feed(PADDING, padSize);

	feed(bits, 8);
}

const uint32_t* MD5::get() const
{
	return m_md5;
}

std::wstring MD5::format() const
{
	const wchar_t hex[] = { L"0123456789ABCDEF" };
	const int MD5_SIZE = 16;

	uint8_t md5[MD5_SIZE];
	dwordToByte(md5, m_md5, MD5_SIZE);

	std::wstringstream ss;
	for (int i = 0; i < MD5_SIZE; ++i)
		ss << hex[(md5[i] >> 4) & 15] << hex[md5[i] & 15];

	T_ASSERT (ss.str().length() == 32);
	return ss.str();    
}

void MD5::serialize(ISerializer& s)
{
	if (s.getDirection() == ISerializer::SdRead)
	{
		std::wstring md5;
		s >> Member< std::wstring >(L"md5", md5);
		create(md5);
	}
	else	// SdWrite
	{
		std::wstring md5 = format();
		s >> Member< std::wstring >(L"md5", md5);
	}
}

bool MD5::operator == (const MD5& md5) const
{
	return std::memcmp(m_md5, md5.m_md5, sizeof(m_md5)) == 0;
}

bool MD5::operator != (const MD5& md5) const
{
	return std::memcmp(m_md5, md5.m_md5, sizeof(m_md5)) != 0;
}

bool MD5::operator < (const MD5& md5) const
{
	return std::memcmp(m_md5, md5.m_md5, sizeof(m_md5)) < 0;
}

void MD5::transform(const uint8_t block[64])
{
	uint32_t a = m_md5[0];
	uint32_t b = m_md5[1];
	uint32_t c = m_md5[2];
	uint32_t d = m_md5[3];

	uint32_t x[16];
	byteToDword(x, block, 64);

	FF (a, b, c, d, x[ 0], MD5_S11, MD5_T01); 
	FF (d, a, b, c, x[ 1], MD5_S12, MD5_T02); 
	FF (c, d, a, b, x[ 2], MD5_S13, MD5_T03); 
	FF (b, c, d, a, x[ 3], MD5_S14, MD5_T04); 
	FF (a, b, c, d, x[ 4], MD5_S11, MD5_T05); 
	FF (d, a, b, c, x[ 5], MD5_S12, MD5_T06); 
	FF (c, d, a, b, x[ 6], MD5_S13, MD5_T07); 
	FF (b, c, d, a, x[ 7], MD5_S14, MD5_T08); 
	FF (a, b, c, d, x[ 8], MD5_S11, MD5_T09); 
	FF (d, a, b, c, x[ 9], MD5_S12, MD5_T10); 
	FF (c, d, a, b, x[10], MD5_S13, MD5_T11); 
	FF (b, c, d, a, x[11], MD5_S14, MD5_T12); 
	FF (a, b, c, d, x[12], MD5_S11, MD5_T13); 
	FF (d, a, b, c, x[13], MD5_S12, MD5_T14); 
	FF (c, d, a, b, x[14], MD5_S13, MD5_T15); 
	FF (b, c, d, a, x[15], MD5_S14, MD5_T16); 

	GG (a, b, c, d, x[ 1], MD5_S21, MD5_T17); 
	GG (d, a, b, c, x[ 6], MD5_S22, MD5_T18); 
	GG (c, d, a, b, x[11], MD5_S23, MD5_T19); 
	GG (b, c, d, a, x[ 0], MD5_S24, MD5_T20); 
	GG (a, b, c, d, x[ 5], MD5_S21, MD5_T21); 
	GG (d, a, b, c, x[10], MD5_S22, MD5_T22); 
	GG (c, d, a, b, x[15], MD5_S23, MD5_T23); 
	GG (b, c, d, a, x[ 4], MD5_S24, MD5_T24); 
	GG (a, b, c, d, x[ 9], MD5_S21, MD5_T25); 
	GG (d, a, b, c, x[14], MD5_S22, MD5_T26); 
	GG (c, d, a, b, x[ 3], MD5_S23, MD5_T27); 
	GG (b, c, d, a, x[ 8], MD5_S24, MD5_T28); 
	GG (a, b, c, d, x[13], MD5_S21, MD5_T29); 
	GG (d, a, b, c, x[ 2], MD5_S22, MD5_T30); 
	GG (c, d, a, b, x[ 7], MD5_S23, MD5_T31); 
	GG (b, c, d, a, x[12], MD5_S24, MD5_T32); 

	HH (a, b, c, d, x[ 5], MD5_S31, MD5_T33); 
	HH (d, a, b, c, x[ 8], MD5_S32, MD5_T34); 
	HH (c, d, a, b, x[11], MD5_S33, MD5_T35); 
	HH (b, c, d, a, x[14], MD5_S34, MD5_T36); 
	HH (a, b, c, d, x[ 1], MD5_S31, MD5_T37); 
	HH (d, a, b, c, x[ 4], MD5_S32, MD5_T38); 
	HH (c, d, a, b, x[ 7], MD5_S33, MD5_T39); 
	HH (b, c, d, a, x[10], MD5_S34, MD5_T40); 
	HH (a, b, c, d, x[13], MD5_S31, MD5_T41); 
	HH (d, a, b, c, x[ 0], MD5_S32, MD5_T42); 
	HH (c, d, a, b, x[ 3], MD5_S33, MD5_T43); 
	HH (b, c, d, a, x[ 6], MD5_S34, MD5_T44); 
	HH (a, b, c, d, x[ 9], MD5_S31, MD5_T45); 
	HH (d, a, b, c, x[12], MD5_S32, MD5_T46); 
	HH (c, d, a, b, x[15], MD5_S33, MD5_T47); 
	HH (b, c, d, a, x[ 2], MD5_S34, MD5_T48); 

	II (a, b, c, d, x[ 0], MD5_S41, MD5_T49); 
	II (d, a, b, c, x[ 7], MD5_S42, MD5_T50); 
	II (c, d, a, b, x[14], MD5_S43, MD5_T51); 
	II (b, c, d, a, x[ 5], MD5_S44, MD5_T52); 
	II (a, b, c, d, x[12], MD5_S41, MD5_T53); 
	II (d, a, b, c, x[ 3], MD5_S42, MD5_T54); 
	II (c, d, a, b, x[10], MD5_S43, MD5_T55); 
	II (b, c, d, a, x[ 1], MD5_S44, MD5_T56); 
	II (a, b, c, d, x[ 8], MD5_S41, MD5_T57); 
	II (d, a, b, c, x[15], MD5_S42, MD5_T58); 
	II (c, d, a, b, x[ 6], MD5_S43, MD5_T59); 
	II (b, c, d, a, x[13], MD5_S44, MD5_T60); 
	II (a, b, c, d, x[ 4], MD5_S41, MD5_T61); 
	II (d, a, b, c, x[11], MD5_S42, MD5_T62); 
	II (c, d, a, b, x[ 2], MD5_S43, MD5_T63); 
	II (b, c, d, a, x[ 9], MD5_S44, MD5_T64); 

	m_md5[0] += a;
	m_md5[1] += b;
	m_md5[2] += c;
	m_md5[3] += d;
}

}
