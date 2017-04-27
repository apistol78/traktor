/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionOpcodes_H
#define traktor_flash_ActionOpcodes_H

#include "Core/Config.h"

namespace traktor
{
	namespace flash
	{

/*! \brief AVM2 opcodes.
 * \ingroup Flash
 */
enum Avm2OpCodes
{
	Avm2OpAdd				= 0xa0,
	Avm2OpAdd_i				= 0xc5,
	Avm2OpAsType			= 0x86,
	Avm2OpAsTypeLate		= 0x87,
	Avm2OpBitAnd			= 0xa8,
	Avm2OpBitNot			= 0x97,
	Avm2OpBitOr				= 0xa9,
	Avm2OpBitXor			= 0xaa,
	Avm2OpCall				= 0x41,
	Avm2OpCallMethod		= 0x43,
	Avm2OpCallProperty		= 0x46,
	Avm2OpCallProplex		= 0x4c,
	Avm2OpCallPropVoid		= 0x4f,
	Avm2OpCallStatic		= 0x44,
	Avm2OpCallSuper			= 0x45,
	Avm2OpCallSuperVoid		= 0x4e,
	Avm2OpCheckFilter		= 0x78,
	Avm2OpCoerce			= 0x80,
	Avm2OpCoerce_a			= 0x82,
	Avm2OpCoerce_s			= 0x85,
	Avm2OpConstruct			= 0x42,
	Avm2OpConstructProp		= 0x4a,
	Avm2OpConstructSuper	= 0x49,
	Avm2OpConvert_b			= 0x76,
	Avm2OpConvert_i			= 0x73,
	Avm2OpConvert_d			= 0x75,
	Avm2OpConvert_o			= 0x77,
	Avm2OpConvert_u			= 0x74,
	Avm2OpConvert_s			= 0x70,
	Avm2OpDebug				= 0xef,
	Avm2OpDebugFile			= 0xf1,
	Avm2OpDebugLine			= 0xf0,
	Avm2OpDecLocal			= 0x94,
	Avm2OpDecLocal_i		= 0xc3,
	Avm2OpDecrement			= 0x93,
	Avm2OpDecrement_i		= 0xc1,
	Avm2OpDeleteProperty	= 0x6a,
	Avm2OpDivide			= 0xa3,
	Avm2OpDup				= 0x2a,
	Avm2OpDxns				= 0x06,
	Avm2OpDxnsLate			= 0x07,
	Avm2OpEquals			= 0xab,
	Avm2OpEscXAttr			= 0x72,
	Avm2OpEscXElem			= 0x71,
	Avm2OpFindProperty		= 0x5e,
	Avm2OpFindPropStrict	= 0x5d,
	Avm2OpGetDescendants	= 0x59,
	Avm2OpGetGlobalScope	= 0x64,
	Avm2OpGetGlobalSlot		= 0x6e,
	Avm2OpGetLex			= 0x60,
	Avm2OpGetLocal			= 0x62,
	Avm2OpGetLocal_0		= 0xd0,
	Avm2OpGetLocal_1		= 0xd1,
	Avm2OpGetLocal_2		= 0xd2,
	Avm2OpGetLocal_3		= 0xd3,
	Avm2OpGetProperty		= 0x66,
	Avm2OpGetScopeObject	= 0x65,
	Avm2OpGetSlot			= 0x6c,
	Avm2OpGetSuper			= 0x04,
	Avm2OpGreaterEquals		= 0xb0,	// \fixme Probably not correct; typo in Adobe doc.
	Avm2OpGreaterThan		= 0xaf,
	Avm2OpHasNext			= 0x1f,
	Avm2OpHasNext2			= 0x32,
	Avm2OpIfEq				= 0x13,
	Avm2OpIfFalse			= 0x12,
	Avm2OpIfGe				= 0x18,
	Avm2OpIfGt				= 0x17,
	Avm2OpIfLe				= 0x16,
	Avm2OpIfLt				= 0x15,
	Avm2OpIfNge				= 0x0f,
	Avm2OpIfNgt				= 0x0e,
	Avm2OpIfNle				= 0x0d,
	Avm2OpIfNlt				= 0x0c,
	Avm2OpIfNe				= 0x14,
	Avm2OpIfStrictEq		= 0x19,
	Avm2OpIfStrictNe		= 0x1a,
	Avm2OpIfTrue			= 0x11,
	Avm2OpIn				= 0xb4,
	Avm2OpIncLocal			= 0x92,
	Avm2OpIncLocal_i		= 0xc2,
	Avm2OpIncrement			= 0x91,
	Avm2OpIncrement_i		= 0xc0,
	Avm2OpInitProperty		= 0x68,
	Avm2OpInstanceOf		= 0xb1,
	Avm2OpIsType			= 0xb2,
	Avm2OpIsTypeLate		= 0xb3,
	Avm2OpJump				= 0x10,
	Avm2OpKill				= 0x08,
	Avm2OpLabel				= 0x09,
	Avm2OpLessEquals		= 0xae,
	Avm2OpLessThan			= 0xad,
	Avm2OpLookupSwitch		= 0x1b,
	Avm2OpLShift			= 0xa5,
	Avm2OpModulo			= 0xa4,
	Avm2OpMultiply			= 0xa2,
	Avm2OpMultiply_i		= 0xc7,
	Avm2OpNegate			= 0x90,
	Avm2OpNegate_i			= 0xc4,
	Avm2OpNewActivation		= 0x57,
	Avm2OpNewArray			= 0x56,
	Avm2OpNewCatch			= 0x5a,
	Avm2OpNewClass			= 0x58,
	Avm2OpNewFunction		= 0x40,
	Avm2OpNewObject			= 0x55,
	Avm2OpNextName			= 0x1e,
	Avm2OpNextValue			= 0x23,
	Avm2OpNop				= 0x02,
	Avm2OpNot				= 0x96,
	Avm2OpPop				= 0x29,
	Avm2OpPopScope			= 0x1d,
	Avm2OpPushByte			= 0x24,
	Avm2OpPushDouble		= 0x2f,
	Avm2OpPushFalse			= 0x27,
	Avm2OpPushInt			= 0x2d,
	Avm2OpPushNamespace		= 0x31,
	Avm2OpPushNan			= 0x28,
	Avm2OpPushNull			= 0x20,
	Avm2OpPushScope			= 0x30,
	Avm2OpPushShort			= 0x25,
	Avm2OpPushString		= 0x2c,
	Avm2OpPushTrue			= 0x26,
	Avm2OpPushUInt			= 0x2e,
	Avm2OpPushUndefined		= 0x21,
	Avm2OpPushWith			= 0x1c,
	Avm2OpReturnValue		= 0x48,
	Avm2OpReturnVoid		= 0x47,
	Avm2OpRShift			= 0xa6,
	Avm2OpSetLocal			= 0x63,
	Avm2OpSetLocal_0		= 0xd4,
	Avm2OpSetLocal_1		= 0xd5,
	Avm2OpSetLocal_2		= 0xd6,
	Avm2OpSetLocal_3		= 0xd7,
	Avm2OpSetGlobalSlot		= 0x6f,
	Avm2OpSetProperty		= 0x61,
	Avm2OpSetSlot			= 0x6d,
	Avm2OpSetSuper			= 0x05,
	Avm2OpStrictEqual		= 0xac,
	Avm2OpSubtract			= 0xa1,
	Avm2OpSubtract_i		= 0xc6,
	Avm2OpSwap				= 0x2b,
	Avm2OpThrow				= 0x03,
	Avm2OpTypeOf			= 0x95,
	Avm2OpURShift			= 0xa7
};

struct Avm2OpCodeInfo
{
	Avm2OpCodes op;
	const wchar_t* name;
	uint32_t width;
};

extern const Avm2OpCodeInfo c_avm2OpCodeInfo[];

const Avm2OpCodeInfo* findOpCodeInfo(uint8_t op);

	}
}

#endif	// traktor_flash_ActionOpcodes_H
