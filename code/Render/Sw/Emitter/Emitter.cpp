#pragma optimize( "", off )

#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/Shader/Nodes.h"
#include "Render/Sw/Core/Types.h"
#include "Render/Sw/VaryingUtils.h"
#include "Render/Sw/Emitter/Emitter.h"
#include "Render/Sw/Emitter/EmitterContext.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#define SWIZZLE_MASK(x, y, z, w) ( ((x) << 6) | ((y) << 4) | ((z) << 2) | (w) )

Variable* expandTypes(EmitterContext& cx, Variable* in, VariableType intoType)
{
	T_ASSERT (in->type != VtFloat4x4);
	T_ASSERT (intoType != VtFloat4x4);
	if (in->type < intoType)
	{
		if (in->type == VtFloat)
		{
			// Splat single scalars.
			Variable* tmp = cx.allocTemporary(intoType);

			Instruction inst(OpSwizzle, tmp->reg, SWIZZLE_MASK(0, 0, 0, 0), in->reg, 0, 0);
			cx.emitInstruction(inst);

			return tmp;
		}
		else
		{
			int count = int(intoType) - int(in->type);
			int mask = 0;
			
			while (count > 0)
			{
				mask = (mask >> 1) | 8;
				count--;
			}

			Variable* tmp = cx.allocTemporary(intoType);

			Instruction inst(OpExpandWithZero, tmp->reg, in->reg, mask, 0, 0);
			cx.emitInstruction(inst);

			return tmp;
		}
	}
	return in;
}

void collapseTypes(EmitterContext& cx, Variable* in, Variable*& xin)
{
	if (in != xin)
		cx.freeTemporary(xin);
}

void emitAbs(EmitterContext& cx, Abs* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpAbs, out, in);
}

void emitAdd(EmitterContext& cx, Add* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	Variable* xin1 = expandTypes(cx, in1, out->type);
	Variable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpAdd, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitArcusCos(EmitterContext& cx, ArcusCos* node)
{
	Variable* theta = cx.emitInput(node, L"Theta");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpAcos, out, theta);
}

void emitArcusTan(EmitterContext& cx, ArcusTan* node)
{
	Variable* xy = cx.emitInput(node, L"XY");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpAtan, out, xy);
}

void emitClamp(EmitterContext& cx, Clamp* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	Variable* min = cx.emitConstant(node->getMin());
	Variable* max = cx.emitConstant(node->getMax());
	cx.emitInstruction(OpClamp, out, in, min, max);
}

void emitColor(EmitterContext& cx, Color* node)
{
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Color4ub color = node->getColor();
	Variable* in = cx.emitConstant(Vector4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f));
	cx.emitInstruction(OpFetchConstant, out, in);
}

void emitConditional(EmitterContext& cx, Conditional* node)
{
	// Find common input pins from both sides of branch;
	// emit those before condition in order to have them evaluated outside of conditional.
	std::vector< const InputPin* > inputPins;
	cx.findCommonInputs(node, L"CaseTrue", L"CaseFalse", inputPins);
	for (std::vector< const InputPin* >::const_iterator i = inputPins.begin(); i != inputPins.end(); ++i)
		cx.emitInput(*i);

	Variable* in = cx.emitInput(node, L"Input");
	Variable* ref = cx.emitInput(node, L"Reference");
	T_ASSERT (in->type == ref->type);

	Variable* out = cx.emitOutput(node, L"Output", in->type);

	Variable* tmp = cx.allocTemporary(in->type);

	switch (node->getOperator())
	{
	case Conditional::CoLess:
		cx.emitInstruction(OpCompareGreaterEqual, tmp, ref, in);
		break;

	case Conditional::CoLessEqual:
		cx.emitInstruction(OpCompareGreater, tmp, ref, in);
		break;

	case Conditional::CoEqual:
		cx.emitInstruction(OpCompareEqual, tmp, in, ref);
		break;

	case Conditional::CoNotEqual:
		cx.emitInstruction(OpCompareNotEqual, tmp, in, ref);
		break;

	case Conditional::CoGreater:
		cx.emitInstruction(OpCompareGreater, tmp, in, ref);
		break;

	case Conditional::CoGreaterEqual:
		cx.emitInstruction(OpCompareGreaterEqual, tmp, in, ref);
		break;
	}

	Instruction is1(OpJumpIfZero, tmp->reg, 0, 0, 0, 0);
	uint32_t offsetJumpFalse = cx.emitInstruction(is1);

	cx.freeTemporary(tmp);

	uint32_t offsetTrueBegin = cx.getCurrentAddress();
	{
		Variable* ct = cx.emitInput(node, L"CaseTrue");
		T_ASSERT (ct);

		cx.emitInstruction(OpMove, out, ct);
	}

	Instruction is2(OpJump, 0, 0, 0, 0, 0);
	uint32_t offsetJump = cx.emitInstruction(is2);

	uint32_t offsetFalseBegin = cx.getCurrentAddress();
	{
		Variable* cf = cx.emitInput(node, L"CaseFalse");
		T_ASSERT (cf);

		cx.emitInstruction(OpMove, out, cf);
	}

	uint32_t offsetEnd = cx.getCurrentAddress();

	is1.offset = getRelativeOffset(offsetTrueBegin, offsetFalseBegin);
	cx.emitInstruction(offsetJumpFalse, is1);

	is2.offset = getRelativeOffset(offsetFalseBegin, offsetEnd);
	cx.emitInstruction(offsetJump, is2);
}

