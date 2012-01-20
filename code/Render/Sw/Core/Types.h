#ifndef traktor_render_sw_Types_H
#define traktor_render_sw_Types_H

#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup SW */
//@{

enum Opcode
{
	OpNop,
	OpFetchConstant,
	OpFetchTargetSize,
	OpFetchUniform,
	OpFetchIndexedUniform,
	OpFetchVarying,
	OpStoreVarying,
	OpMove,
	OpAbs,
	OpIncrement,
	OpDecrement,
	OpAdd,					// 11
	OpDiv,
	OpMul,
	OpMulAdd,
	OpLog,
	OpLog2,
	OpLog10,
	OpExp,
	OpFraction,
	OpNeg,
	OpPow,					// 21
	OpSqrt,
	OpSub,
	OpAcos,
	OpAtan,
	OpCos,
	OpSin,
	OpTan,
	OpCross,
	OpDot3,
	OpDot4,					// 31
	OpLength,
	OpNormalize,
	OpTransform,
	OpTranspose,
	OpClamp,
	OpLerp,
	OpMixIn,
	OpMax,
	OpSampler,
	OpSwizzle,				// 41
	OpSet,
	OpExpandWithZero,
	OpCompareGreaterEqual,
	OpCompareGreater,
	OpCompareEqual,
	OpCompareNotEqual,
	OpJumpIfZero,
	OpJump,
	OpTrace					// 51
};

static const wchar_t* c_opcodeText[] =
{
	L"OpNop",
	L"OpFetchConstant",
	L"OpFetchTargetSize",
	L"OpFetchUniform",
	L"OpFetchIndexedUniform",
	L"OpFetchVarying",
	L"OpStoreVarying",
	L"OpMove",
	L"OpAbs",
	L"OpIncrement",
	L"OpDecrement",
	L"OpAdd",
	L"OpDiv",
	L"OpMul",
	L"OpMulAdd",
	L"OpLog",
	L"OpLog2",
	L"OpLog10",
	L"OpExp",
	L"OpFraction",
	L"OpNeg",
	L"OpPow",
	L"OpSqrt",
	L"OpSub",
	L"OpAcos",
	L"OpAtan",
	L"OpCos",
	L"OpSin",
	L"OpTan",
	L"OpCross",
	L"OpDot3",
	L"OpDot4",
	L"OpLength",
	L"OpNormalize",
	L"OpTransform",
	L"OpTranspose",
	L"OpClamp",
	L"OpLerp",
	L"OpMixIn",
	L"OpMax",
	L"OpSampler",
	L"OpSwizzle",
	L"OpSet",
	L"OpExpandWithZero",
	L"OpCompareGreaterEqual",
	L"OpCompareGreater",
	L"OpCompareEqual",
	L"OpCompareNotEqual",
	L"OpJumpIfZero",
	L"OpJump",
	L"OpTrace"
};

struct Instruction
{
	uint8_t op;
	uint8_t dest;
	union
	{
		uint8_t src[4];
		int32_t offset;
	};

	Instruction()
	:	op(0)
	,	dest(0)
	,	offset(0)
	{
	}

	Instruction(uint8_t op, uint8_t dest, uint8_t src1, uint8_t src2, uint8_t src3, uint8_t src4)
	{
		this->op = op;
		this->dest = dest;
		this->src[0] = src1;
		this->src[1] = src2;
		this->src[2] = src3;
		this->src[3] = src4;
	}

	Instruction(uint8_t op, uint8_t dest, int32_t offset)
	{
		this->op = op;
		this->dest = dest;
		this->offset = offset;
	}
};

inline int32_t getRelativeOffset(uint32_t jumpFrom, uint32_t jumpTo)
{
	return int32_t(jumpTo - jumpFrom);
}

//@}

	}
}

#endif	// traktor_render_sw_Types_H
