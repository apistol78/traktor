/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/BitReader.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Misc/Endian.h"
#include "UnitTest/CaseBitReader.h"

namespace traktor
{

#pragma pack(1)
struct Brd
{
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	int8_t s8;
	int16_t s16;
	int32_t s32;
};
#pragma pack()

void CaseBitReader::run()
{

	//{
	//	Brd b;
	//	b.u8 = 0x24;
	//	b.u16 = 0x1245;
	//	b.u32 = 0xcafebabe;
	//	b.s8 = -12;
	//	b.s16 = -1234;
	//	b.s32 = -12345678;

	//	MemoryStream ms(&b, sizeof(b), true, false);
	//	BitReader br(&ms);

	//	CASE_ASSERT_EQUAL(br.readUnsigned(8), b.u8);
	//	CASE_ASSERT_EQUAL(br.readUnsigned(16), b.u16);
	//	CASE_ASSERT_EQUAL(br.readUnsigned(32), b.u32);
	//	CASE_ASSERT_EQUAL(br.readSigned(8), b.s8);
	//	CASE_ASSERT_EQUAL(br.readSigned(16), b.s16);
	//	CASE_ASSERT_EQUAL(br.readSigned(32), b.s32);
	//}
	{
		uint8_t d[] = { 0xa5 };

		MemoryStream ms(d, sizeof(d), true, false);
		BitReader br(&ms);

		CASE_ASSERT_EQUAL(br.readUnsigned(4), 0xa);
		CASE_ASSERT_EQUAL(br.readUnsigned(4), 0x5);
	}
	{
		uint8_t d[] = { 0xb6 };

		MemoryStream ms(d, sizeof(d), true, false);
		BitReader br(&ms);

		CASE_ASSERT_EQUAL(br.readUnsigned(3), 5);
		CASE_ASSERT_EQUAL(br.readUnsigned(5), 22);
	}
	{
		uint8_t d[] = { 0xb6, 0x00 };

		MemoryStream ms(d, sizeof(d), true, false);
		BitReader br(&ms);

		CASE_ASSERT_EQUAL(br.readUnsigned(3), 5);
		CASE_ASSERT_EQUAL(br.readUnsigned(6), 44);
	}
	{
		uint8_t d[] = { 0xa6, 0xc0 };

		MemoryStream ms(d, sizeof(d), true, false);
		BitReader br(&ms);

		CASE_ASSERT_EQUAL(br.readUnsigned(10), 667);
	}
}

}