void emitCos(EmitterContext& cx, Cos* node)
{
	Variable* theta = cx.emitInput(node, L"Theta");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpCos, out, theta);
}

void emitCross(EmitterContext& cx, Cross* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat3);

	Variable* xin1 = expandTypes(cx, in1, VtFloat3);
	Variable* xin2 = expandTypes(cx, in2, VtFloat3);

	cx.emitInstruction(OpCross, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitDerivative(EmitterContext& cx, Derivative* node)
{
	// @fixme
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	Instruction is0(OpSet, out->reg, 0, 1|2|4|8, 0, 0);
	cx.emitInstruction(is0);
}

void emitDiscard(EmitterContext& cx, Discard* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* ref = cx.emitInput(node, L"Reference");

	Variable* tmp = cx.allocTemporary(in->type);

	switch (node->getOperator())
	{
	case Conditional::CoLess:
		cx.emitInstruction(OpCompareGreaterEqual, tmp, in, ref);
		break;

	case Conditional::CoLessEqual:
		cx.emitInstruction(OpCompareGreater, tmp, in, ref);
		break;

	case Conditional::CoEqual:
		cx.emitInstruction(OpCompareNotEqual, tmp, in, ref);
		break;

	case Conditional::CoNotEqual:
		cx.emitInstruction(OpCompareEqual, tmp, in, ref);
		break;

	case Conditional::CoGreater:
		cx.emitInstruction(OpCompareGreaterEqual, tmp, ref, in);
		break;

	case Conditional::CoGreaterEqual:
		cx.emitInstruction(OpCompareGreater, tmp, ref, in);
		break;
	}

	Instruction is(OpJumpIfZero, tmp->reg, 0, 0, 0, 0);
	uint32_t offsetJump = cx.emitInstruction(is);

	cx.freeTemporary(tmp);

	cx.emitInstruction(OpDiscard);

	is.offset = getRelativeOffset(offsetJump, cx.getCurrentAddress());
	cx.emitInstruction(offsetJump, is);

	Variable* pass = cx.emitInput(node, L"Pass");
	Variable* out = cx.emitOutput(node, L"Output", pass->type);
	cx.emitInstruction(OpMove, out, pass);
}

void emitDiv(EmitterContext& cx, Div* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	Variable* xin1 = expandTypes(cx, in1, out->type);
	Variable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpDiv, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitDot(EmitterContext& cx, Dot* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	
	VariableType type = std::max(in1->type, in2->type);
	
	Variable* xin1 = expandTypes(cx, in1, type);
	Variable* xin2 = expandTypes(cx, in2, type);

	if (type > VtFloat3)
		cx.emitInstruction(OpDot4, out, xin1, xin2);
	else
		cx.emitInstruction(OpDot3, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitExp(EmitterContext& cx, Exp* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpExp, out, in);
}

void emitFraction(EmitterContext& cx, Fraction* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpFraction, out, in);
}

void emitFragmentPosition(EmitterContext& cx, FragmentPosition* node)
{
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	cx.emitInstruction(OpFetchFragmentPosition, out);
}

void emitInterpolator(EmitterContext& cx, Interpolator* node)
{
	if (!cx.inPixel())
	{
		Variable* in = cx.emitInput(node, L"Input");
		Variable* out = cx.emitOutput(node, L"Output", in->type);
		cx.emitInstruction(OpMove, out, in);
		return;
	}

	uint32_t allocatorIndex = cx.allocInterpolator();
	T_ASSERT (allocatorIndex < 8);	// Ensure custom interpolators don't exceed max indices.

	uint32_t varyingOffset = getVaryingOffset(DuCustom, allocatorIndex);
	T_ASSERT (varyingOffset != 0);

	cx.enterVertex();
	Variable* in = cx.emitInput(node, L"Input");
	Variable* vo = cx.emitVarying(varyingOffset);
	cx.emitInstruction(OpStoreVarying, vo, in);

	cx.enterPixel();
	Variable* pi = cx.emitVarying(varyingOffset);
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpFetchVarying, out, pi);
}

void emitInstance(EmitterContext& cx, Instance* node)
{
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpFetchInstance, out);
}

