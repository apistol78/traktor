/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

EmitterVariable* expandTypes(EmitterContext& cx, EmitterVariable* in, EmitterVariableType intoType)
{
	T_ASSERT (in->type != VtFloat4x4);
	T_ASSERT (intoType != VtFloat4x4);
	if (in->type < intoType)
	{
		if (in->type == VtFloat)
		{
			// Splat single scalars.
			EmitterVariable* tmp = cx.allocTemporary(intoType);

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

			EmitterVariable* tmp = cx.allocTemporary(intoType);

			Instruction inst(OpExpandWithZero, tmp->reg, in->reg, mask, 0, 0);
			cx.emitInstruction(inst);

			return tmp;
		}
	}
	return in;
}

#define EMIT_MANDATORY_IN_PIN(var, cx, inputPin) \
	EmitterVariable* var = cx.emitInput(inputPin); \
	if (!var) \
	{ \
		log::error << L"Failed to emit mandatory input \"" << inputPin->getName() << L"\" of a " << type_name(node) << L" " << node->getId().format() << Endl; \
		Debugger::getInstance().breakDebugger(); \
		return false; \
	}

#define EMIT_MANDATORY_IN(var, cx, node, input) \
	EmitterVariable* var = cx.emitInput(node, input); \
	if (!var) \
	{ \
		log::error << L"Failed to emit mandatory input \"" << input << L"\" of a " << type_name(node) << L" " << node->getId().format() << Endl; \
		Debugger::getInstance().breakDebugger(); \
		return false; \
	}

void collapseTypes(EmitterContext& cx, EmitterVariable* in, EmitterVariable*& xin)
{
	if (in != xin)
		cx.freeTemporary(xin);
}

bool emitAbs(EmitterContext& cx, Abs* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpAbs, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitAdd(EmitterContext& cx, Add* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");

	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	EmitterVariable* xin1 = expandTypes(cx, in1, out->type);
	EmitterVariable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpAdd, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitArcusCos(EmitterContext& cx, ArcusCos* node)
{
	EMIT_MANDATORY_IN(theta, cx, node, L"Theta");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpAcos, out, theta);
	cx.releaseInput(node, L"Theta");
	return true;
}

bool emitArcusTan(EmitterContext& cx, ArcusTan* node)
{
	EMIT_MANDATORY_IN(xy, cx, node, L"XY");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpAtan, out, xy);
	cx.releaseInput(node, L"XY");
	return true;
}

bool emitClamp(EmitterContext& cx, Clamp* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	EmitterVariable* min = cx.emitConstant(node->getMin());
	EmitterVariable* max = cx.emitConstant(node->getMax());
	cx.emitInstruction(OpClamp, out, in, min, max);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitColor(EmitterContext& cx, Color* node)
{
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Color4ub color = node->getColor();
	EmitterVariable* in = cx.emitConstant(Vector4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f));
	cx.emitInstruction(OpFetchConstant, out, in);
	return true;
}

bool emitConditional(EmitterContext& cx, Conditional* node)
{
	//// Find common input pins from both sides of branch;
	//// emit those before condition in order to have them evaluated outside of conditional.
	//std::vector< const InputPin* > inputPins;
	//cx.findCommonInputs(node, L"CaseTrue", L"CaseFalse", inputPins);
	//for (std::vector< const InputPin* >::const_iterator i = inputPins.begin(); i != inputPins.end(); ++i)
	//	cx.emitInput(*i);

	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EMIT_MANDATORY_IN(ref, cx, node, L"Reference");
	T_ASSERT (in->type == ref->type);

	EmitterVariable* tmp = cx.allocTemporary(in->type);

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

	EmitterVariable* ct = cx.emitInput(node, L"CaseTrue");
	T_ASSERT (ct);

	Instruction instMoveTrue(OpMove, 0, ct->reg, 0, 0, 0);
	uint32_t offsetMoveTrue = cx.emitInstruction(instMoveTrue);

	Instruction is2(OpJump, 0, 0, 0, 0, 0);
	uint32_t offsetJump = cx.emitInstruction(is2);

	uint32_t offsetFalseBegin = cx.getCurrentAddress();

	EmitterVariable* cf = cx.emitInput(node, L"CaseFalse");
	T_ASSERT (cf);

	Instruction instMoveFalse(OpMove, 0, cf->reg, 0, 0, 0);
	uint32_t offsetMoveFalse = cx.emitInstruction(instMoveFalse);

	uint32_t offsetEnd = cx.getCurrentAddress();

	EmitterVariable* out = cx.emitOutput(node, L"Output", max(ct->type, cf->type));
	instMoveTrue.dest = out->reg;
	instMoveFalse.dest = out->reg;
	cx.emitInstruction(offsetMoveTrue, instMoveTrue);
	cx.emitInstruction(offsetMoveFalse, instMoveFalse);

	cx.releaseInput(node, L"CaseTrue");
	cx.releaseInput(node, L"CaseFalse");

	is1.offset = getRelativeOffset(offsetTrueBegin, offsetFalseBegin);
	cx.emitInstruction(offsetJumpFalse, is1);

	is2.offset = getRelativeOffset(offsetFalseBegin, offsetEnd);
	cx.emitInstruction(offsetJump, is2);

	cx.releaseInput(node, L"Input");
	cx.releaseInput(node, L"Reference");

	return true;
}

