#include <algorithm>
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/Emitter/Variable.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberInstruction : public MemberComplex
{
public:
	typedef Instruction value_type;

	MemberInstruction(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< uint8_t >(L"op", m_ref.op);
		s >> Member< uint8_t >(L"dest", m_ref.dest);
		s >> MemberStaticArray< uint8_t, 4 >(L"src", m_ref.src);
	}

private:
	value_type& m_ref;
};

#define DEF_DUMP_FN(op) \
	void dump_##op##(OutputStream& os, uint32_t offset, const Instruction& inst, const AlignedVector< Vector4 >& constants, const std::map< std::wstring, Variable* >& uniforms)

DEF_DUMP_FN(OpFetchConstant)
{
	const Vector4& constant = constants[inst.src[0]];
	os << offset << L": OpFetchConstant       R" << int32_t(inst.dest) << L" = " << constant.x() << L" " << constant.y() << L" " << constant.z() << L" " << constant.w() << Endl;
}

DEF_DUMP_FN(OpFetchTargetSize)
{
	os << offset << L": OpFetchTargetSize     R" << int32_t(inst.dest) << L" = (Size)" << Endl;
}

DEF_DUMP_FN(OpFetchUniform)
{
	std::wstring uniform = L"(Undefined)";
	for (std::map< std::wstring, Variable* >::const_iterator i = uniforms.begin(); i != uniforms.end(); ++i)
	{
		if (i->second->reg == inst.src[0])
		{
			uniform = i->first;
			break;
		}
	}
	os << offset << L": OpFetchUniform        R" << int32_t(inst.dest) << L" = " << uniform << L" (" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpFetchIndexedUniform)
{
	std::wstring uniform = L"(Undefined)";
	for (std::map< std::wstring, Variable* >::const_iterator i = uniforms.begin(); i != uniforms.end(); ++i)
	{
		if (i->second->reg == inst.src[0])
		{
			uniform = i->first;
			break;
		}
	}
	os << offset << L": OpFetchIndexedUniform R[" << int32_t(inst.dest) << L"-" << int32_t(inst.dest + inst.src[1] - 1) << L"] = " << uniform << L" (" << int32_t(inst.src[0]) << L")" << L" [R" << int32_t(inst.src[2]) << L" * " << int32_t(inst.src[1]) << L"]" << Endl;
}

DEF_DUMP_FN(OpFetchVarying)
{
	os << offset << L": OpFetchVarying        R" << int32_t(inst.dest) << L" = Vin[" << int32_t(inst.src[0]) << L"]" << Endl;
}

DEF_DUMP_FN(OpStoreVarying)
{
	os << offset << L": OpStoreVarying        Vout[" << int32_t(inst.dest) << L"] = R" << int32_t(inst.src[0]) << Endl;
}

DEF_DUMP_FN(OpMove)
{
	os << offset << L": OpMove                R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << Endl;
}

DEF_DUMP_FN(OpAbs)
{
	os << offset << L": OpAbs                 R" << int32_t(inst.dest) << L" = |R" << int32_t(inst.src[0]) << L"|" << Endl;
}

DEF_DUMP_FN(OpIncrement)
{
	os << offset << L": OpIncrement           R" << int32_t(inst.dest) << L"++" << Endl;
}

DEF_DUMP_FN(OpDecrement)
{
	os << offset << L": OpDecrement           R" << int32_t(inst.dest) << L"--" << Endl;
}

DEF_DUMP_FN(OpAdd)
{
	os << offset << L": OpAdd                 R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" + R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpDiv)
{
	os << offset << L": OpDiv                 R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" / R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpMul)
{
	os << offset << L": OpMul                 R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" * R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpMulAdd)
{
	os << offset << L": OpMulAdd              R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" * R" << int32_t(inst.src[1]) << L" + R" << int32_t(inst.src[2]) << Endl;
}