void emitIterate(EmitterContext& cx, Iterate* node)
{
	int from = node->getFrom();
	int to = node->getTo();
	int count = to - from;
	if (count < 0 || count >= 256)
		log::error << L"Too many iterations in Iterate node" << Endl;

	// Setup counter, load with initial value.
	Variable* N = cx.emitOutput(node, L"N", VtFloat, true);
	cx.emitInstruction(OpFetchConstant, N, cx.emitConstant(float(from)));

	Variable* tmp1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpFetchConstant, tmp1, cx.emitConstant(float(to)));

	// Initialize output accumulator.
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Variable* initial = cx.emitInput(node, L"Initial");
	if (initial)
	{
		Instruction is0(OpMove, out->reg, initial->reg, 0, 0, 0);
		cx.emitInstruction(is0);
	}
	else
	{
		Instruction is0(OpSet, out->reg, 0, 0xff, 0, 0);
		cx.emitInstruction(is0);
	}

	uint32_t address = cx.getCurrentAddress();

	Variable* in = cx.emitInput(node, L"Input");
	Variable* xin = expandTypes(cx, in, VtFloat4);
	cx.emitInstruction(OpMove, out, xin);
	collapseTypes(cx, in, xin);

	// Check condition.
	Variable* cond = cx.emitInput(node, L"Condition");
	if (cond)
	{
		// \fixme
	}

	// Increment counter, repeat loop if not at target.
	cx.emitInstruction(OpIncrement, N);

	Variable* tmp2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpCompareGreater, tmp2, N, tmp1);

	Instruction inst(OpJumpIfZero, tmp2->reg, getRelativeOffset(cx.getCurrentAddress(), address));
	cx.emitInstruction(inst);

	cx.freeTemporary(tmp2);
	cx.freeTemporary(tmp1);
}

void emitIterate2d(EmitterContext& cx, Iterate2d* node)
{
	// \fixme
}

void emitIndexedUniform(EmitterContext& cx, IndexedUniform* node)
{
	VariableType variableType;
	
	switch (node->getParameterType())
	{
	case PtScalar:
		variableType = VtFloat;
		break;

	case PtVector:
		variableType = VtFloat4;
		break;

	case PtMatrix:
		variableType = VtFloat4x4;
		break;

	default:
		T_BREAKPOINT;
	}

	Variable* index = cx.emitInput(node, L"Index");
	Variable* in = cx.emitUniform(node->getParameterName(), variableType, node->getLength());
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpFetchIndexedUniform, out, in, index);
}

void emitLength(EmitterContext& cx, Length* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpLength, out, in);
}

void emitLerp(EmitterContext& cx, Lerp* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* blend = cx.emitInput(node, L"Blend");
	Variable* out = cx.emitOutput(node, L"Output", in1->type);
	cx.emitInstruction(OpLerp, out, blend, in1, in2);
}

void emitLog(EmitterContext& cx, Log* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	switch (node->getBase())
	{
	case Log::LbTwo:
		cx.emitInstruction(OpLog2, out, in);
		break;
	case Log::LbTen:
		cx.emitInstruction(OpLog10, out, in);
		break;
	case Log::LbNatural:
		cx.emitInstruction(OpLog, out, in);
		break;
	}
}

void emitMatrixIn(EmitterContext& cx, MatrixIn* node)
{
	Variable* axisX = cx.emitInput(node, L"XAxis");
	Variable* axisY = cx.emitInput(node, L"YAxis");
	Variable* axisZ = cx.emitInput(node, L"ZAxis");
	Variable* translate = cx.emitInput(node, L"Translate");
	Variable* tmp = cx.allocTemporary(VtFloat4x4);
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4x4);

	if (axisX)
	{
		Variable* xaxisX = expandTypes(cx, axisX, VtFloat4);
		Instruction is0(OpMove, tmp->reg + 0, xaxisX->reg, 0, 0, 0);
		cx.emitInstruction(is0);
		collapseTypes(cx, axisX, xaxisX);
	}
	// else set 1,0,0,0
	else
	{
		// xyzw 1,2,4,8
		Instruction is0(OpSet, tmp->reg + 0, 1, 2|4|8, 0, 0);
		cx.emitInstruction(is0);
	}

	if (axisY)
	{
		Variable* xaxisY = expandTypes(cx, axisY, VtFloat4);
		Instruction is1(OpMove, tmp->reg + 1, xaxisY->reg, 0, 0, 0);
		cx.emitInstruction(is1);
		collapseTypes(cx, axisY, xaxisY);
	}
	// else set 0,1,0,0
	else
	{
		// xyzw 1,2,4,8
		Instruction is1(OpSet, tmp->reg + 1, 2, 1|4|8, 0, 0);
		cx.emitInstruction(is1);
	}

	if (axisZ)
	{
		Variable* xaxisZ = expandTypes(cx, axisZ, VtFloat4);
		Instruction is2(OpMove, tmp->reg + 2, xaxisZ->reg, 0, 0, 0);
		cx.emitInstruction(is2);
		collapseTypes(cx, axisZ, xaxisZ);
	}
	// else set 0,0,1,0
	else
	{
		// xyzw 1,2,4,8
		Instruction is2(OpSet, tmp->reg + 2, 4, 1|2|8, 0, 0);
		cx.emitInstruction(is2);
	}

	if (translate)
	{
		Variable* xtranslate = expandTypes(cx, translate, VtFloat4);
		Instruction is3(OpMove, tmp->reg + 3, xtranslate->reg, 0, 0, 0);
		cx.emitInstruction(is3);
		collapseTypes(cx, translate, xtranslate);
	}
	// else set 0,0,0,1
	else
	{
		// xyzw 1,2,4,8
		Instruction is3(OpSet, tmp->reg + 3, 8, 1|2|4, 0, 0);
		cx.emitInstruction(is3);
	}

	// Matrices are column major thus we need to transpose it to become valid.
	cx.emitInstruction(OpTranspose, out, tmp);
	cx.freeTemporary(tmp);
}

