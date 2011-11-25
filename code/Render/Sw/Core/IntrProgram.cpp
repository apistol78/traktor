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

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint8_t >(L"op", m_ref.op);
		s >> Member< uint8_t >(L"dest", m_ref.dest);
		s >> MemberStaticArray< uint8_t, 4 >(L"src", m_ref.src);
		return true;
	}

private:
	value_type& m_ref;
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

		switch (i->op)
		{
		case OpFetchConstant:
			{
				const Vector4& constant = m_constants[i->src[0]];
				os << offset << L": " << c_opcodeText[i->op] << L" R" << int32_t(i->dest) << L" = " << constant.x() << L" " << constant.y() << L" " << constant.z() << L" " << constant.w() << Endl;
			}
			break;

		case OpFetchUniform:
		case OpFetchIndexedUniform:
			{
				std::wstring uniform = L"(Undefined)";
				for (std::map< std::wstring, Variable* >::const_iterator j = uniforms.begin(); j != uniforms.end(); ++j)
				{
					if (j->second->reg == i->src[0])
					{
						uniform = j->first;
						break;
					}
				}
				if (i->op == OpFetchUniform)
					os << offset << L": " << c_opcodeText[i->op] << L" R" << int32_t(i->dest) << L" = " << uniform << L" (" << int32_t(i->src[0]) << L")" << Endl;
				else
					os << offset << L": " << c_opcodeText[i->op] << L" R" << int32_t(i->dest) << L"-" << int32_t(i->dest + i->src[1] - 1) << L" = " << uniform << L" (" << int32_t(i->src[0]) << L")" << L" [R" << int32_t(i->src[2]) << L" * " << int32_t(i->src[1]) << L"]" << Endl;
			}
			break;

		case OpSwizzle:
			{
				uint8_t ch = i->src[0];

				uint8_t cx = (ch & 0xc0) >> 6;
				uint8_t cy = (ch & 0x30) >> 4;
				uint8_t cz = (ch & 0x0c) >> 2;
				uint8_t cw = (ch & 0x03);

				const wchar_t xyzw[] = L"xyzw";
				wchar_t swizzled[] = { xyzw[cx], xyzw[cy], xyzw[cz], xyzw[cw], 0 };

				os << offset << L": " << c_opcodeText[i->op] << L" R" << int32_t(i->dest) << L" = R" << int32_t(i->src[1]) << L"." << swizzled << Endl;
			}
			break;

		case OpSet:
			{
				wchar_t xyzw[] = L"xyzw";

				if (i->src[0] & 1)
					xyzw[0] = L'1';
				if (i->src[0] & 2)
					xyzw[1] = L'1';
				if (i->src[0] & 4)
					xyzw[2] = L'1';
				if (i->src[0] & 8)
					xyzw[3] = L'1';

				if (i->src[1] & 1)
					xyzw[0] = L'0';
				if (i->src[1] & 2)
					xyzw[1] = L'0';
				if (i->src[1] & 4)
					xyzw[2] = L'0';
				if (i->src[1] & 8)
					xyzw[3] = L'0';

				os << offset << L": " << c_opcodeText[i->op] << L" R" << int32_t(i->dest) << L" = R" << int32_t(i->dest) << L"." << xyzw << Endl;
			}
			break;

		case OpJump:
			os << offset << L": " << c_opcodeText[i->op] << L" " << int32_t(i->offset) << L" (" << int32_t(offset + i->offset) << L")" << Endl;
			break;

		case OpJumpIfZero:
			os << offset << L": " << c_opcodeText[i->op] << L" " << int32_t(i->dest) << L" - " << int32_t(i->offset) << L" (" << int32_t(offset + i->offset) << L")" << Endl;
			break;

		default:
			os << offset << L": " << c_opcodeText[i->op] << L" R" << int32_t(i->dest) << L" - " << int32_t(i->src[0]) << L" " << int32_t(i->src[1]) << L" " << int32_t(i->src[2]) << L" " << int32_t(i->src[3]) << Endl;
			break;
		}
	}

	os << L"--- End ---" << Endl;
}

bool IntrProgram::serialize(ISerializer& s)
{
	s >> MemberStlVector< Instruction, MemberInstruction >(L"instructions", m_instructions);
	s >> MemberAlignedVector< Vector4 >(L"constants", m_constants);
	return true;
}

	}
}