bool emitCos(EmitterContext& cx, Cos* node)
{
	EMIT_MANDATORY_IN(theta, cx, node, L"Theta");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpCos, out, theta);
	cx.releaseInput(node, L"Theta");
	return true;
}

bool emitCross(EmitterContext& cx, Cross* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");

	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat3);

	EmitterVariable* xin1 = expandTypes(cx, in1, VtFloat3);
	EmitterVariable* xin2 = expandTypes(cx, in2, VtFloat3);

	cx.emitInstruction(OpCross, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitDerivative(EmitterContext& cx, Derivative* node)
{
	// @fixme
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	Instruction is0(OpSet, out->reg, 0, 1|2|4|8, 0, 0);
	cx.emitInstruction(is0);
	return true;
}

bool emitDiscard(EmitterContext& cx, Discard* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EMIT_MANDATORY_IN(ref, cx, node, L"Reference");

	EmitterVariable* tmp = cx.allocTemporary(in->type);

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

	EmitterVariable* pass = cx.emitInput(node, L"Pass");
	EmitterVariable* out = cx.emitOutput(node, L"Output", pass->type);
	cx.emitInstruction(OpMove, out, pass);

	cx.releaseInput(node, L"Pass");
	cx.releaseInput(node, L"Reference");
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitDiv(EmitterContext& cx, Div* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");

	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	EmitterVariable* xin1 = expandTypes(cx, in1, out->type);
	EmitterVariable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpDiv, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitDot(EmitterContext& cx, Dot* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");

	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	
	EmitterVariableType type = std::max(in1->type, in2->type);
	
	EmitterVariable* xin1 = expandTypes(cx, in1, type);
	EmitterVariable* xin2 = expandTypes(cx, in2, type);

	if (type > VtFloat3)
		cx.emitInstruction(OpDot4, out, xin1, xin2);
	else
	{
		switch (in1->type)
		{
		case VtFloat:
			{
				Instruction is0(OpSet, xin1->reg, 0, 2|4|8, 0, 0);
				cx.emitInstruction(is0);
			}
			break;

		case VtFloat2:
			{
				Instruction is0(OpSet, xin1->reg, 0, 4|8, 0, 0);
				cx.emitInstruction(is0);
			}
			break;
		}

		switch (in2->type)
		{
		case VtFloat:
			{
				Instruction is0(OpSet, xin2->reg, 0, 2|4|8, 0, 0);
				cx.emitInstruction(is0);
			}
			break;

		case VtFloat2:
			{
				Instruction is0(OpSet, xin2->reg, 0, 4|8, 0, 0);
				cx.emitInstruction(is0);
			}
			break;
		}

		cx.emitInstruction(OpDot3, out, xin1, xin2);
	}

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitExp(EmitterContext& cx, Exp* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpExp, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitFraction(EmitterContext& cx, Fraction* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpFraction, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitFragmentPosition(EmitterContext& cx, FragmentPosition* node)
{
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	cx.emitInstruction(OpFetchFragmentPosition, out);
	return true;
}

bool emitInterpolator(EmitterContext& cx, Interpolator* node)
{
	if (!cx.inPixel())
	{
		EMIT_MANDATORY_IN(in, cx, node, L"Input");
		EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
		cx.emitInstruction(OpMove, out, in);
		cx.releaseInput(node, L"Input");
		return true;
	}

	uint32_t allocatorIndex = cx.allocInterpolator();
	T_ASSERT (allocatorIndex < 8);	// Ensure custom interpolators don't exceed max indices.

	uint32_t varyingOffset = getVaryingOffset(DuCustom, allocatorIndex);
	T_ASSERT (varyingOffset != 0);

	cx.enterVertex();
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* vo = cx.emitVarying(varyingOffset);
	cx.emitInstruction(OpStoreVarying, vo, in);
	cx.releaseInput(node, L"Input");

	cx.enterPixel();
	EmitterVariable* pi = cx.emitVarying(varyingOffset);
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpFetchVarying, out, pi);
	return true;
}

bool emitInstance(EmitterContext& cx, Instance* node)
{
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpFetchInstance, out);
	return true;
}

bool emitIterate(EmitterContext& cx, Iterate* node)
{
	int from = node->getFrom();
	int to = node->getTo();
	int count = to - from;
	if (count < 0 || count >= 256)
		log::error << L"Too many iterations in Iterate node" << Endl;

	// Setup counter, load with initial value.
	EmitterVariable* N = cx.emitOutput(node, L"N", VtFloat, true);
	cx.emitInstruction(OpFetchConstant, N, cx.emitConstant(float(from)));

	EmitterVariable* tmp1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpFetchConstant, tmp1, cx.emitConstant(float(to)));

	// Initialize output accumulator.
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	EmitterVariable* initial = cx.emitInput(node, L"Initial");
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

	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* xin = expandTypes(cx, in, VtFloat4);
	cx.emitInstruction(OpMove, out, xin);
	collapseTypes(cx, in, xin);

	// Check condition.
	EmitterVariable* cond = cx.emitInput(node, L"Condition");
	uint32_t condOffset = 0;
	if (cond)
	{
		Instruction inst(OpJumpIfZero, 0, 0);
		condOffset = cx.emitInstruction(inst);
	}

	// Increment counter, repeat loop if not at target.
	cx.emitInstruction(OpIncrement, N);

	EmitterVariable* tmp2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpCompareGreater, tmp2, N, tmp1);

	Instruction inst(OpJumpIfZero, tmp2->reg, getRelativeOffset(cx.getCurrentAddress(), address));
	cx.emitInstruction(inst);

	// Patch up condition jump address.
	if (cond)
	{
		Instruction inst(OpJumpIfZero, cond->reg, getRelativeOffset(condOffset, cx.getCurrentAddress()));
		cx.emitInstruction(condOffset, inst);
	}

	cx.releaseOutput(node, L"N");

	cx.freeTemporary(tmp2);
	cx.freeTemporary(tmp1);
	return true;
}

bool emitIterate2d(EmitterContext& cx, Iterate2d* node)
{
	int fromX = node->getFromX();
	int toX = node->getToX();
	int fromY = node->getFromY();
	int toY = node->getToY();

	int countX = toX - fromX;
	int countY = toY - fromY;
	if (countX < 0 || countX >= 256 || countY < 0 || countY >= 256)
		log::error << L"Too many iterations in Iterate node" << Endl;

	// Initialize output accumulator.
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	EmitterVariable* initial = cx.emitInput(node, L"Initial");
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

	// Setup outer counter, load with initial value.
	EmitterVariable* Y = cx.emitOutput(node, L"Y", VtFloat, true);
	cx.emitInstruction(OpFetchConstant, Y, cx.emitConstant(float(fromY)));

	EmitterVariable* tmpY1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpFetchConstant, tmpY1, cx.emitConstant(float(toY)));

	uint32_t addressY = cx.getCurrentAddress();

	// Setup inner counter, load with initial value.
	EmitterVariable* X = cx.emitOutput(node, L"X", VtFloat, true);
	cx.emitInstruction(OpFetchConstant, X, cx.emitConstant(float(fromX)));

	EmitterVariable* tmpX1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpFetchConstant, tmpX1, cx.emitConstant(float(toX)));

	uint32_t addressX = cx.getCurrentAddress();

	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* xin = expandTypes(cx, in, VtFloat4);
	cx.emitInstruction(OpMove, out, xin);
	collapseTypes(cx, in, xin);

	// Check condition.
	EmitterVariable* cond = cx.emitInput(node, L"Condition");
	uint32_t condOffset = 0;
	if (cond)
	{
		Instruction inst(OpJumpIfZero, 0, 0);
		condOffset = cx.emitInstruction(inst);
	}

	// Increment inner counter, repeat loop if not at target.
	cx.emitInstruction(OpIncrement, X);

	EmitterVariable* tmpX2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpCompareGreater, tmpX2, X, tmpX1);

	Instruction instX(OpJumpIfZero, tmpX2->reg, getRelativeOffset(cx.getCurrentAddress(), addressX));
	cx.emitInstruction(instX);

	// Increment counter, repeat loop if not at target.
	cx.emitInstruction(OpIncrement, Y);

	EmitterVariable* tmpY2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpCompareGreater, tmpY2, Y, tmpY1);

	Instruction instY(OpJumpIfZero, tmpY2->reg, getRelativeOffset(cx.getCurrentAddress(), addressY));
	cx.emitInstruction(instY);

	// Patch up condition jump address.
	if (cond)
	{
		Instruction inst(OpJumpIfZero, cond->reg, getRelativeOffset(condOffset, cx.getCurrentAddress()));
		cx.emitInstruction(condOffset, inst);
	}

	cx.releaseOutput(node, L"X");
	cx.releaseOutput(node, L"Y");

	cx.freeTemporary(tmpY2);
	cx.freeTemporary(tmpX2);
	cx.freeTemporary(tmpY1);
	cx.freeTemporary(tmpX1);
	return true;
}