void emitMatrixOut(EmitterContext& cx, MatrixOut* node)
{
	Variable* in = cx.emitInput(node, L"Input");

	Variable* axisX = cx.emitOutput(node, L"XAxis", VtFloat4);
	Variable* axisY = cx.emitOutput(node, L"YAxis", VtFloat4);
	Variable* axisZ = cx.emitOutput(node, L"ZAxis", VtFloat4);
	Variable* translate = cx.emitOutput(node, L"Translate", VtFloat4);

	if (axisX)
	{
		Instruction is(OpMove, axisX->reg, in->reg, 0, 0, 0);
		cx.emitInstruction(is);
	}

	if (axisY)
	{
		Instruction is(OpMove, axisY->reg, in->reg + 1, 0, 0, 0);
		cx.emitInstruction(is);
	}

	if (axisZ)
	{
		Instruction is(OpMove, axisZ->reg, in->reg + 2, 0, 0, 0);
		cx.emitInstruction(is);
	}

	if (translate)
	{
		Instruction is(OpMove, translate->reg, in->reg + 3, 0, 0, 0);
		cx.emitInstruction(is);
	}
}

void emitMax(EmitterContext& cx, Max* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	Variable* xin1 = expandTypes(cx, in1, out->type);
	Variable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpMax, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitMin(EmitterContext& cx, Min* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	Variable* xin1 = expandTypes(cx, in1, out->type);
	Variable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpMin, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitMixIn(EmitterContext& cx, MixIn* node)
{
	Variable* x = cx.emitInput(node, L"X");
	Variable* y = cx.emitInput(node, L"Y");
	Variable* z = cx.emitInput(node, L"Z");
	Variable* w = cx.emitInput(node, L"W");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	cx.emitInstruction(OpMixIn, out, x, y, z, w);

	uint8_t s0 = 0x00;

	if (!x)
		s0 |= 1;
	if (!y)
		s0 |= 2;
	if (!z)
		s0 |= 4;
	if (!w)
		s0 |= 8;

	if (s0)
	{
		Instruction is0(OpSet, out->reg, 0, s0, 0, 0);
		cx.emitInstruction(is0); 
	}
}

void emitMixOut(EmitterContext& cx, MixOut* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* x = cx.emitOutput(node, L"X", VtFloat);
	Variable* y = cx.emitOutput(node, L"Y", VtFloat);
	Variable* z = cx.emitOutput(node, L"Z", VtFloat);
	Variable* w = cx.emitOutput(node, L"W", VtFloat);
	if (x)
	{
		Instruction inst(OpSwizzle, x->reg, SWIZZLE_MASK(0, 0, 0, 0), in->reg, 0, 0);
		cx.emitInstruction(inst);
	}
	if (y)
	{
		Instruction inst(OpSwizzle, y->reg, SWIZZLE_MASK(1, 1, 1, 1), in->reg, 0, 0);
		cx.emitInstruction(inst);
	}
	if (z)
	{
		Instruction inst(OpSwizzle, z->reg, SWIZZLE_MASK(2, 2, 2, 2), in->reg, 0, 0);
		cx.emitInstruction(inst);
	}
	if (w)
	{
		Instruction inst(OpSwizzle, w->reg, SWIZZLE_MASK(3, 3, 3, 3), in->reg, 0, 0);
		cx.emitInstruction(inst);
	}
}

void emitMul(EmitterContext& cx, Mul* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));
	
	Variable* xin1 = expandTypes(cx, in1, out->type);
	Variable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpMul, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitMulAdd(EmitterContext& cx, MulAdd* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* in3 = cx.emitInput(node, L"Input3");
	
	VariableType type = std::max(std::max(in1->type, in2->type), in3->type);
	Variable* out = cx.emitOutput(node, L"Output", type);

	Variable* xin1 = expandTypes(cx, in1, out->type);
	Variable* xin2 = expandTypes(cx, in2, out->type);
	Variable* xin3 = expandTypes(cx, in3, out->type);

	cx.emitInstruction(OpMulAdd, out, xin1, xin2, xin3);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
	collapseTypes(cx, in3, xin3);
}

void emitNeg(EmitterContext& cx, Neg* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpNeg, out, in);
}

void emitNormalize(EmitterContext& cx, Normalize* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpNormalize, out, in);
}

void emitPixelOutput(EmitterContext& cx, PixelOutput* node)
{
	cx.enterPixel();

	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitVarying(0);
	cx.emitInstruction(OpStoreVarying, out, in);

	RenderState rs = node->getRenderState();

	RenderStateDesc state;
	state.depthEnable = rs.depthEnable;
	state.depthWriteEnable = rs.depthWriteEnable;
	state.cullMode = rs.cullMode;
	state.blendEnable = rs.blendEnable;
	state.blendOperation = rs.blendOperation;
	state.blendSource = rs.blendSource;
	state.blendDestination = rs.blendDestination;
	cx.setRenderState(state);
}

