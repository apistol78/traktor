#include <windows.h>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Memory/Alloc.h"
#include "Render/Sw/Core/IntrProgram.h"
#include "Render/Sw/Core/Sampler.h"
#include "Render/Sw/Core/x86/Assembler.h"
#include "Render/Sw/Core/x86/JitX86.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_trigResolution = 65536;

#define Z  0U
#define O ~0U

#define R0 256
#define R1 257

#define REGISTERS						ecx
#define CONTEXT							esi
#define CONTEXT_OFFSET_CONSTANTS		0
#define CONTEXT_OFFSET_INSTANCE			4
#define CONTEXT_OFFSET_TARGETSIZE		8
#define CONTEXT_OFFSET_FRAGMENTPOSITION	12
#define CONTEXT_OFFSET_UNIFORMS			16
#define CONTEXT_OFFSET_VARYINGS			20
#define CONTEXT_OFFSET_SAMPLERS			24
#define CONTEXT_OFFSET_OUT				28

#pragma pack(1)
struct ExecuteContext
{
	void* constants;
	void* instance;
	void* targetSize;
	void* fragmentPosition;
	void* uniforms;
	void* varyings;
	void* samplers;
	void* out;
};
#pragma pack()

struct InternalImage
{
	float constants[256 * 4];
	void* native;
};