bool emitIndexedUniform(EmitterContext& cx, IndexedUniform* node)
{
	EmitterVariableType variableType;
	
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

	EMIT_MANDATORY_IN(index, cx, node, L"Index");
	EmitterVariable* in = cx.emitUniform(node->getParameterName(), variableType, node->getLength());
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpFetchIndexedUniform, out, in, index);
	cx.releaseInput(node, L"Index");
	return true;
}

bool emitLength(EmitterContext& cx, Length* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpLength, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitLerp(EmitterContext& cx, Lerp* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");
	EMIT_MANDATORY_IN(blend, cx, node, L"Blend");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in1->type);
	cx.emitInstruction(OpLerp, out, blend, in1, in2);
	cx.releaseInput(node, L"Blend");
	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitLog(EmitterContext& cx, Log* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
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
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitMatrixIn(EmitterContext& cx, MatrixIn* node)
{
	EmitterVariable* axisX = cx.emitInput(node, L"XAxis");
	EmitterVariable* axisY = cx.emitInput(node, L"YAxis");
	EmitterVariable* axisZ = cx.emitInput(node, L"ZAxis");
	EmitterVariable* translate = cx.emitInput(node, L"Translate");
	EmitterVariable* tmp = cx.allocTemporary(VtFloat4x4);
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4x4);

	if (axisX)
	{
		EmitterVariable* xaxisX = expandTypes(cx, axisX, VtFloat4);
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
		EmitterVariable* xaxisY = expandTypes(cx, axisY, VtFloat4);
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
		EmitterVariable* xaxisZ = expandTypes(cx, axisZ, VtFloat4);
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
		EmitterVariable* xtranslate = expandTypes(cx, translate, VtFloat4);
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

	cx.releaseInput(node, L"Translate");
	cx.releaseInput(node, L"ZAxis");
	cx.releaseInput(node, L"YAxis");
	cx.releaseInput(node, L"XAxis");
	return true;
}

bool emitMatrixOut(EmitterContext& cx, MatrixOut* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");

	EmitterVariable* axisX = cx.emitOutput(node, L"XAxis", VtFloat4);
	EmitterVariable* axisY = cx.emitOutput(node, L"YAxis", VtFloat4);
	EmitterVariable* axisZ = cx.emitOutput(node, L"ZAxis", VtFloat4);
	EmitterVariable* translate = cx.emitOutput(node, L"Translate", VtFloat4);

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

	cx.releaseInput(node, L"Input");
	return true;
}

bool emitMax(EmitterContext& cx, Max* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");

	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	EmitterVariable* xin1 = expandTypes(cx, in1, out->type);
	EmitterVariable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpMax, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitMin(EmitterContext& cx, Min* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");

	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	EmitterVariable* xin1 = expandTypes(cx, in1, out->type);
	EmitterVariable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpMin, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitMixIn(EmitterContext& cx, MixIn* node)
{
	EmitterVariable* x = cx.emitInput(node, L"X");
	EmitterVariable* y = cx.emitInput(node, L"Y");
	EmitterVariable* z = cx.emitInput(node, L"Z");
	EmitterVariable* w = cx.emitInput(node, L"W");

	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
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

	if (w)
		cx.releaseInput(node, L"W");
	if (z)
		cx.releaseInput(node, L"Z");
	if (y)
		cx.releaseInput(node, L"Y");
	if (x)
		cx.releaseInput(node, L"X");
	return true;
}

bool emitMixOut(EmitterContext& cx, MixOut* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");

	EmitterVariable* x = cx.emitOutput(node, L"X", VtFloat);
	EmitterVariable* y = cx.emitOutput(node, L"Y", VtFloat);
	EmitterVariable* z = cx.emitOutput(node, L"Z", VtFloat);
	EmitterVariable* w = cx.emitOutput(node, L"W", VtFloat);

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

	cx.releaseInput(node, L"Input");
	return true;
}

bool emitMul(EmitterContext& cx, Mul* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");

	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));
	
	EmitterVariable* xin1 = expandTypes(cx, in1, out->type);
	EmitterVariable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpMul, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitMulAdd(EmitterContext& cx, MulAdd* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");
	EMIT_MANDATORY_IN(in3, cx, node, L"Input3");
	
	EmitterVariableType type = std::max(std::max(in1->type, in2->type), in3->type);
	EmitterVariable* out = cx.emitOutput(node, L"Output", type);

	EmitterVariable* xin1 = expandTypes(cx, in1, out->type);
	EmitterVariable* xin2 = expandTypes(cx, in2, out->type);
	EmitterVariable* xin3 = expandTypes(cx, in3, out->type);

	cx.emitInstruction(OpMulAdd, out, xin1, xin2, xin3);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);
	collapseTypes(cx, in3, xin3);

	cx.releaseInput(node, L"Input3");
	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitNeg(EmitterContext& cx, Neg* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpNeg, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitNormalize(EmitterContext& cx, Normalize* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpNormalize, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitPixelOutput(EmitterContext& cx, PixelOutput* node)
{
	cx.enterPixel();

	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitVarying(0);
	cx.emitInstruction(OpStoreVarying, out, in);
	cx.releaseInput(node, L"Input");

	RenderState rs = node->getRenderState();

	RenderStateDesc state;
	state.depthEnable = rs.depthEnable;
	state.depthWriteEnable = rs.depthWriteEnable;
	state.cullMode = rs.cullMode;
	state.blendEnable = rs.blendEnable;
	state.blendOperation = rs.blendColorOperation;
	state.blendSource = rs.blendColorSource;
	state.blendDestination = rs.blendColorDestination;
	cx.setRenderState(state);

	return true;
}

bool emitReflect(EmitterContext& cx, Reflect* node)
{
	EMIT_MANDATORY_IN(normal, cx, node, L"Normal");
	EMIT_MANDATORY_IN(direction, cx, node, L"Direction");

	EmitterVariable* out = cx.emitOutput(node, L"Output", direction->type);

	EmitterVariable* tmp1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpDot3, tmp1, normal, direction);

	EmitterVariable* two = cx.emitConstant(2.0f);
	EmitterVariable* tmp2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpMul, tmp2, tmp1, two);

	EmitterVariable* xtmp2 = expandTypes(cx, tmp2, out->type);

	EmitterVariable* tmp3 = cx.allocTemporary(out->type);
	cx.emitInstruction(OpMul, tmp3, normal, xtmp2);

	collapseTypes(cx, tmp2, xtmp2);

	cx.emitInstruction(OpSub, out, tmp3, direction);

	cx.releaseInput(node, L"Direction");
	cx.releaseInput(node, L"Normal");
	return true;
}

bool emitRecipSqrt(EmitterContext& cx, RecipSqrt* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpRecipSqrt, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitRepeat(EmitterContext& cx, Repeat* node)
{
	// Setup counter, load with initial value.
	EmitterVariable* N = cx.emitOutput(node, L"N", VtFloat);
	cx.emitInstruction(OpFetchConstant, N, cx.emitConstant(0.0f));

	// Initialize output accumulator.
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	EmitterVariable* initial = cx.emitInput(node, L"Initial");
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

	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* xin = expandTypes(cx, in, VtFloat4);
	cx.emitInstruction(OpMove, out, xin);
	collapseTypes(cx, in, xin);

	// Check condition.
	EmitterVariable* cond = cx.emitInput(node, L"Condition");
	uint32_t condOffset = 0;
	if (cond)
	{
		Instruction inst(OpJumpIfZero, 0, 0);
		condOffset = cx.emitInstruction(inst);
	}

	// Increment counter, repeat loop if not at target.
	cx.emitInstruction(OpIncrement, N);

	Instruction inst(OpJump, 0, getRelativeOffset(cx.getCurrentAddress(), address));
	cx.emitInstruction(inst);

	// Patch up condition jump address.
	if (cond)
	{
		Instruction inst(OpJumpIfZero, cond->reg, getRelativeOffset(condOffset, cx.getCurrentAddress()));
		cx.emitInstruction(condOffset, inst);
		cx.releaseInput(node, L"Condition");
	}

	return true;
}

bool emitRound(EmitterContext& cx, Round* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpRound, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitPow(EmitterContext& cx, Pow* node)
{
	EMIT_MANDATORY_IN(exp, cx, node, L"Exponent");
	EMIT_MANDATORY_IN(in, cx, node, L"Input");

	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(in->type, exp->type));
	cx.emitInstruction(OpPow, out, in, exp);

	cx.releaseInput(node, L"Input");
	cx.releaseInput(node, L"Exponent");
	return true;
}

bool emitSampler(EmitterContext& cx, Sampler* node)
{
	EMIT_MANDATORY_IN(texture, cx, node, L"Texture");
	EMIT_MANDATORY_IN(texCoord, cx, node, L"TexCoord");

	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Instruction inst(OpSampler, out->reg, texCoord->reg, texture->reg, 0, 0);
	cx.emitInstruction(inst);

	cx.releaseInput(node, L"TexCoord");
	cx.releaseInput(node, L"Texture");
	return true;
}

bool emitScalar(EmitterContext& cx, Scalar* node)
{
	EmitterVariable* in = cx.emitConstant(node->get());
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpFetchConstant, out, in);
	return true;
}

bool emitSign(EmitterContext& cx, Sign* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpSign, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitSin(EmitterContext& cx, Sin* node)
{
	EMIT_MANDATORY_IN(theta, cx, node, L"Theta");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpSin, out, theta);
	cx.releaseInput(node, L"Theta");
	return true;
}

bool emitSqrt(EmitterContext& cx, Sqrt* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpSqrt, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitStep(EmitterContext& cx, Step* node)
{
	EMIT_MANDATORY_IN(x, cx, node, L"X");
	EMIT_MANDATORY_IN(y, cx, node, L"Y");

	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(x->type, y->type));

	EmitterVariable* xx = expandTypes(cx, x, out->type);
	EmitterVariable* xy = expandTypes(cx, y, out->type);

	cx.emitInstruction(OpStep, out, xx, xy);

	collapseTypes(cx, x, xx);
	collapseTypes(cx, y, xy);

	cx.releaseInput(node, L"Y");
	cx.releaseInput(node, L"X");
	return true;
}

bool emitSub(EmitterContext& cx, Sub* node)
{
	EMIT_MANDATORY_IN(in1, cx, node, L"Input1");
	EMIT_MANDATORY_IN(in2, cx, node, L"Input2");
	EmitterVariable* out = cx.emitOutput(node, L"Output", std::max(in1->type, in2->type));

	EmitterVariable* xin1 = expandTypes(cx, in1, out->type);
	EmitterVariable* xin2 = expandTypes(cx, in2, out->type);

	cx.emitInstruction(OpSub, out, xin1, xin2);

	collapseTypes(cx, in1, xin1);
	collapseTypes(cx, in2, xin2);

	cx.releaseInput(node, L"Input2");
	cx.releaseInput(node, L"Input1");
	return true;
}

bool emitSum(EmitterContext& cx, Sum* node)
{
	int from = node->getFrom();
	int to = node->getTo();
	int count = to - from;
	if (count < 0 || count >= 256)
	{
		log::error << L"Too many iterations in Sum node" << Endl;
		return false;
	}

	// Setup counter, load with initial value.
	EmitterVariable* N = cx.emitOutput(node, L"N", VtFloat, true);
	cx.emitInstruction(OpFetchConstant, N, cx.emitConstant(float(from)));

	EmitterVariable* tmp1 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpFetchConstant, tmp1, cx.emitConstant(float(to)));
	
	// Initialize output accumulator.
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Instruction is0(OpSet, out->reg, 0, 0xff, 0, 0);
	cx.emitInstruction(is0);

	uint32_t address = cx.getCurrentAddress();

	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* xin = expandTypes(cx, in, VtFloat4);

	cx.emitInstruction(OpAdd, out, out, xin);

	collapseTypes(cx, in, xin);
	
	// Increment counter, repeat loop if not at target.
	cx.emitInstruction(OpIncrement, N);

	EmitterVariable* tmp2 = cx.allocTemporary(VtFloat);
	cx.emitInstruction(OpCompareGreater, tmp2, N, tmp1);

	Instruction inst(OpJumpIfZero, tmp2->reg, getRelativeOffset(cx.getCurrentAddress(), address));
	cx.emitInstruction(inst);

	cx.releaseOutput(node, L"N");

	cx.freeTemporary(tmp2);
	cx.freeTemporary(tmp1);
	return true;
}

bool emitSwitch(EmitterContext& cx, Switch* node)
{
	EMIT_MANDATORY_IN(select, cx, node, L"Select");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);

	EmitterVariable* N = cx.allocTemporary(VtFloat);

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

		EMIT_MANDATORY_IN_PIN(in, cx, caseInputPin);

		EmitterVariable* xin = expandTypes(cx, in, VtFloat4);
		cx.emitInstruction(OpMove, out, xin);
		collapseTypes(cx, in, xin);

		Instruction is2(OpJump, 0, 0, 0, 0, 0);
		addressBreaks.push_back(cx.emitInstruction(is2));

		is1.offset = getRelativeOffset(addressJump, cx.getCurrentAddress());
		cx.emitInstruction(addressJump, is1);
	}

	cx.freeTemporary(N);

	// Default
	EmitterVariable* in = cx.emitInput(node, L"Default");
	T_ASSERT (in);

	EmitterVariable* xin = expandTypes(cx, in, VtFloat4);
	cx.emitInstruction(OpMove, out, xin);
	collapseTypes(cx, in, xin);

	// Patch break instructions.
	uint32_t addressEnd = cx.getCurrentAddress();
	for (std::vector< uint32_t >::iterator i = addressBreaks.begin(); i != addressBreaks.end(); ++i)
	{
		Instruction inst(OpJump, 0, getRelativeOffset(*i, addressEnd));
		cx.emitInstruction(*i, inst);
	}

	cx.releaseInput(node, L"Default");
	for (uint32_t i = 0; i < uint32_t(cases.size()); ++i)
	{
		const InputPin* caseInputPin = node->getInputPin(i + 2);
		T_ASSERT (caseInputPin);

		cx.releaseInput(caseInputPin);
	}
	cx.releaseInput(node, L"Select");
	return true;
}