void emitReflect(EmitterContext& cx, Reflect* node)
{
	Variable* normal = cx.emitInput(node, L"Normal");
	Variable* direction = cx.emitInput(node, L"Direction");
	Variable* out = cx.emitOutput(node, L"Output", direction->type);

	Variable* tmp1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpDot3, tmp1, normal, direction);

	Variable* two = cx.emitConstant(2.0f);
	Variable* tmp2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpMul, tmp2, tmp1, two);

	Variable* xtmp2 = expandTypes(cx, tmp2, out->type);

	Variable* tmp3 = cx.allocTemporary(out->type);
	cx.emitInstruction(OpMul, tmp3, normal, xtmp2);

	collapseTypes(cx, tmp2, xtmp2);

	cx.emitInstruction(OpSub, out, tmp3, direction);
}

void emitRecipSqrt(EmitterContext& cx, RecipSqrt* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpRecipSqrt, out, in);
}

void emitRound(EmitterContext& cx, Round* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpRound, out, in);
}

void emitPow(EmitterContext& cx, Pow* node)
{
	float exponent = 0.0f;
	if (cx.evaluateConstant(node, L"Exponent", exponent))
	{
		Variable* in = cx.emitInput(node, L"Input");
		Variable* out = cx.emitOutput(node, L"Output", in->type);
		switch (int(exponent))
		{
		case 0:
			{
				Instruction is0(OpSet, out->reg, 1|2|4|8, 0, 0, 0);
				cx.emitInstruction(is0);
			}
			break;
		case 1:
			cx.emitInstruction(OpMove, out, in);
			break;
		case 2:
			cx.emitInstruction(OpMove, out, in);
			cx.emitInstruction(OpMul, out, out, in);
			break;
		case 3:
			cx.emitInstruction(OpMove, out, in);
			cx.emitInstruction(OpMul, out, out, in);
			cx.emitInstruction(OpMul, out, out, in);
			break;
		case 4:
			cx.emitInstruction(OpMove, out, in);
			cx.emitInstruction(OpMul, out, out, in);
			cx.emitInstruction(OpMul, out, out, in);
			cx.emitInstruction(OpMul, out, out, in);
			break;
		default:
			{
				Variable* exp = cx.emitConstant(exponent);
				cx.emitInstruction(OpPow, out, in, exp);
			}
			break;
		}
	}
	else
	{
		Variable* exp = cx.emitInput(node, L"Exponent");
		Variable* in = cx.emitInput(node, L"Input");
		Variable* out = cx.emitOutput(node, L"Output", std::max(in->type, exp->type));
		cx.emitInstruction(OpPow, out, in, exp);
	}
}

void emitSampler(EmitterContext& cx, Sampler* node)
{
	Variable* texture = cx.emitInput(node, L"Texture");
	Variable* texCoord = cx.emitInput(node, L"TexCoord");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Instruction inst(OpSampler, out->reg, texCoord->reg, texture->reg, 0, 0);
	cx.emitInstruction(inst);
}

void emitScalar(EmitterContext& cx, Scalar* node)
{
	Variable* in = cx.emitConstant(node->get());
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpFetchConstant, out, in);
}

void emitSign(EmitterContext& cx, Sign* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpSign, out, in);
}

void emitSin(EmitterContext& cx, Sin* node)
{
	Variable* theta = cx.emitInput(node, L"Theta");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpSin, out, theta);
}

void emitSqrt(EmitterContext& cx, Sqrt* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpSqrt, out, in);
}

void emitSub(EmitterContext& cx, Sub* node)
{
	Variable* in1 = cx.emitInput(node, L"Input1");
	Variable* in2 = cx.emitInput(node, L"Input2");
	Variable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	Variable* xin1 = expandTypes(cx, in1, out->type);
	Variable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpSub, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
}

void emitSum(EmitterContext& cx, Sum* node)
{
	int from = node->getFrom();
	int to = node->getTo();
	int count = to - from;
	if (count < 0 || count >= 256)
		log::error << L"Too many iterations in Sum node" << Endl;

	// Setup counter, load with initial value.
	Variable* N = cx.emitOutput(node, L"N", VtFloat, true);
	cx.emitInstruction(OpFetchConstant, N, cx.emitConstant(float(from)));

	Variable* tmp1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpFetchConstant, tmp1, cx.emitConstant(float(to)));
	
	// Initialize output accumulator.
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Instruction is0(OpSet, out->reg, 0, 0xff, 0, 0);
	cx.emitInstruction(is0);

	uint32_t address = cx.getCurrentAddress();

	Variable* in = cx.emitInput(node, L"Input");
	Variable* xin = expandTypes(cx, in, VtFloat4);

	cx.emitInstruction(OpAdd, out, out, xin);

	collapseTypes(cx, in, xin);
	
	// Increment counter, repeat loop if not at target.
	cx.emitInstruction(OpIncrement, N);

	Variable* tmp2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpCompareGreater, tmp2, N, tmp1);

	Instruction inst(OpJumpIfZero, tmp2->reg, getRelativeOffset(cx.getCurrentAddress(), address));
	cx.emitInstruction(inst);

	cx.freeTemporary(tmp2);
	cx.freeTemporary(tmp1);
}