const uint32_t T_ALIGN16 s_absoluteMask[]		= { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const uint32_t T_ALIGN16 s_negateMask[]			= { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const uint32_t T_ALIGN16 s_negateCrossMask[]	= { 0x00000000, 0x80000000, 0x00000000, 0x00000000 };
const uint32_t T_ALIGN16 s_zero[]				= { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
const uint32_t T_ALIGN16 s_one[]				= { 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000 };
const uint32_t T_ALIGN16 s_masks[] =
{
	Z, Z, Z, Z,
	O, Z, Z, Z,
	Z, O, Z, Z,
	O, O, Z, Z,
	Z, Z, O, Z,
	O, Z, O, Z,
	Z, O, O, Z,
	O, O, O, Z,
	Z, Z, Z, O,
	O, Z, Z, O,
	Z, O, Z, O,
	O, O, Z, O,
	Z, Z, O, O,
	O, Z, O, O,
	Z, O, O, O,
	O, O, O, O
};

const float T_ALIGN16 s_trigCosPhase[] = { PI / 2.0f, PI / 2.0f, PI / 2.0f, PI / 2.0f };
const float T_ALIGN16 s_trigSinCosScale[] = { (c_trigResolution - 1) / (2.0f * PI), (c_trigResolution - 1) / (2.0f * PI), (c_trigResolution - 1) / (2.0f * PI), (c_trigResolution - 1) / (2.0f * PI) };
const float T_ALIGN16 s_trigArcCosOffset[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float T_ALIGN16 s_trigArcCosScale[] = { (c_trigResolution - 1) / 2.0f, (c_trigResolution - 1) / 2.0f, (c_trigResolution - 1) / 2.0f, (c_trigResolution - 1) / 2.0f };
static Vector4 T_ALIGN16 s_trigSin[c_trigResolution];
static Vector4 T_ALIGN16 s_trigArcCos[c_trigResolution];

#define SSE_SHUFFLE_MASK(x, y, z, w) \
	(x) | ((y) << 2) | ((z) << 4) | ((w) << 6)

void __stdcall callPow(Vector4* registers, ExecuteContext* context, uint32_t dest, uint32_t src0, uint32_t src1)
{
	registers[dest].set(
		std::powf(registers[src0].x(), registers[src1].x()),
		std::powf(registers[src0].y(), registers[src1].y()),
		std::powf(registers[src0].z(), registers[src1].z()),
		std::powf(registers[src0].w(), registers[src1].w())
	);
}

void __stdcall callSampler(Vector4* registers, ExecuteContext* context, uint32_t dest, uint32_t src0, uint32_t src1)
{
	const Ref< AbstractSampler >* samplers = reinterpret_cast< const Ref< AbstractSampler >* >(context->samplers);
	if (samplers && samplers[src1])
		registers[dest] = samplers[src1]->get(registers[src0]);
}

static
struct XmmActive
{
	XmmRegister xmm;
	uint16_t reg;
	uint32_t last;
} s_xmmActive[] =
{
	{ xmm0, 0xffff, 0 },
	{ xmm1, 0xffff, 0 },
	{ xmm2, 0xffff, 0 },
	{ xmm3, 0xffff, 0 },
	{ xmm4, 0xffff, 0 },
	{ xmm5, 0xffff, 0 },
	{ xmm6, 0xffff, 0 },
	{ xmm7, 0xffff, 0 }
};

void initializeRegisterAllocator()
{
	s_xmmActive[0].reg = 0xffff; s_xmmActive[0].last = 0;
	s_xmmActive[1].reg = 0xffff; s_xmmActive[1].last = 0;
	s_xmmActive[2].reg = 0xffff; s_xmmActive[2].last = 0;
	s_xmmActive[3].reg = 0xffff; s_xmmActive[3].last = 0;
	s_xmmActive[4].reg = 0xffff; s_xmmActive[4].last = 0;
	s_xmmActive[5].reg = 0xffff; s_xmmActive[5].last = 0;
	s_xmmActive[6].reg = 0xffff; s_xmmActive[6].last = 0;
	s_xmmActive[7].reg = 0xffff; s_xmmActive[7].last = 0;
}

const XmmRegister& xmmr(Assembler& a, uint16_t reg)
{
	// Do we have "reg" loaded in some xmm registry already?
	for (uint32_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		if (s_xmmActive[i].reg == reg)
		{
			s_xmmActive[i].last = a.size();
			return s_xmmActive[i].xmm;
		}
	}

	// No, allocate xmm registry, preferably anyone unused but otherwise the oldest.
	for (uint32_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		if (s_xmmActive[i].reg == 0xffff)
		{
			s_xmmActive[i].reg = reg;
			s_xmmActive[i].last = a.size();
			a.movaps(s_xmmActive[i].xmm, REGISTERS, reg * 16);
			return s_xmmActive[i].xmm;
		}
	}

	// Spill xmm registry into memory and reload with new registry value.
	uint32_t furthest = 0, spill = 0;
	for (uint32_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		uint32_t distance = a.size() - s_xmmActive[i].last;
		if (distance >= furthest)
		{
			furthest = distance;
			spill = i;
		}
	}

	a.movaps(REGISTERS, s_xmmActive[spill].reg * 16, s_xmmActive[spill].xmm);
	s_xmmActive[spill].reg = reg;
	s_xmmActive[spill].last = a.size();
	a.movaps(s_xmmActive[spill].xmm, REGISTERS, reg * 16);

	return s_xmmActive[spill].xmm;
}

// Direct write to register
const XmmRegister& xmmw(Assembler& a, uint16_t reg)
{
	// Do we have "reg" loaded in some xmm registry already?
	for (uint32_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		if (s_xmmActive[i].reg == reg)
			return s_xmmActive[i].xmm;
	}

	// No, allocate xmm registry, preferably anyone unused but otherwise the oldest.
	for (uint32_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		if (s_xmmActive[i].reg == 0xffff)
		{
			s_xmmActive[i].reg = reg;
			return s_xmmActive[i].xmm;
		}
	}

	// Spill xmm registry into memory and reload with new registry value.
	uint32_t furthest = 0, spill = 0;
	for (uint32_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		uint32_t distance = a.size() - s_xmmActive[i].last;
		if (distance >= furthest)
		{
			furthest = distance;
			spill = i;
		}
	}

	a.movaps(REGISTERS, s_xmmActive[spill].reg * 16, s_xmmActive[spill].xmm);
	s_xmmActive[spill].reg = reg;

	return s_xmmActive[spill].xmm;
}

// Spill register into memory.
void xmmrFlush(Assembler& a, uint16_t reg)
{
	for (size_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		if (s_xmmActive[i].reg == reg)
		{
			a.movaps(REGISTERS, reg * 16, s_xmmActive[i].xmm);
			s_xmmActive[i].reg = 0xffff;
			return;
		}
	}
}

// Spill all registers into memory.
void xmmrFlushAll(Assembler& a)
{
	for (size_t i = 0; i < sizeof_array(s_xmmActive); ++i)
	{
		if (s_xmmActive[i].reg != 0xffff)
		{
			a.movaps(REGISTERS, s_xmmActive[i].reg * 16, s_xmmActive[i].xmm);
			s_xmmActive[i].reg = 0xffff;
		}
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.JitX86", JitX86, Processor)

JitX86::JitX86()
{
	float v;
	for (uint32_t i = 0; i < c_trigResolution; ++i)
	{
		v = sinf(i * PI * 2.0f / (c_trigResolution - 1));
		s_trigSin[i].set(v, v, v, v);

		v = acos(-1.0f + float(i * 2.0f) / (c_trigResolution - 1));
		s_trigArcCos[i].set(v, v, v, v);
	}
}

Processor::image_t JitX86::compile(const IntrProgram& program) const
{
	Assembler a;

	initializeRegisterAllocator();

	const std::vector< Instruction >& instructions = program.getInstructions();
	for (uint32_t offset = 0; offset < uint32_t(instructions.size()); ++offset)
	{
		const Instruction& i = instructions[offset];

		// If this instruction is a target of any jump we need to flush the register
		// allocator.
		bool jumpTarget = false;
		for (uint32_t offset2 = 0; offset2 < uint32_t(instructions.size()); ++offset2)
		{
			const Instruction& i2 = instructions[offset2];
			if ((i2.op == OpJump || i2.op == OpJumpIfZero) && offset2 + i2.offset == offset)
			{
				jumpTarget = true;
				break;
			}
		}

		// Flush all registers.
		if (jumpTarget)
			xmmrFlushAll(a);

		a.label(offset);

		switch (i.op)
		{
		case OpNop:
			break;

		case OpFetchConstant:
			a.lea(eax, CONTEXT, CONTEXT_OFFSET_CONSTANTS);
			a.movaps(xmmw(a, i.dest), eax, i.src[0] * 16);
			break;

		case OpFetchInstance:
			a.lea(eax, CONTEXT, CONTEXT_OFFSET_INSTANCE);
			a.movaps(xmmw(a, i.dest), eax, 0);
			break;

		case OpFetchTargetSize:
			a.lea(eax, CONTEXT, CONTEXT_OFFSET_TARGETSIZE);
			a.movaps(xmmw(a, i.dest), eax, 0);
			break;

		case OpFetchFragmentPosition:
			a.lea(eax, CONTEXT, CONTEXT_OFFSET_FRAGMENTPOSITION);
			a.movaps(xmmw(a, i.dest), eax, 0);
			break;

		case OpFetchUniform:
			{
				uint32_t base = i.src[0];
				uint32_t size = i.src[1];
				T_ASSERT (base + size < 256);

				a.lea(eax, CONTEXT, CONTEXT_OFFSET_UNIFORMS);
				for (uint32_t j = 0; j < size; ++j)
					a.movaps(xmmw(a, i.dest + j), eax, (base + j) * 16);
			}
			break;

		case OpFetchIndexedUniform:
			{
				uint32_t base = i.src[0];
				uint32_t size = i.src[1];

				a.lea(eax, CONTEXT, CONTEXT_OFFSET_UNIFORMS);
				a.cvtss2si(edx, xmmr(a, i.src[2]));		// edx = round(index)
				a.mul(edx, size * 16);					// edx = index * size * 16
				a.add(edx, eax);						// edx = inUniforms + index * size * 16

				for (uint32_t j = 0; j < size; ++j)
					a.movaps(xmmw(a, i.dest + j), edx, (base + j) * 16);
			}
			break;

		case OpFetchVarying:
			a.lea(eax, CONTEXT, CONTEXT_OFFSET_VARYINGS);
			a.movaps(xmmw(a, i.dest), eax, i.src[0] * 16);
			break;

		case OpStoreVarying:
			a.lea(eax, CONTEXT, CONTEXT_OFFSET_OUT);
			a.movaps(eax, i.dest * 16, xmmr(a, i.src[0]));
			break;

		case OpMove:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			break;

		case OpAbs:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.andps(xmmr(a, i.dest), XmmWordPtr(s_absoluteMask));
			break;

		case OpIncrement:
			a.movaps(xmmw(a, R0), XmmWordPtr(s_one));
			a.addps(xmmr(a, i.dest), xmmr(a, R0));
			break;

		case OpDecrement:
			a.movaps(xmmw(a, R0), XmmWordPtr(s_one));
			a.subps(xmmr(a, i.dest), xmmr(a, R0));
			break;

		case OpAdd:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.addps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpDiv:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.divps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpMul:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.mulps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpMulAdd:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.mulps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			a.addps(xmmr(a, i.dest), xmmr(a, i.src[2]));
			break;

		//case OpLog:
		//case OpLog2:
		//case OpLog10:
		//case OpExp:
		//	break;

		case OpFraction:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.cvttps2dq(xmmw(a, R0), xmmr(a, i.dest));
			a.cvtdq2ps(xmmr(a, R0), xmmr(a, R0));
			a.subps(xmmr(a, i.dest), xmmr(a, R0));
			break;

		case OpNeg:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.xorps(xmmr(a, i.dest), XmmWordPtr(s_negateMask));
			break;

		case OpPow:
			xmmrFlush(a, i.src[0]);
			xmmrFlush(a, i.src[1]);
			xmmrFlush(a, i.dest);
			a.push(CONTEXT);
			a.push(REGISTERS);
			a.push(i.src[1]);
			a.push(i.src[0]);
			a.push(i.dest);
			a.push(CONTEXT);
			a.push(REGISTERS);
			a.call((void*)callPow);
			a.pop(REGISTERS);
			a.pop(CONTEXT);
			break;

		case OpSqrt:
			a.sqrtps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			break;

		case OpRecipSqrt:
			break;

		case OpSub:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.subps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpAcos:
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.movaps(xmmw(a, R1), XmmWordPtr(&s_trigArcCosOffset));
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmr(a, R1), XmmWordPtr(&s_trigArcCosScale));
			a.mulps(xmmr(a, R0), xmmr(a, R1));

			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigArcCos));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[1 * 4]));
			a.movaps(xmmw(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigArcCos));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[2 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigArcCos));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[4 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigArcCos));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[8 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));
			break;

		//case OpAtan:

		case OpCos:
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.movaps(xmmw(a, R1), XmmWordPtr(&s_trigCosPhase));
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), XmmWordPtr(&s_trigSinCosScale));
			a.mulps(xmmr(a, R0), xmmr(a, R1));

			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[1 * 4]));
			a.movaps(xmmw(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[2 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[4 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[8 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));
			break;

		case OpSin:
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.movaps(xmmw(a, R1), XmmWordPtr(&s_trigSinCosScale));
			a.mulps(xmmr(a, R0), xmmr(a, R1));

			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[1 * 4]));
			a.movaps(xmmw(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[2 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[4 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));

			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(1, 2, 3, 0));		// Rotate left
			a.cvttss2si(eax, xmmr(a, R0));
			a.and(eax, (c_trigResolution - 1));
			a.mul(eax, 16);
			a.movaps(xmmw(a, R1), eax, uint32_t(s_trigSin));
			a.andps(xmmr(a, R1), XmmWordPtr(&s_masks[8 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R1));
			break;

		//case OpTan:

		case OpCross:
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0]));
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[1]));
			a.shufps(xmmr(a, i.dest), xmmr(a, i.src[0]), 0xca);
			a.shufps(xmmr(a, R1), xmmr(a, i.src[1]), 0xd1);
			a.mulps(xmmr(a, i.dest), xmmr(a, R1));
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.movaps(xmmw(a, R1), xmmr(a, i.src[1]));
			a.shufps(xmmr(a, R0), xmmr(a, R1), 0xca);
			a.shufps(xmmr(a, R1), xmmr(a, i.src[0]), 0xd1);
			a.mulps(xmmr(a, R0), xmmr(a, R1));
			a.subps(xmmr(a, i.dest), xmmr(a, R0));
			a.xorps(xmmr(a, i.dest), XmmWordPtr(s_negateCrossMask));	// @hack Negate Y-axis, find out why it's negative in the first place.
			break;

		case OpDot3:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.mulps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			a.movaps(xmmw(a, R0), xmmr(a, i.dest));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0x55);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0xaa);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, i.dest), xmmr(a, R0));
			break;

		case OpDot4:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.mulps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			a.movaps(xmmw(a, R0), xmmr(a, i.dest));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0x55);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0xaa);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0xff);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, i.dest), xmmr(a, R0));
			break;

		case OpLength:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.mulps(xmmr(a, i.dest), xmmr(a, i.dest));
			a.movaps(xmmw(a, R0), xmmr(a, i.dest));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0x55);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0xaa);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0xff);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.sqrtps(xmmr(a, R0), xmmr(a, R0));
			a.movaps(xmmw(a, i.dest), xmmr(a, R0));
			break;

		case OpNormalize:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.mulps(xmmr(a, i.dest), xmmr(a, i.dest));
			a.movaps(xmmw(a, R0), xmmr(a, i.dest));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0x55);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0xaa);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), xmmr(a, i.dest));
			a.shufps(xmmr(a, R1), xmmr(a, R1), 0xff);
			a.addps(xmmr(a, R0), xmmr(a, R1));
			a.sqrtps(xmmr(a, R0), xmmr(a, R0));
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.divps(xmmr(a, i.dest), xmmr(a, R0));
			break;

		case OpTransform:
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);		// xmm1 = xxxx
			a.mulps(xmmr(a, R0), xmmr(a, i.src[1] + 0));	// xmm1 = x000'
			a.movaps(xmmw(a, i.dest), xmmr(a, R0));			// xmm? = x000'
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x55);		// xmm1 = yyyy
			a.mulps(xmmr(a, R0), xmmr(a, i.src[1] + 1));	// xmm1 = 0y00'
			a.addps(xmmr(a, i.dest), xmmr(a, R0));			// xmm? = xy00'
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0xaa);		// xmm1 = zzzz
			a.mulps(xmmr(a, R0), xmmr(a, i.src[1] + 2));	// xmm1 = 00z0'
			a.addps(xmmr(a, i.dest), xmmr(a, R0));			// xmm? = xyz0'
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0xff);		// xmm1 = wwww
			a.mulps(xmmr(a, R0), xmmr(a, i.src[1] + 3));	// xmm1 = 000w'
			a.addps(xmmr(a, i.dest), xmmr(a, R0));			// xmm? = xyzw'
			break;

		case OpTranspose:
			a.movaps(xmmw(a, R0), XmmWordPtr(&s_masks[1 * 4]));
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));				// dest = 00 01 02 03
			a.andps(xmmr(a, i.dest), xmmr(a, R0));						// dest = 00 . . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 1));						// R1 = 10 11 12 13
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = 10 . . .
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(1, 0, 2, 3));	// R1 = . 10 . .
			a.addps(xmmr(a, i.dest), xmmr(a, R1));								// dest = 00 10 . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 2));						// R1 = 20 21 22 23
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = 20 . . .
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(1, 2, 0, 3));	// R1 = . . 20 .
			a.addps(xmmr(a, i.dest), xmmr(a, R1));								// dest = 00 10 20 .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 3));						// R1 = 30 31 32 33
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = 30 . . .
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(1, 2, 3, 0));	// R1 = . . . 30
			a.addps(xmmr(a, i.dest), xmmr(a, R1));								// dest = 00 10 20 30
			xmmrFlush(a, i.dest);

			a.movaps(xmmw(a, R0), XmmWordPtr(&s_masks[2 * 4]));
			a.movaps(xmmw(a, i.dest + 1), xmmr(a, i.src[0]));									// dest = 00 01 02 03
			a.andps(xmmr(a, i.dest + 1), xmmr(a, R0));											// dest = . 01 . .
			a.shufps(xmmr(a, i.dest + 1), xmmr(a, i.dest + 1), SSE_SHUFFLE_MASK(1, 0, 2, 3));	// dest = 01 . . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 1));						// R1 = 10 11 12 13
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . 11 . .
			a.addps(xmmr(a, i.dest + 1), xmmr(a, R1));							// dest = 01 11 . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 2));						// R1 = 20 21 22 23
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . 21 . .
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(0, 2, 1, 3));	// R1 = . . 21 .
			a.addps(xmmr(a, i.dest + 1), xmmr(a, R1));							// dest = 01 11 21 .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 3));						// R1 = 30 31 32 33
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . 31 . .
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(0, 2, 3, 1));	// R1 = . . . 31
			a.addps(xmmr(a, i.dest + 1), xmmr(a, R1));							// dest = 01 11 21 31
			xmmrFlush(a, i.dest + 1);

			a.movaps(xmmw(a, R0), XmmWordPtr(&s_masks[4 * 4]));
			a.movaps(xmmw(a, i.dest + 2), xmmr(a, i.src[0]));									// dest = 00 01 02 03
			a.andps(xmmr(a, i.dest + 2), xmmr(a, R0));											// dest = . . 02 .
			a.shufps(xmmr(a, i.dest + 2), xmmr(a, i.dest + 2), SSE_SHUFFLE_MASK(2, 0, 1, 3));	// dest = 02 . . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 1));						// R1 = 10 11 12 13
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . . 12 .
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(0, 2, 1, 3));	// R1 = . 12 . .
			a.addps(xmmr(a, i.dest + 2), xmmr(a, R1));							// dest = 02 12 . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 2));						// R1 = 20 21 22 23
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . . 22 .
			a.addps(xmmr(a, i.dest + 2), xmmr(a, R1));							// dest = 02 12 22 .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 3));						// R1 = 30 31 32 33
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . . 32 .
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(0, 1, 3, 2));	// R1 = . . . 32
			a.addps(xmmr(a, i.dest + 2), xmmr(a, R1));							// dest = 02 12 22 32
			xmmrFlush(a, i.dest + 2);

			a.movaps(xmmw(a, R0), XmmWordPtr(&s_masks[8 * 4]));
			a.movaps(xmmw(a, i.dest + 3), xmmr(a, i.src[0]));									// dest = 00 01 02 03
			a.andps(xmmr(a, i.dest + 3), xmmr(a, R0));											// dest = . . . 03
			a.shufps(xmmr(a, i.dest + 3), xmmr(a, i.dest + 3), SSE_SHUFFLE_MASK(3, 0, 1, 2));	// dest = 03 . . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 1));						// R1 = 10 11 12 13
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . . . 13
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(0, 3, 1, 2));	// R1 = . 13 . .
			a.addps(xmmr(a, i.dest + 3), xmmr(a, R1));							// dest = 03 13 . .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 2));						// R1 = 20 21 22 23
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . . . 23
			a.shufps(xmmr(a, R1), xmmr(a, R1), SSE_SHUFFLE_MASK(0, 1, 3, 2));	// R1 = . . 23 .
			a.addps(xmmr(a, i.dest + 3), xmmr(a, R1));							// dest = 03 13 23 .
			a.movaps(xmmw(a, R1), xmmr(a, i.src[0] + 3));						// R1 = 30 31 32 33
			a.andps(xmmr(a, R1), xmmr(a, R0));									// R1 = . . . 33
			a.addps(xmmr(a, i.dest + 3), xmmr(a, R1));							// dest = 03 13 23 33
			xmmrFlush(a, i.dest + 3);
			break;

		case OpClamp:
			a.lea(eax, CONTEXT, CONTEXT_OFFSET_CONSTANTS);
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.movaps(xmmw(a, R1), eax, i.src[1] * 16);
			a.maxps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, R1), eax, i.src[2] * 16);
			a.minps(xmmr(a, R0), xmmr(a, R1));
			a.movaps(xmmw(a, i.dest), xmmr(a, R0));
			break;

		case OpTrunc:
			// \fixme
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			break;

		case OpRound:
			// \fixme
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			break;

		case OpLerp:
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));	// xmm0 = { blend, ... }
			a.shufps(xmmr(a, R0), xmmr(a, R0), SSE_SHUFFLE_MASK(0, 0, 0, 0));
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[2]));
			a.subps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			a.mulps(xmmr(a, i.dest), xmmr(a, R0));
			a.addps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpMixIn:
			a.movaps(xmmw(a, R0), xmmr(a, i.src[0]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.andps(xmmr(a, R0), XmmWordPtr(&s_masks[1 * 4]));
			a.movaps(xmmw(a, i.dest), xmmr(a, R0));
			a.movaps(xmmw(a, R0), xmmr(a, i.src[1]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.andps(xmmr(a, R0), XmmWordPtr(&s_masks[2 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R0));
			a.movaps(xmmw(a, R0), xmmr(a, i.src[2]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.andps(xmmr(a, R0), XmmWordPtr(&s_masks[4 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R0));
			a.movaps(xmmw(a, R0), xmmr(a, i.src[3]));
			a.shufps(xmmr(a, R0), xmmr(a, R0), 0x00);
			a.andps(xmmr(a, R0), XmmWordPtr(&s_masks[8 * 4]));
			a.addps(xmmr(a, i.dest), xmmr(a, R0));	
			break;

		case OpMin:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.minps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpMax:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.maxps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpSign:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			// \fixme
			break;

		case OpSampler:
			xmmrFlush(a, i.src[0]);
			xmmrFlush(a, i.src[1]);
			xmmrFlush(a, i.dest);
			a.push(CONTEXT);
			a.push(REGISTERS);
			a.push(i.src[1]);
			a.push(i.src[0]);
			a.push(i.dest);
			a.push(CONTEXT);
			a.push(REGISTERS);
			a.call((void*)callSampler);
			a.pop(REGISTERS);
			a.pop(CONTEXT);
			break;

		case OpSwizzle:
			{
				uint8_t ch = i.src[0];

				uint8_t cx = (ch & 0xc0) >> 6;
				uint8_t cy = (ch & 0x30) >> 4;
				uint8_t cz = (ch & 0x0c) >> 2;
				uint8_t cw = (ch & 0x03);

				uint8_t shuffle = cx | (cy << 2) | (cz << 4) | (cw << 6);

				a.movaps(xmmw(a, i.dest), xmmr(a, i.src[1]));
				a.shufps(xmmr(a, i.dest), xmmr(a, i.dest), shuffle);
			}
			break;

		case OpSet:
			if (i.src[0])	// Ones
			{
				a.andps(xmmr(a, i.dest), XmmWordPtr(&s_masks[((~i.src[0]) & 15) * 4]));
				a.movaps(xmmw(a, R0), XmmWordPtr(s_one));
				a.andps(xmmr(a, R0), XmmWordPtr(&s_masks[(( i.src[0]) & 15) * 4]));
				a.addps(xmmr(a, i.dest), xmmr(a, R0));
			}
			if (i.src[1])	// Zeros
			{
				a.andps(xmmr(a, i.dest), XmmWordPtr(&s_masks[((~i.src[1]) & 15) * 4]));
			}
			break;

		case OpExpandWithZero:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.andps(xmmr(a, i.dest), XmmWordPtr(&s_masks[((~i.src[1]) & 15) * 4]));
			break;

		case OpCompareGreaterEqual:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[1]));
			a.cmpleps(xmmr(a, i.dest), xmmr(a, i.src[0]));
			break;

		case OpCompareGreater:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[1]));
			a.cmpltps(xmmr(a, i.dest), xmmr(a, i.src[0]));
			break;

		case OpCompareEqual:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.cmpeqps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpCompareNotEqual:
			a.movaps(xmmw(a, i.dest), xmmr(a, i.src[0]));
			a.cmpneqps(xmmr(a, i.dest), xmmr(a, i.src[1]));
			break;

		case OpJumpIfZero:
			xmmrFlushAll(a);
			a.mov(eax, REGISTERS, i.dest * 16);
			a.test(eax, eax);
			a.jz(offset + i.offset);
			break;

		case OpJump:
			xmmrFlushAll(a);
			a.jmp(offset + i.offset);
			break;

		case OpDiscard:
			a.ret();
			break;

		case OpTrace:
			break;

		default:
			T_BREAKPOINT;
		}
	}

	a.ret();
	a.fixup();

	InternalImage* image = (InternalImage*)Alloc::acquireAlign(sizeof(InternalImage), 16, T_FILE_LINE);
	image->native = VirtualAlloc(NULL, a.get().size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	memset(image->constants, 0, 256 * 4 * sizeof(float));
	
	if (!program.getConstants().empty())
		memcpy(image->constants, &program.getConstants().front(), program.getConstants().size() * 4 * sizeof(float));

	memcpy(image->native, &a.get().front(), a.get().size());

	return image;
}

void JitX86::destroy(image_t image) const
{
	InternalImage* i = reinterpret_cast< InternalImage* >(image);
	VirtualFree(i->native, 0, MEM_RELEASE);
	Alloc::freeAlign(i);
}

bool JitX86::execute(
	const image_t image,
	int32_t instance,
	const Vector4* inUniforms,
	const Vector4* inVaryings,
	const Vector4& targetSize,
	const Vector4& fragmentPosition,
	const Ref< AbstractSampler >* inSamplers,
	Vector4* outVaryings
) const
{
	InternalImage* i = reinterpret_cast< InternalImage* >(image);
	void* native = i->native;

	float T_ALIGN16 instance4f[4];
	float T_ALIGN16 registers[258 * 4];

	instance4f[0] =
	instance4f[1] =
	instance4f[2] =
	instance4f[3] = float(instance);

	ExecuteContext T_ALIGN16 context;
	context.constants = (void*)i->constants;
	context.instance = (void*)instance4f;
	context.targetSize = (void*)&targetSize;
	context.fragmentPosition = (void*)&fragmentPosition;
	context.uniforms = (void*)inUniforms;
	context.varyings = (void*)inVaryings;
	context.samplers = (void*)inSamplers;
	context.out = (void*)outVaryings;

	__asm
	{
		lea REGISTERS, registers
		lea CONTEXT, context
		call dword ptr [native]
	}

	return true;
}

	}
}