bool emitSwizzle(EmitterContext& cx, Swizzle* node)
{
	std::wstring map = node->get();
	T_ASSERT (map.length() > 0);

	const EmitterVariableType types[] = { VtFloat, VtFloat2, VtFloat3, VtFloat4 };
	EmitterVariableType type = types[map.length() - 1];

	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", type);

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

	cx.releaseInput(node, L"Input");
	return true;
}

bool emitTargetSize(EmitterContext& cx, TargetSize* node)
{
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	cx.emitInstruction(OpFetchTargetSize, out);
	return true;
}

bool emitTan(EmitterContext& cx, Tan* node)
{
	EMIT_MANDATORY_IN(theta, cx, node, L"Theta");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat);
	cx.emitInstruction(OpTan, out, theta);
	cx.releaseInput(node, L"Theta");
	return true;
}

bool emitTextureSize(EmitterContext& cx, TextureSize* node)
{
	EMIT_MANDATORY_IN(texture, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	Instruction inst(OpFetchTextureSize, out->reg, texture->reg, 0, 0, 0);
	cx.emitInstruction(inst);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitTransform(EmitterContext& cx, Transform* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EMIT_MANDATORY_IN(transform, cx, node, L"Transform");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpTransform, out, in, transform);
	cx.releaseInput(node, L"Transform");
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitTranspose(EmitterContext& cx, Transpose* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpTranspose, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitTruncate(EmitterContext& cx, Truncate* node)
{
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
	cx.emitInstruction(OpTrunc, out, in);
	cx.releaseInput(node, L"Input");
	return true;
}

bool emitUniform(EmitterContext& cx, Uniform* node)
{
	EmitterVariableType variableType;
	
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
		EmitterVariable* out = cx.emitOutput(node, L"Output", VtTexture);
		out->reg = cx.allocSampler(node->getParameterName());
	}
	else
	{
		EmitterVariable* in = cx.emitUniform(node->getParameterName(), variableType);
		EmitterVariable* out = cx.emitOutput(node, L"Output", in->type);
		cx.emitInstruction(OpFetchUniform, out, in);
	}

	return true;
}

bool emitVector(EmitterContext& cx, Vector* node)
{
	EmitterVariable* in = cx.emitConstant(node->get());
	EmitterVariable* out = cx.emitOutput(node, L"Output", VtFloat4);
	cx.emitInstruction(OpFetchConstant, out, in);
	return true;
}

bool emitVertexInput(EmitterContext& cx, VertexInput* node)
{
	T_ASSERT_M (cx.inVertex(), L"VertexInput node in wrong scope");

	EmitterVariableType type;
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

	EmitterVariable* in = cx.emitVarying(varyingOffset);
	EmitterVariable* out = cx.emitOutput(node, L"Output", type);
	cx.emitInstruction(OpFetchVarying, out, in);

	if (node->getDataUsage() == DuPosition && type != VtFloat4)
	{
		Instruction is1(OpSet, out->reg, 8, 0, 0, 0);
		cx.emitInstruction(is1);
		out->type = VtFloat4;
	}

	return true;
}

bool emitVertexOutput(EmitterContext& cx, VertexOutput* node)
{
	cx.enterVertex();
	
	EMIT_MANDATORY_IN(in, cx, node, L"Input");
	if (in->type != VtFloat4)
	{
		EmitterVariable* tmp = cx.allocTemporary(VtFloat4);
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

		EmitterVariable* out = cx.emitVarying(0);
		cx.emitInstruction(OpStoreVarying, out, tmp);
	}
	else
	{
		EmitterVariable* out = cx.emitVarying(0);
		cx.emitInstruction(OpStoreVarying, out, in);
	}

	cx.releaseInput(node, L"Input");
	return true;
}

struct EmitterImpl
{
	virtual bool emit(EmitterContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public EmitterImpl
{
	typedef bool (*function_t)(EmitterContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function)
	:	m_function(function)
	{
	}

	virtual bool emit(EmitterContext& c, Node* node)
	{
		T_ASSERT (is_a< NodeType >(node));
		return (*m_function)(c, static_cast< NodeType* >(node));
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
	m_emitters[&type_of< RecipSqrt >() ] = new EmitterCast< RecipSqrt >(emitRecipSqrt);
	m_emitters[&type_of< Reflect >()] = new EmitterCast< Reflect >(emitReflect);
	m_emitters[&type_of< Repeat >()] = new EmitterCast< Repeat >(emitRepeat);
	m_emitters[&type_of< Round >()] = new EmitterCast< Round >(emitRound);
	m_emitters[&type_of< Sampler >()] = new EmitterCast< Sampler >(emitSampler);
	m_emitters[&type_of< Scalar >()] = new EmitterCast< Scalar >(emitScalar);
	m_emitters[&type_of< Sign >()] = new EmitterCast< Sign >(emitSign);
	m_emitters[&type_of< Sin >()] = new EmitterCast< Sin >(emitSin);
	m_emitters[&type_of< Sqrt >()] = new EmitterCast< Sqrt >(emitSqrt);
	m_emitters[&type_of< Step >()] = new EmitterCast< Step >(emitStep);
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

bool Emitter::emit(EmitterContext& c, Node* node)
{
	std::map< const TypeInfo*, EmitterImpl* >::iterator i = m_emitters.find(&type_of(node));
	if (i == m_emitters.end())
	{
		log::error << L"No emitter for node type \"" << type_name(node) << L"\"" << Endl;
		return false;
	}

	T_FATAL_ASSERT (i->second);
	if (i->second)
	{
		if (i->second->emit(c, node))
			return true;
		else
		{
			log::error << L"Failed to emit \"" << type_name(node) << L"\"" << Endl;
			return false;
		}
	}
	else
	{
		log::error << L"Failed to emit \"" << type_name(node) << L"\"; node type \"" << type_name(node) << L"\" not supported." << Endl;
		return false;
	}
}

	}
}