void emitSwitch(EmitterContext& cx, Switch* node)
{
	Variable* select = cx.emitInput(node, L"Select");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);

	Variable* N = cx.allocTemporary(VtFloat);

	std::vector< uint32_t > addressBreaks;

	const std::vector< int32_t >& cases = node->getCases();
	for (uint32_t i = 0; i < uint32_t(cases.size()); ++i)
	{
		cx.emitInstruction(OpFetchConstant, N, cx.emitConstant(float(cases[i])));
		cx.emitInstruction(OpCompareEqual, N, select, N);

		Instruction is1(OpJumpIfZero, N->reg, 0, 0, 0, 0);
		uint32_t addressJump = cx.emitInstruction(is1);

		const InputPin* caseInputPin = node->getInputPin(i + 2);
		T_ASSERT (caseInputPin);

		Variable* in = cx.emitInput(caseInputPin);
		T_ASSERT (in);

		Variable* xin = expandTypes(cx, in, VtFloat4);
		cx.emitInstruction(OpMove, out, xin);
		collapseTypes(cx, in, xin);

		Instruction is2(OpJump, 0, 0, 0, 0, 0);
		addressBreaks.push_back(cx.emitInstruction(is2));

		is1.offset = getRelativeOffset(addressJump, cx.getCurrentAddress());
		cx.emitInstruction(addressJump, is1);
	}

	cx.freeTemporary(N);

	// Default
	Variable* in = cx.emitInput(node, L"Default");
	T_ASSERT (in);

	Variable* xin = expandTypes(cx, in, VtFloat4);
	cx.emitInstruction(OpMove, out, xin);
	collapseTypes(cx, in, xin);

	// Patch break instructions.
	uint32_t addressEnd = cx.getCurrentAddress();
	for (std::vector< uint32_t >::iterator i = addressBreaks.begin(); i != addressBreaks.end(); ++i)
	{
		Instruction inst(OpJump, 0, getRelativeOffset(*i, addressEnd));
		cx.emitInstruction(*i, inst);
	}
}

void emitSwizzle(EmitterContext& cx, Swizzle* node)
{
	std::wstring map = node->get();
	T_ASSERT (map.length() > 0);

	const VariableType types[] = { VtFloat, VtFloat2, VtFloat3, VtFloat4 };
	VariableType type = types[map.length() - 1];

	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", type);

	uint8_t s0 = 0x00, s1 = 0x00;
	uint8_t ch[4] = { 0, 0, 0, 0 };

	for (size_t i = 0; i < map.length() ; ++i)
	{
		switch (tolower(map[i]))
		{
		case 'x':
			ch[i] = 0;
			break;
		case 'y':
			ch[i] = 1;
			break;
		case 'z':
			ch[i] = 2;
			break;
		case 'w':
			ch[i] = 3;
			break;
		case '0':
			s0 |= (1 << i);
			break;
		case '1':
			s1 |= (1 << i);
			break;
		}
	}

	Instruction inst(OpSwizzle, out->reg, SWIZZLE_MASK(ch[0], ch[1], ch[2], ch[3]), in->reg, 0, 0);
	cx.emitInstruction(inst);

	if (s0 || s1)
	{
		Instruction is0(OpSet, out->reg, s1, s0, 0, 0);
		cx.emitInstruction(is0); 
	}
}

void emitTargetSize(EmitterContext& cx, TargetSize* node)
{
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	cx.emitInstruction(OpFetchTargetSize, out);
}

void emitTan(EmitterContext& cx, Tan* node)
{
	Variable* theta = cx.emitInput(node, L"Theta");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpTan, out, theta);
}

void emitTextureSize(EmitterContext& cx, TextureSize* node)
{
	Variable* texture = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Instruction inst(OpFetchTextureSize, out->reg, texture->reg, 0, 0, 0);
	cx.emitInstruction(inst);
}

void emitTransform(EmitterContext& cx, Transform* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* transform = cx.emitInput(node, L"Transform");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpTransform, out, in, transform);
}

void emitTranspose(EmitterContext& cx, Transpose* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpTranspose, out, in);
}

void emitTruncate(EmitterContext& cx, Truncate* node)
{
	Variable* in = cx.emitInput(node, L"Input");
	Variable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpTrunc, out, in);
}