DEF_DUMP_FN(OpLog)
{
	os << offset << L": OpLog                 R" << int32_t(inst.dest) << L" = log(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpLog10)
{
	os << offset << L": OpLog10               R" << int32_t(inst.dest) << L" = log10(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpExp)
{
	os << offset << L": OpExp                 R" << int32_t(inst.dest) << L" = exp(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpFraction)
{
	os << offset << L": OpFraction            R" << int32_t(inst.dest) << L" = fract(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpNeg)
{
	os << offset << L": OpNeg                 R" << int32_t(inst.dest) << L" = -R" << int32_t(inst.src[0]) << Endl;
}

DEF_DUMP_FN(OpPow)
{
	os << offset << L": OpPow                 R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" ^ R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpSqrt)
{
	os << offset << L": OpSqrt                R" << int32_t(inst.dest) << L" = sqrt(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpSub)
{
	os << offset << L": OpSub                 R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" - R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpAcos)
{
	os << offset << L": OpAcos                R" << int32_t(inst.dest) << L" = acos(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpAtan)
{
	os << offset << L": OpAtan                R" << int32_t(inst.dest) << L" = atan(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpCos)
{
	os << offset << L": OpCos                 R" << int32_t(inst.dest) << L" = cos(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpSin)
{
	os << offset << L": OpSin                 R" << int32_t(inst.dest) << L" = sin(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpTan)
{
	os << offset << L": OpTan                 R" << int32_t(inst.dest) << L" = tan(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpCross)
{
	os << offset << L": OpCross               R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" x R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpDot3)
{
	os << offset << L": OpDot3                R" << int32_t(inst.dest) << L" = dot3(R" << int32_t(inst.src[0]) << L", R" << int32_t(inst.src[1]) << L")" << Endl;
}

DEF_DUMP_FN(OpDot4)
{
	os << offset << L": OpDot4                R" << int32_t(inst.dest) << L" = dot4(R" << int32_t(inst.src[0]) << L", R" << int32_t(inst.src[1]) << L")" << Endl;
}

DEF_DUMP_FN(OpLength)
{
	os << offset << L": OpLength              R" << int32_t(inst.dest) << L" = length(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpNormalize)
{
	os << offset << L": OpNormalize           R" << int32_t(inst.dest) << L" = normalize(R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpTransform)
{
	os << offset << L": OpTransform           R" << int32_t(inst.dest) << L" = transform(R" << int32_t(inst.src[0]) << L", R[" << int32_t(inst.src[1]) << L"-" << int32_t(inst.src[1] + 3) << L"])" << Endl;
}

DEF_DUMP_FN(OpTranspose)
{
	os << offset << L": OpTranspose           R[" << int32_t(inst.dest) << L"-" << int32_t(inst.dest + 3) << L"] = transpose(R[" << int32_t(inst.src[0]) << L"-" << int32_t(inst.src[0] + 3) << L"])" << Endl;
}

DEF_DUMP_FN(OpClamp)
{
	os << offset << L": OpClamp               R" << int32_t(inst.dest) << L" = clamp(R" << int32_t(inst.src[0]) << L", min R" << int32_t(inst.src[1]) << L", max R" << int32_t(inst.src[2]) << L")" << Endl;
}

DEF_DUMP_FN(OpLerp)
{
	os << offset << L": OpLerp                R" << int32_t(inst.dest) << L" = lerp(R" << int32_t(inst.src[0]) << L", from R" << int32_t(inst.src[1]) << L", to R" << int32_t(inst.src[2]) << L")" << Endl;
}

DEF_DUMP_FN(OpMixIn)
{
	os << offset << L": OpMixIn               R" << int32_t(inst.dest) << L" = { R" << int32_t(inst.src[0]) << L".x, R" << int32_t(inst.src[1]) << L".x, R" << int32_t(inst.src[2]) << L".x, R" << int32_t(inst.src[3]) << L".x }" << Endl;
}

DEF_DUMP_FN(OpMax)
{
	os << offset << L": OpMax                 R" << int32_t(inst.dest) << L" = max(R" << int32_t(inst.src[0]) << L", R" << int32_t(inst.src[1]) << L")" << Endl;
}

DEF_DUMP_FN(OpSampler)
{
	os << offset << L": OpSampler             R" << int32_t(inst.dest) << L" = sampler(S" << int32_t(inst.src[1]) << L", uv R" << int32_t(inst.src[0]) << L")" << Endl;
}

DEF_DUMP_FN(OpSwizzle)
{
	uint8_t ch = inst.src[0];

	uint8_t cx = (ch & 0xc0) >> 6;
	uint8_t cy = (ch & 0x30) >> 4;
	uint8_t cz = (ch & 0x0c) >> 2;
	uint8_t cw = (ch & 0x03);

	const wchar_t mask[] = { L"xyzw" };

	os << offset << L": OpSwizzle             R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[1]) << L"." << mask[cx] << mask[cy] << mask[cz] << mask[cw] << Endl;
}

DEF_DUMP_FN(OpSet)
{
	uint8_t ones = inst.src[0];
	uint8_t zeros = inst.src[1];

	wchar_t mask[] = { L"xyzw" };

	if (ones & 1)
		mask[0] = L'1';
	if (ones & 2)
		mask[1] = L'1';
	if (ones & 4)
		mask[2] = L'1';
	if (ones & 8)
		mask[3] = L'1';
	
	if (zeros & 1)
		mask[0] = L'0';
	if (zeros & 2)
		mask[1] = L'0';
	if (zeros & 4)
		mask[2] = L'0';
	if (zeros & 8)
		mask[3] = L'0';

	os << offset << L": OpSet                 R" << int32_t(inst.dest) << L" = R" << int32_t(inst.dest) << L"." << mask << Endl;
}

DEF_DUMP_FN(OpExpandWithZero)
{
	uint8_t zeros = inst.src[1];

	wchar_t mask[] = { L"xyzw" };

	if (zeros & 1)
		mask[0] = L'0';
	if (zeros & 2)
		mask[1] = L'0';
	if (zeros & 4)
		mask[2] = L'0';
	if (zeros & 8)
		mask[3] = L'0';

	os << offset << L": OpExpandWithZero      R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L"." << mask << Endl;
}

DEF_DUMP_FN(OpCompareGreaterEqual)
{
	os << offset << L": OpCompareGreaterEqual R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" >= R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpCompareGreater)
{
	os << offset << L": OpCompareGreater      R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" > R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpCompareEqual)
{
	os << offset << L": OpCompareEqual        R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" == R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpCompareNotEqual)
{
	os << offset << L": OpCompareNotEqual     R" << int32_t(inst.dest) << L" = R" << int32_t(inst.src[0]) << L" != R" << int32_t(inst.src[1]) << Endl;
}

DEF_DUMP_FN(OpJumpIfZero)
{
	os << offset << L": OpJumpIfZero          R" << int32_t(inst.dest) << L" == 0 ? " << int32_t(offset + inst.offset) << Endl;
}

DEF_DUMP_FN(OpJump)
{
	os << offset << L": OpJump                " << int32_t(offset + inst.offset) << Endl;
}

DEF_DUMP_FN(OpTrace)
{
	os << offset << L": OpTrace               " << int32_t(inst.dest) << Endl;
}

#define DECL_DUMP_FN(op) \
	{ op, &dump_##op## }

const struct DumpInfo
{
	uint8_t op;
	void (*formatter)(OutputStream& os, uint32_t offset, const Instruction& inst, const AlignedVector< Vector4 >& constants, const std::map< std::wstring, Variable* >& uniforms);
}
c_dumpInfo[] =
{
	DECL_DUMP_FN(OpFetchConstant),
	DECL_DUMP_FN(OpFetchTargetSize),
	DECL_DUMP_FN(OpFetchUniform),
	DECL_DUMP_FN(OpFetchIndexedUniform),
	DECL_DUMP_FN(OpFetchVarying),
	DECL_DUMP_FN(OpStoreVarying),
	DECL_DUMP_FN(OpMove),
	DECL_DUMP_FN(OpAbs),
	DECL_DUMP_FN(OpIncrement),
	DECL_DUMP_FN(OpDecrement),
	DECL_DUMP_FN(OpAdd),
	DECL_DUMP_FN(OpDiv),
	DECL_DUMP_FN(OpMul),
	DECL_DUMP_FN(OpMulAdd),
	DECL_DUMP_FN(OpLog),
	DECL_DUMP_FN(OpLog10),
	DECL_DUMP_FN(OpExp),
	DECL_DUMP_FN(OpFraction),
	DECL_DUMP_FN(OpNeg),
	DECL_DUMP_FN(OpPow),
	DECL_DUMP_FN(OpSqrt),
	DECL_DUMP_FN(OpSub),
	DECL_DUMP_FN(OpAcos),
	DECL_DUMP_FN(OpAtan),
	DECL_DUMP_FN(OpCos),
	DECL_DUMP_FN(OpSin),
	DECL_DUMP_FN(OpTan),
	DECL_DUMP_FN(OpCross),
	DECL_DUMP_FN(OpDot3),
	DECL_DUMP_FN(OpDot4),
	DECL_DUMP_FN(OpLength),
	DECL_DUMP_FN(OpNormalize),
	DECL_DUMP_FN(OpTransform),
	DECL_DUMP_FN(OpTranspose),
	DECL_DUMP_FN(OpClamp),
	DECL_DUMP_FN(OpLerp),
	DECL_DUMP_FN(OpMixIn),
	DECL_DUMP_FN(OpMax),
	DECL_DUMP_FN(OpSampler),
	DECL_DUMP_FN(OpSwizzle),
	DECL_DUMP_FN(OpSet),
	DECL_DUMP_FN(OpExpandWithZero),
	DECL_DUMP_FN(OpCompareGreaterEqual),
	DECL_DUMP_FN(OpCompareGreater),
	DECL_DUMP_FN(OpCompareEqual),
	DECL_DUMP_FN(OpCompareNotEqual),
	DECL_DUMP_FN(OpJumpIfZero),
	DECL_DUMP_FN(OpJump),
	DECL_DUMP_FN(OpTrace)
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IntrProgram", 0, IntrProgram, ISerializable)

uint32_t IntrProgram::addInstruction(const Instruction& instruction)
{
	m_instructions.push_back(instruction);
	return uint32_t(m_instructions.size() - 1);
}

uint32_t IntrProgram::addConstant(const Vector4& value)
{
	AlignedVector< Vector4 >::iterator i = std::find(m_constants.begin(), m_constants.end(), value);
	if (i != m_constants.end())
		return int(std::distance(m_constants.begin(), i));

	m_constants.push_back(value);
	return uint32_t(m_constants.size() - 1);
}

void IntrProgram::setInstruction(uint32_t offset, const Instruction& instruction)
{
	m_instructions[offset] = instruction;
}

void IntrProgram::setConstant(uint32_t index, const Vector4& value)
{
	m_constants[index] = value;
}

void IntrProgram::dump(OutputStream& os, const std::map< std::wstring, Variable* >& uniforms) const
{
	os << L"--- Begin ---" << Endl;

	os << L"# Constants" << Endl;
	for (AlignedVector< Vector4 >::const_iterator i = m_constants.begin(); i != m_constants.end(); ++i)
		os << uint32_t(std::distance(m_constants.begin(), i)) << L" : " << i->x() << L" " << i->y() << L" " << i->z() << L" " << i->w() << Endl;
	os << Endl;

	os << L"# Instructions" << Endl;
	for (std::vector< Instruction >::const_iterator i = m_instructions.begin(); i != m_instructions.end(); ++i)
	{
		T_ASSERT (i->op < sizeof_array(c_opcodeText));

		uint32_t offset = uint32_t(std::distance(m_instructions.begin(), i));

		bool foundInfo = false;
		for (uint32_t j = 0; j < sizeof_array(c_dumpInfo); ++j)
		{
			if (c_dumpInfo[j].op == i->op)
			{
				c_dumpInfo[j].formatter(os, offset, *i, m_constants, uniforms);
				foundInfo = true;
				break;
			}
		}

		if (!foundInfo)
			os << offset << L": " << c_opcodeText[i->op] << L" R" << int32_t(i->dest) << L" - " << int32_t(i->src[0]) << L" " << int32_t(i->src[1]) << L" " << int32_t(i->src[2]) << L" " << int32_t(i->src[3]) << Endl;
	}

	os << L"--- End ---" << Endl;
}

void IntrProgram::serialize(ISerializer& s)
{
	s >> MemberStlVector< Instruction, MemberInstruction >(L"instructions", m_instructions);
	s >> MemberAlignedVector< Vector4 >(L"constants", m_constants);
}

	}
}