void emitUniform(EmitterContext& cx, Uniform* node)
{
	VariableType variableType;
	
	switch (node->getParameterType())
	{
	case PtScalar:
		variableType = VtFloat;
		break;

	case PtVector:
		variableType = VtFloat4;
		break;

	case PtMatrix:
		variableType = VtFloat4x4;
		break;

	case PtTexture2D:
	case PtTexture3D:
	case PtTextureCube:
		variableType = VtTexture;
		break;
	}

	if (variableType == VtTexture)
	{
		Variable* out = cx.emitOutput(node, L"Output", VtTexture);
		out->reg = cx.allocSampler(node->getParameterName());
	}
	else
	{
		Variable* in = cx.emitUniform(node->getParameterName(), variableType);
		Variable* out = cx.emitOutput(node, L"Output", in->type);
		cx.emitInstruction(OpFetchUniform, out, in);
	}
}

void emitVector(EmitterContext& cx, Vector* node)
{
	Variable* in = cx.emitConstant(node->get());
	Variable* out = cx.emitOutput(node, L"Output", VtFloat4);
	cx.emitInstruction(OpFetchConstant, out, in);
}

void emitVertexInput(EmitterContext& cx, VertexInput* node)
{
	T_ASSERT_M (cx.inVertex(), L"VertexInput node in wrong scope");

	VariableType type;
	switch (node->getDataType())
	{
	case DtFloat1:
		type = VtFloat;
		break;

	case DtFloat2:
	case DtShort2:
	case DtShort2N:
	case DtHalf2:
		type = VtFloat2;
		break;

	case DtFloat3:
		type = VtFloat3;
		break;

	case DtFloat4:
	case DtByte4:
	case DtByte4N:
	case DtShort4:
	case DtShort4N:
	case DtHalf4:
		type = VtFloat4;
		break;

	default:
		log::error << L"Unknown data type" << Endl;
		T_ASSERT (0);
	}

	uint32_t varyingOffset = getVaryingOffset(node->getDataUsage(), node->getIndex());

	Variable* in = cx.emitVarying(varyingOffset);
	Variable* out = cx.emitOutput(node, L"Output", type);
	cx.emitInstruction(OpFetchVarying, out, in);

	if (node->getDataUsage() == DuPosition && type != VtFloat4)
	{
		Instruction is1(OpSet, out->reg, 8, 0, 0, 0);
		cx.emitInstruction(is1);
		out->type = VtFloat4;
	}
}

void emitVertexOutput(EmitterContext& cx, VertexOutput* node)
{
	cx.enterVertex();
	
	Variable* in = cx.emitInput(node, L"Input");
	if (in->type != VtFloat4)
	{
		Variable* tmp = cx.allocTemporary(VtFloat4);
		cx.emitInstruction(OpMove, tmp, in);

		switch (in->type)
		{
		case VtFloat:
			{
				Instruction is0(OpSet, tmp->reg, 0, (2 | 4 | 8), 0, 0);
				cx.emitInstruction(is0);
			}
			break;
		case VtFloat2:
			{
				Instruction is0(OpSet, tmp->reg, 0, (4 | 8), 0, 0);
				cx.emitInstruction(is0);
			}
			break;
		case VtFloat3:
			{
				Instruction is0(OpSet, tmp->reg, 0, (8), 0, 0);
				cx.emitInstruction(is0);
			}
			break;
		}

		Variable* out = cx.emitVarying(0);
		cx.emitInstruction(OpStoreVarying, out, tmp);
	}
	else
	{
		Variable* out = cx.emitVarying(0);
		cx.emitInstruction(OpStoreVarying, out, in);
	}
}

struct EmitterImpl
{
	virtual void emit(EmitterContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public EmitterImpl
{
	typedef void (*function_t)(EmitterContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function) :
		m_function(function)
	{
	}

	virtual void emit(EmitterContext& c, Node* node)
	{
		T_ASSERT (is_a< NodeType >(node));
		(*m_function)(c, static_cast< NodeType* >(node));
	}
};

		}

Emitter::Emitter()
{
	m_emitters[&type_of< Abs >()] = new EmitterCast< Abs >(emitAbs);
	m_emitters[&type_of< Add >()] = new EmitterCast< Add >(emitAdd);
	m_emitters[&type_of< ArcusCos >()] = new EmitterCast< ArcusCos >(emitArcusCos);
	m_emitters[&type_of< ArcusTan >()] = new EmitterCast< ArcusTan >(emitArcusTan);
	m_emitters[&type_of< Clamp >()] = new EmitterCast< Clamp >(emitClamp);
	m_emitters[&type_of< Color >()] = new EmitterCast< Color >(emitColor);
	m_emitters[&type_of< Conditional >()] = new EmitterCast< Conditional >(emitConditional);
	m_emitters[&type_of< Cos >()] = new EmitterCast< Cos >(emitCos);
	m_emitters[&type_of< Cross >()] = new EmitterCast< Cross >(emitCross);
	m_emitters[&type_of< Derivative >()] = new EmitterCast< Derivative >(emitDerivative);
	m_emitters[&type_of< Discard >()] = new EmitterCast< Discard >(emitDiscard);
	m_emitters[&type_of< Div >()] = new EmitterCast< Div >(emitDiv);
	m_emitters[&type_of< Dot >()] = new EmitterCast< Dot >(emitDot);
	m_emitters[&type_of< Exp >()] = new EmitterCast< Exp >(emitExp);
	m_emitters[&type_of< Fraction >()] = new EmitterCast< Fraction >(emitFraction);
	m_emitters[&type_of< FragmentPosition >()] = new EmitterCast< FragmentPosition >(emitFragmentPosition);
	m_emitters[&type_of< IndexedUniform >()] = new EmitterCast< IndexedUniform >(emitIndexedUniform);
	m_emitters[&type_of< Interpolator >()] = new EmitterCast< Interpolator >(emitInterpolator);
	m_emitters[&type_of< Instance >()] = new EmitterCast< Instance >(emitInstance);
	m_emitters[&type_of< Iterate >()] = new EmitterCast< Iterate >(emitIterate);
	m_emitters[&type_of< Iterate2d >()] = new EmitterCast< Iterate2d >(emitIterate2d);
	m_emitters[&type_of< Length >()] = new EmitterCast< Length >(emitLength);
	m_emitters[&type_of< Lerp >()] = new EmitterCast< Lerp >(emitLerp);
	m_emitters[&type_of< Log >()] = new EmitterCast< Log >(emitLog);
	m_emitters[&type_of< MatrixIn >()] = new EmitterCast< MatrixIn >(emitMatrixIn);
	m_emitters[&type_of< MatrixOut >()] = new EmitterCast< MatrixOut >(emitMatrixOut);
	m_emitters[&type_of< Max >()] = new EmitterCast< Max >(emitMax);
	m_emitters[&type_of< Min >()] = new EmitterCast< Min >(emitMin);
	m_emitters[&type_of< MixIn >()] = new EmitterCast< MixIn >(emitMixIn);
	m_emitters[&type_of< MixOut >()] = new EmitterCast< MixOut >(emitMixOut);
	m_emitters[&type_of< Mul >()] = new EmitterCast< Mul >(emitMul);
	m_emitters[&type_of< MulAdd >()] = new EmitterCast< MulAdd >(emitMulAdd);
	m_emitters[&type_of< Neg >()] = new EmitterCast< Neg >(emitNeg);
	m_emitters[&type_of< Normalize >()] = new EmitterCast< Normalize >(emitNormalize);
	m_emitters[&type_of< Pow >()] = new EmitterCast< Pow >(emitPow);
	m_emitters[&type_of< PixelOutput >()] = new EmitterCast< PixelOutput >(emitPixelOutput);
	m_emitters[&type_of< Reflect >()] = new EmitterCast< Reflect >(emitReflect);
	m_emitters[&type_of< RecipSqrt >() ] = new EmitterCast< RecipSqrt >(emitRecipSqrt);
	m_emitters[&type_of< Round >()] = new EmitterCast< Round >(emitRound);
	m_emitters[&type_of< Sampler >()] = new EmitterCast< Sampler >(emitSampler);
	m_emitters[&type_of< Scalar >()] = new EmitterCast< Scalar >(emitScalar);
	m_emitters[&type_of< Sign >()] = new EmitterCast< Sign >(emitSign);
	m_emitters[&type_of< Sin >()] = new EmitterCast< Sin >(emitSin);
	m_emitters[&type_of< Sqrt >()] = new EmitterCast< Sqrt >(emitSqrt);
	m_emitters[&type_of< Sub >()] = new EmitterCast< Sub >(emitSub);
	m_emitters[&type_of< Sum >()] = new EmitterCast< Sum >(emitSum);
	m_emitters[&type_of< Switch >()] = new EmitterCast< Switch >(emitSwitch);
	m_emitters[&type_of< Swizzle >()] = new EmitterCast< Swizzle >(emitSwizzle);
	m_emitters[&type_of< TargetSize >()] = new EmitterCast< TargetSize >(emitTargetSize);
	m_emitters[&type_of< Tan >()] = new EmitterCast< Tan >(emitTan);
	m_emitters[&type_of< TextureSize >()] = new EmitterCast< TextureSize >(emitTextureSize);
	m_emitters[&type_of< Transform >()] = new EmitterCast< Transform >(emitTransform);
	m_emitters[&type_of< Transpose >()] = new EmitterCast< Transpose >(emitTranspose);
	m_emitters[&type_of< Truncate >()] = new EmitterCast< Truncate >(emitTruncate);
	m_emitters[&type_of< Uniform >()] = new EmitterCast< Uniform >(emitUniform);
	m_emitters[&type_of< Vector >()] = new EmitterCast< Vector >(emitVector);
	m_emitters[&type_of< VertexInput >()] = new EmitterCast< VertexInput >(emitVertexInput);
	m_emitters[&type_of< VertexOutput >()] = new EmitterCast< VertexOutput >(emitVertexOutput);
}

Emitter::~Emitter()
{
	for (std::map< const TypeInfo*, EmitterImpl* >::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
		delete i->second;
}

void Emitter::emit(EmitterContext& c, Node* node)
{
	std::map< const TypeInfo*, EmitterImpl* >::iterator i = m_emitters.find(&type_of(node));
	T_ASSERT_M (i != m_emitters.end(), L"No emitter for node");
	T_ASSERT (i->second);
	i->second->emit(c, node);
}

	}
}
