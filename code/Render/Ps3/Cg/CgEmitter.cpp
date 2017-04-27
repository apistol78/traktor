/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps3/Cg/CgContext.h"
#include "Render/Ps3/Cg/CgEmitter.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/VertexElement.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/Script.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

StringOutputStream& assign(StringOutputStream& f, CgVariable* out)
{
	f << cg_type_name(out->getType()) << L" " << out->getName() << L" = ";
	return f;
}

bool emitAbs(CgContext& cx, Abs* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"abs(" << in->getName() << L");" << Endl;
	return true;
}

bool emitAdd(CgContext& cx, Add* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitArcusCos(CgContext& cx, ArcusCos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != CtFloat)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	assign(f, out) << L"acos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitArcusTan(CgContext& cx, ArcusTan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* xy = cx.emitInput(node, L"XY");
	if (!xy || xy->getType() != CtFloat2)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	assign(f, out) << L"atan2(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;
	return true;
}

bool emitClamp(CgContext& cx, Clamp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	if (node->getMin() == 0.0f && node->getMax() == 1.0f)
		assign(f, out) << L"saturate(" << in->getName() << L");" << Endl;
	else
		assign(f, out) << L"clamp(" << in->getName() << L", " << node->getMin() << L", " << node->getMax() << L");" << Endl;
	return true;
}

bool emitColor(CgContext& cx, Color* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat4);
	if (!out)
		return false;
	traktor::Color4ub color = node->getColor();
	f << L"const float4 " << out->getName() << L" = float4(" << (color.r / 255.0f) << L", " << (color.g / 255.0f) << L", " << (color.b / 255.0f) << L", " << (color.a / 255.0f) << L");" << Endl;
	return true;
}

bool emitConditional(CgContext& cx, Conditional* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);

	// Emit input and reference branches.
	CgVariable* in = cx.emitInput(node, L"Input");
	CgVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	CgVariable caseTrue, caseFalse;
	std::wstring caseTrueBranch, caseFalseBranch;

	// Emit true branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
		cx.getShader().pushScope();

		CgVariable* ct = cx.emitInput(node, L"CaseTrue");
		if (!ct)
			return false;

		caseTrue = *ct;
		caseTrueBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(CgShader::BtBody);
	}

	// Emit false branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
		cx.getShader().pushScope();

		CgVariable* cf = cx.emitInput(node, L"CaseFalse");
		if (!cf)
			return false;

		caseFalse = *cf;
		caseFalseBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(CgShader::BtBody);
	}

	// Create output variable.
	CgType outputType = std::max< CgType >(caseTrue.getType(), caseFalse.getType());
	
	CgVariable* out = cx.emitOutput(node, L"Output", outputType);
	f << cg_type_name(out->getType()) << L" " << out->getName() << L";" << Endl;

	//if (node->getBranch() == Conditional::BrStatic)
	//	f << L"[flatten]" << Endl;
	//else if (node->getBranch() == Conditional::BrDynamic)
	//	f << L"[branch]" << Endl;

	// Create condition statement.
	switch (node->getOperator())
	{
	case Conditional::CoLess:
		f << L"if (" << in->getName() << L" < " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoLessEqual:
		f << L"if (" << in->getName() << L" <= " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoEqual:
		f << L"if (" << in->getName() << L" == " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoNotEqual:
		f << L"if (" << in->getName() << L" != " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoGreater:
		f << L"if (" << in->getName() << L" > " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoGreaterEqual:
		f << L"if (" << in->getName() << L" >= " << ref->getName() << L")" << Endl;
		break;
	default:
		T_ASSERT (0);
	}

	f << L"{" << Endl;
	f << IncreaseIndent;

	f << caseTrueBranch;
	f << out->getName() << L" = " << caseTrue.cast(outputType) << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;
	f << L"else" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	f << caseFalseBranch;
	f << out->getName() << L" = " << caseFalse.cast(outputType) << L";" << Endl;
	
	f << DecreaseIndent;
	f << L"}" << Endl;

	return true;
}

bool emitCos(CgContext& cx, Cos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != CtFloat)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	assign(f, out) << L"cos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitCross(CgContext& cx, Cross* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat3);
	assign(f, out) << L"cross(" << in1->cast(CtFloat3) << L", " << in2->cast(CtFloat3) << L");" << Endl;
	return true;
}

bool emitDerivative(CgContext& cx, Derivative* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* input = cx.emitInput(node, L"Input");
	if (!input)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", input->getType());
	switch (node->getAxis())
	{
	case Derivative::DaX:
		assign(f, out) << L"ddx(" << input->getName() << L");" << Endl;
		break;
	case Derivative::DaY:
		assign(f, out) << L"ddy(" << input->getName() << L");" << Endl;
		break;
	default:
		return false;
	}
	return true;
}

bool emitDiscard(CgContext& cx, Discard* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);

	// Emit input and reference branches.
	CgVariable* in = cx.emitInput(node, L"Input");
	CgVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	//f << L"[branch]" << Endl;

	// Create condition statement.
	switch (node->getOperator())
	{
	case Conditional::CoLess:
		f << L"if (" << in->getName() << L" >= " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoLessEqual:
		f << L"if (" << in->getName() << L" > " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoEqual:
		f << L"if (" << in->getName() << L" != " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoNotEqual:
		f << L"if (" << in->getName() << L" == " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoGreater:
		f << L"if (" << in->getName() << L" <= " << ref->getName() << L")" << Endl;
		break;
	case Conditional::CoGreaterEqual:
		f << L"if (" << in->getName() << L" < " << ref->getName() << L")" << Endl;
		break;
	default:
		T_ASSERT (0);
	}

	f << L"\tdiscard;" << Endl;

	CgVariable* pass = cx.emitInput(node, L"Pass");
	if (!pass)
		return false;

	CgVariable* out = cx.emitOutput(node, L"Output", pass->getType());
	assign(f, out) << pass->getName() << L";" << Endl;

	return true;
}

bool emitDiv(CgContext& cx, Div* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitDot(CgContext& cx, Dot* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	assign(f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitExp(CgContext& cx, Exp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"exp(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFraction(CgContext& cx, Fraction* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"frac(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFragmentPosition(CgContext& cx, FragmentPosition* node)
{
	if (!cx.inPixel())
		return false;

	cx.allocateVPos();

	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat2);
	assign(f, out) << L"float2(vPos.x, vPos.y);" << Endl;

	return true;
}

bool emitIndexedUniform(CgContext& cx, IndexedUniform* node)
{
	const CgType c_parameterType[] = { CtFloat, CtFloat4, CtFloat4x4 };

	CgVariable* index = cx.emitInput(node, L"Index");
	if (!index)
		return false;

	CgVariable* out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		c_parameterType[node->getParameterType()]
	);

	StringOutputStream& fb = cx.getShader().getOutputStream(CgShader::BtBody);
	assign(fb, out) << node->getParameterName() << L"[" << index->getName() << L"];" << Endl;

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		uint32_t registerIndex = cx.getShader().addUniform(node->getParameterName(), out->getType(), node->getLength());
		if (registerIndex == ~0U)
			return false;

		StringOutputStream& fu = cx.getShader().getOutputStream(CgShader::BtUniform);
		fu << L"uniform " << cg_type_name(out->getType()) << L" " << node->getParameterName() << L"[" << node->getLength() << L"] : register(c" << registerIndex << L");" << Endl;
	}

	return true;
}

bool emitInstance(CgContext& cx, Instance* node)
{
	CgVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		L"__private__instanceID",
		CtFloat
	);

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(L"__private__instanceID") == uniforms.end())
	{
		uint32_t registerIndex = cx.getShader().addUniform(L"__private__instanceID", CtFloat, 1);
		if (registerIndex == ~0U)
			return false;

		StringOutputStream& fu = cx.getShader().getOutputStream(CgShader::BtUniform);
		fu << L"uniform float __private__instanceID : register(c" << registerIndex << L");" << Endl;
	}

	return true;
}

bool emitInterpolator(CgContext& cx, Interpolator* node)
{
	if (!cx.inPixel())
	{
		// We're already in vertex state; skip interpolation.
		CgVariable* in = cx.emitInput(node, L"Input");
		if (!in)
			return false;

		CgVariable* out = cx.emitOutput(node, L"Output", in->getType());

		StringOutputStream& fb = cx.getShader().getOutputStream(CgShader::BtBody);
		assign(fb, out) << in->getName() << L";" << Endl;

		return true;
	}

	cx.enterVertex();

	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	cx.enterPixel();

	int32_t interpolatorWidth = cg_type_width(in->getType());
	if (!interpolatorWidth)
		return false;

	int32_t interpolatorId;
	int32_t interpolatorOffset;

	bool declare = cx.allocateInterpolator(interpolatorWidth, interpolatorId, interpolatorOffset);

	std::wstring interpolatorName = L"Attr" + toString(interpolatorId);
	std::wstring interpolatorMask = interpolatorName + L"." + std::wstring(L"xyzw").substr(interpolatorOffset, interpolatorWidth);

	StringOutputStream& vfb = cx.getVertexShader().getOutputStream(CgShader::BtBody);
	vfb << L"o." << interpolatorMask << L" = " << in->getName() << L";" << Endl;

	cx.getPixelShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		L"i." + interpolatorMask,
		in->getType()
	);

	if (declare)
	{
		StringOutputStream& vfo = cx.getVertexShader().getOutputStream(CgShader::BtOutput);
		vfo << L"float4 " << interpolatorName << L" : TEXCOORD" << interpolatorId << L";" << Endl;

		StringOutputStream& pfi = cx.getPixelShader().getOutputStream(CgShader::BtInput);
		pfi << L"float4 " << interpolatorName << L" : TEXCOORD" << interpolatorId << L";" << Endl;
	}

	return true;
}

bool emitIterate(CgContext& cx, Iterate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	CgVariable* N = cx.emitOutput(node, L"N", CtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	CgVariable* out = cx.emitOutput(node, L"Output", CtVoid);
	T_ASSERT (out);

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	std::vector< const OutputPin* > outputPins;
	std::vector< const OutputPin* > dependentOutputPins(2);
	dependentOutputPins[0] = node->findOutputPin(L"N");
	dependentOutputPins[1] = node->findOutputPin(L"Output");
	cx.findNonDependentOutputs(node, L"Input", dependentOutputPins, outputPins);
	for (std::vector< const OutputPin* >::const_iterator i = outputPins.begin(); i != outputPins.end(); ++i)
		cx.emit((*i)->getNode());

	// Write input branch in a temporary output stream.
	StringOutputStream fs;
	cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		CgVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		// Emit post condition if connected; break iteration if condition is false.
		CgVariable* condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (!(bool)" << condition->cast(CtFloat) << L")" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(CgShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	CgVariable* initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << N->getName() << L" = " << node->getFrom() << L"; " << N->getName() << L" <= " << node->getTo() << L"; ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << fs.str();
	f << out->getName() << L" = " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;	

	return true;
}

bool emitIterate2d(CgContext& cx, Iterate2d* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	std::wstring inputName;

	// Create iterator variables.
	CgVariable* X = cx.emitOutput(node, L"X", CtFloat);
	T_ASSERT (X);

	CgVariable* Y = cx.emitOutput(node, L"Y", CtFloat);
	T_ASSERT (Y);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	CgVariable* out = cx.emitOutput(node, L"Output", CtVoid);
	T_ASSERT (out);

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	std::vector< const OutputPin* > outputPins;
	std::vector< const OutputPin* > dependentOutputPins(3);
	dependentOutputPins[0] = node->findOutputPin(L"X");
	dependentOutputPins[1] = node->findOutputPin(L"Y");
	dependentOutputPins[2] = node->findOutputPin(L"Output");
	cx.findNonDependentOutputs(node, L"Input", dependentOutputPins, outputPins);
	for (std::vector< const OutputPin* >::const_iterator i = outputPins.begin(); i != outputPins.end(); ++i)
		cx.emit((*i)->getNode());

	// Write input branch in a temporary output stream.
	StringOutputStream fs;
	cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
	cx.getShader().pushScope();

	CgVariable* input = cx.emitInput(node, L"Input");
	if (!input)
		return false;

	// Emit post condition if connected; break iteration if condition is false.
	CgVariable* condition = cx.emitInput(node, L"Condition");
	if (condition)
	{
		fs << L"if (!(bool)" << condition->cast(CtFloat) << L")" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	inputName = input->getName();

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	out->setType(input->getType());

	cx.getShader().popScope();
	cx.getShader().popOutputStream(CgShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	CgVariable* initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << X->getName() << L" = " << node->getFromX() << L"; " << X->getName() << L" <= " << node->getToX() << L"; ++" << X->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	f << L"for (float " << Y->getName() << L" = " << node->getFromY() << L"; " << Y->getName() << L" <= " << node->getToY() << L"; ++" << Y->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << fs.str();
	f << out->getName() << L" = " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;	

	// Emit outer loop post condition.
	if (condition)
	{
		fs << L"if (!(bool)" << condition->cast(CtFloat) << L")" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	f << DecreaseIndent;
	f << L"}" << Endl;	

	return true;
}

bool emitLength(CgContext& cx, Length* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	assign(f, out) << L"length(" << in->getName() << L");" << Endl;
	return true;
}

bool emitLerp(CgContext& cx, Lerp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* blend = cx.emitInput(node, L"Blend");
	if (!blend || blend->getType() != CtFloat)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in1->getType());
	assign(f, out) << L"lerp(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;
	return true;
}

bool emitLog(CgContext& cx, Log* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	switch (node->getBase())
	{
	case Log::LbTwo:
		assign(f, out) << L"log2(" << in->getName() << L");" << Endl;
		break;

	case Log::LbTen:
		assign(f, out) << L"log10(" << in->getName() << L");" << Endl;
		break;

	case Log::LbNatural:
		assign(f, out) << L"log(" << in->getName() << L");" << Endl;
		break;
	}
	return true;
}

bool emitMatrixIn(CgContext& cx, MatrixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* xaxis = cx.emitInput(node, L"XAxis");
	CgVariable* yaxis = cx.emitInput(node, L"YAxis");
	CgVariable* zaxis = cx.emitInput(node, L"ZAxis");
	CgVariable* translate = cx.emitInput(node, L"Translate");
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat4x4);
	assign(f, out) << L"transpose(float4x4(" << Endl;
	f << IncreaseIndent;
	f << (xaxis     ? xaxis->cast(CtFloat4)     : L"1.0f, 0.0f, 0.0f, 0.0f") << L"," << Endl;
	f << (yaxis     ? yaxis->cast(CtFloat4)     : L"0.0f, 1.0f, 0.0f, 0.0f") << L"," << Endl;
	f << (zaxis     ? zaxis->cast(CtFloat4)     : L"0.0f, 0.0f, 1.0f, 0.0f") << L"," << Endl;
	f << (translate ? translate->cast(CtFloat4) : L"0.0f, 0.0f, 0.0f, 1.0f") << Endl;
	f << DecreaseIndent;
	f << L"));" << Endl;
	return true;
}

bool emitMatrixOut(CgContext& cx, MatrixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* xaxis = cx.emitOutput(node, L"XAxis", CtFloat4);
	if (xaxis)
		assign(f, xaxis) << in->getName() << L"._11_21_31_41;" << Endl;
	CgVariable* yaxis = cx.emitOutput(node, L"YAxis", CtFloat4);
	if (yaxis)
		assign(f, yaxis) << in->getName() << L"._12_22_32_42;" << Endl;
	CgVariable* zaxis = cx.emitOutput(node, L"ZAxis", CtFloat4);
	if (zaxis)
		assign(f, zaxis) << in->getName() << L"._13_23_33_43;" << Endl;
	CgVariable* translate = cx.emitOutput(node, L"Translate", CtFloat4);
	if (translate)
		assign(f, translate) << in->getName() << L"._14_24_34_44;" << Endl;
	return true;
}

bool emitMax(CgContext& cx, Max* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"max(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMin(CgContext& cx, Min* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMixIn(CgContext& cx, MixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* x = cx.emitInput(node, L"X");
	CgVariable* y = cx.emitInput(node, L"Y");
	CgVariable* z = cx.emitInput(node, L"Z");
	CgVariable* w = cx.emitInput(node, L"W");
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat4);
	assign(f, out) << L"float4(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L", " << (z ? z->getName() : L"0.0f") << L", " << (w ? w->getName() : L"0.0f") << L");" << Endl;
	return true;
}

bool emitMixOut(CgContext& cx, MixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	switch (in->getType())
	{
	case CtFloat:
		{
			CgVariable* x = cx.emitOutput(node, L"X", CtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
		}
		break;

	case CtFloat2:
		{
			CgVariable* x = cx.emitOutput(node, L"X", CtFloat);
			CgVariable* y = cx.emitOutput(node, L"Y", CtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
		}
		break;

	case CtFloat3:
		{
			CgVariable* x = cx.emitOutput(node, L"X", CtFloat);
			CgVariable* y = cx.emitOutput(node, L"Y", CtFloat);
			CgVariable* z = cx.emitOutput(node, L"Z", CtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
			assign(f, z) << in->getName() << L".z;" << Endl;
		}
		break;

	case CtFloat4:
		{
			CgVariable* x = cx.emitOutput(node, L"X", CtFloat);
			CgVariable* y = cx.emitOutput(node, L"Y", CtFloat);
			CgVariable* z = cx.emitOutput(node, L"Z", CtFloat);
			CgVariable* w = cx.emitOutput(node, L"W", CtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
			assign(f, z) << in->getName() << L".z;" << Endl;
			assign(f, w) << in->getName() << L".w;" << Endl;
		}
		break;

	default:
		return false;
	}

	return true;
}

bool emitMul(CgContext& cx, Mul* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitMulAdd(CgContext& cx, MulAdd* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	CgVariable* in3 = cx.emitInput(node, L"Input3");
	if (!in1 || !in2 || !in3)
		return false;
	CgType type = std::max< CgType >(std::max< CgType >(in1->getType(), in2->getType()), in3->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L" + " << in3->cast(type) << L";" << Endl;
	return true;
}

bool emitNeg(CgContext& cx, Neg* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"-" << in->getName() << L";" << Endl;
	return true;
}

bool emitNormalize(CgContext& cx, Normalize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << in->getName() << L" * rsqrt(dot(" << in->getName() << L", " << in->getName() << L"));" << Endl;

	return true;
}

bool emitPixelOutput(CgContext& cx, PixelOutput* node)
{
	const uint32_t gcmCullFace[] =
	{
		CELL_GCM_FRONT,
		CELL_GCM_BACK,
		CELL_GCM_FRONT
	};

	const uint16_t gcmBlendEquation[] =
	{
		CELL_GCM_FUNC_ADD,
		CELL_GCM_FUNC_SUBTRACT,
		CELL_GCM_FUNC_REVERSE_SUBTRACT,
		CELL_GCM_MIN,
		CELL_GCM_MAX
	};

	const uint16_t gcmBlendFunction[] =
	{
		CELL_GCM_ONE,
		CELL_GCM_ZERO,
		CELL_GCM_SRC_COLOR,
		CELL_GCM_ONE_MINUS_SRC_COLOR,
		CELL_GCM_DST_COLOR,
		CELL_GCM_ONE_MINUS_DST_COLOR,
		CELL_GCM_SRC_ALPHA,
		CELL_GCM_ONE_MINUS_SRC_ALPHA,
		CELL_GCM_DST_ALPHA,
		CELL_GCM_ONE_MINUS_DST_ALPHA
	};

	const uint32_t gcmFunction[] =
	{
		CELL_GCM_ALWAYS,
		CELL_GCM_NEVER,
		CELL_GCM_LESS,
		CELL_GCM_LEQUAL,
		CELL_GCM_GREATER,
		CELL_GCM_GEQUAL,
		CELL_GCM_EQUAL,
		CELL_GCM_NOTEQUAL
	};

	const uint32_t gcmStencilOperation[] =
	{
		CELL_GCM_KEEP,
		CELL_GCM_ZERO,
		CELL_GCM_REPLACE,
		CELL_GCM_INCR,
		CELL_GCM_DECR,
		CELL_GCM_INVERT,
		CELL_GCM_INCR_WRAP,
		CELL_GCM_DECR_WRAP
	};

	cx.enterPixel();

	const wchar_t* inputs[] = { L"Input", L"Input1", L"Input2", L"Input3" };
	CgVariable* in[4] = { 0 };

	for (int32_t i = 0; i < sizeof_array(in); ++i)
		in[i] = cx.emitInput(node, inputs[i]);

	if (!in[0])
		return false;

	for (int32_t i = 0; i < sizeof_array(in); ++i)
	{
		if (!in[i])
			continue;

		StringOutputStream& fpo = cx.getPixelShader().getOutputStream(CgShader::BtOutput);
		fpo << L"float4 Color" << i << L" : COLOR" << i << L";" << Endl;

		StringOutputStream& fpb = cx.getPixelShader().getOutputStream(CgShader::BtBody);
		fpb << L"o.Color" << i << L" = " << in[i]->cast(CtFloat4) << L";" << Endl;
	}

	const RenderState& rs = node->getRenderState();
	RenderStateGCM& rsgcm = cx.getRenderState();

	rsgcm.cullFaceEnable = rs.cullMode == CmNever ? CELL_GCM_FALSE : CELL_GCM_TRUE;
	rsgcm.cullFace = gcmCullFace[rs.cullMode];
	rsgcm.blendEnable = rs.blendEnable ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rsgcm.blendEquation = gcmBlendEquation[rs.blendColorOperation];
	rsgcm.blendFuncSrc = gcmBlendFunction[rs.blendColorSource];
	rsgcm.blendFuncDest = gcmBlendFunction[rs.blendColorDestination];
	rsgcm.depthTestEnable = rs.depthEnable ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rsgcm.depthMask = rs.depthWriteEnable ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rsgcm.depthFunc = gcmFunction[rs.depthFunction];
	rsgcm.alphaTestEnable = rs.alphaTestEnable ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rsgcm.alphaFunc = gcmFunction[rs.alphaTestFunction];
	rsgcm.alphaRef = rs.alphaTestReference;
	rsgcm.stencilTestEnable = rs.stencilEnable ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rsgcm.stencilFunc = gcmFunction[rs.stencilFunction];
	rsgcm.stencilRef = rs.stencilReference;
	rsgcm.stencilOpFail = gcmStencilOperation[rs.stencilFail];
	rsgcm.stencilOpZFail = gcmStencilOperation[rs.stencilZFail];
	rsgcm.stencilOpZPass = gcmStencilOperation[rs.stencilPass];

	rsgcm.colorMask = 0;
	if (rs.colorWriteMask & CwRed)
		rsgcm.colorMask |= CELL_GCM_COLOR_MASK_R;
	if (rs.colorWriteMask & CwGreen)
		rsgcm.colorMask |= CELL_GCM_COLOR_MASK_G;
	if (rs.colorWriteMask & CwBlue)
		rsgcm.colorMask |= CELL_GCM_COLOR_MASK_B;
	if (rs.colorWriteMask & CwAlpha)
		rsgcm.colorMask |= CELL_GCM_COLOR_MASK_A;

	cx.setRegisterCount(node->getRegisterCount());
	return true;
}

bool emitRecipSqrt(CgContext& cx, RecipSqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"rsqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitPolynomial(CgContext& cx, Polynomial* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);

	CgVariable* x = cx.emitInput(node, L"X");
	CgVariable* coeffs = cx.emitInput(node, L"Coefficients");
	if (!x || !coeffs)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);

	assign(f, out);
	switch (coeffs->getType())
	{
	case CtFloat:
		f << coeffs->getName() << L".x * " << x->getName();
		break;
	case CtFloat2:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".y * " << x->getName();
		break;
	case CtFloat3:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".z * " << x->getName();
		break;
	case CtFloat4:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 4) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".z * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".w * " << x->getName();
		break;
	}
	f << L";" << Endl;

	return true;
}

bool emitPow(CgContext& cx, Pow* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* exponent = cx.emitInput(node, L"Exponent");
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!exponent || !in)
		return false;
	CgType type = std::max< CgType >(exponent->getType(), in->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"pow(" << in->cast(type) << L", " << exponent->cast(type) << L");" << Endl;
	return true;
}

bool emitReflect(CgContext& cx, Reflect* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* normal = cx.emitInput(node, L"Normal");
	CgVariable* direction = cx.emitInput(node, L"Direction");
	if (!normal || !direction)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat3);
	assign(f, out) << L"reflect(" << direction->cast(CtFloat3) << L", " << normal->cast(CtFloat3) << L");" << Endl;
	return true;
}

bool emitRepeat(CgContext& cx, Repeat* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	CgVariable* N = cx.emitOutput(node, L"N", CtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	CgVariable* out = cx.emitOutput(node, L"Output", CtVoid);
	T_ASSERT (out);

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	std::vector< const OutputPin* > outputPins;
	std::vector< const OutputPin* > dependentOutputPins(2);
	dependentOutputPins[0] = node->findOutputPin(L"N");
	dependentOutputPins[1] = node->findOutputPin(L"Output");
	cx.findNonDependentOutputs(node, L"Input", dependentOutputPins, outputPins);
	for (std::vector< const OutputPin* >::const_iterator i = outputPins.begin(); i != outputPins.end(); ++i)
		cx.emit((*i)->getNode());

	// Write input branch in a temporary output stream.
	StringOutputStream fs;
	cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		// Emit pre-condition, break iteration if condition is false.
		CgVariable* condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (!(bool)" << condition->cast(CtFloat) << L")" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		CgVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(CgShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	CgVariable* initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << N->getName() << L" = 0.0f;; ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << fs.str();
	f << out->getName() << L" = " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;	

	return true;
}

bool emitRound(CgContext& cx, Round* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"round(" << in->getName() << L");" << Endl;
	return true;
}

bool emitSampler(CgContext& cx, Sampler* node)
{
	const uint8_t gcmFilter[] =
	{
		CELL_GCM_TEXTURE_NEAREST,
		CELL_GCM_TEXTURE_LINEAR
	};

	const uint8_t gcmAddress[] =
	{
		CELL_GCM_TEXTURE_WRAP,
		CELL_GCM_TEXTURE_MIRROR,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_BORDER
	};

	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);

	CgVariable* texture = cx.emitInput(node, L"Texture");
	if (!texture || texture->getType() < CtTexture2D)
		return false;

	CgVariable* texCoord = cx.emitInput(node, L"TexCoord");
	if (!texCoord)
		return false;

	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat4);

	int32_t stage;
	bool defineSampler = cx.getShader().defineSamplerTexture(texture->getName(), stage);
	std::wstring samplerName = L"sampler_" + toString(stage);

	if (defineSampler)
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(CgShader::BtUniform);
		fu << L"sampler " << samplerName << L" : register(s" << stage << L");" << Endl;

		RenderStateGCM& rs = cx.getRenderState();
		SamplerStateGCM& ss = rs.samplerStates[stage];

		const SamplerState& samplerState = node->getSamplerState();

		bool minLinear = samplerState.minFilter != FtPoint;
		bool mipLinear = samplerState.mipFilter != FtPoint;

		if (!minLinear && !mipLinear)
			ss.minFilter = CELL_GCM_TEXTURE_NEAREST;
		else if (!minLinear && mipLinear)
			ss.minFilter = CELL_GCM_TEXTURE_NEAREST_LINEAR;
		else if (minLinear && !mipLinear)
			ss.minFilter = CELL_GCM_TEXTURE_LINEAR_NEAREST;
		else
			ss.minFilter = CELL_GCM_TEXTURE_LINEAR_LINEAR;

		ss.magFilter = gcmFilter[samplerState.magFilter];
		ss.wrapU = gcmAddress[samplerState.addressU];
		ss.wrapV = gcmAddress[samplerState.addressV];
		ss.wrapW = gcmAddress[samplerState.addressW];
	}

	if (cx.inPixel())
	{
		switch (texture->getType())
		{
		case CtTexture2D:
			assign(f, out) << L"tex2D(" << samplerName << L", " << texCoord->getName() << L");" << Endl;
			break;

		case CtTexture3D:
			assign(f, out) << L"tex3D(" << samplerName << L", " << texCoord->getName() << L");" << Endl;
			break;

		case CtTextureCube:
			assign(f, out) << L"texCUBE(" << samplerName << L", " << texCoord->getName() << L");" << Endl;
			break;
		}
	}
	if (cx.inVertex())
	{
		switch (texture->getType())
		{
		case CtTexture2D:
			assign(f, out) << L"tex2Dlod(" << samplerName << L", " << texCoord->cast(CtFloat4) << L");" << Endl;
			break;

		case CtTexture3D:
			assign(f, out) << L"tex3Dlod(" << samplerName << L", " << texCoord->cast(CtFloat4) << L");" << Endl;
			break;

		case CtTextureCube:
			assign(f, out) << L"texCUBElod(" << samplerName << L", " << texCoord->cast(CtFloat4) << L");" << Endl;
			break;
		}
	}

	return true;
}

bool emitScalar(CgContext& cx, Scalar* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	f << L"const float " << out->getName() << L" = " << node->get() << L";" << Endl;
	return true;
}

bool emitScript(CgContext& cx, Script* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);

	// Get platform specific script from node.
	std::wstring script = node->getScript();
	if (script.empty())
		return false;

	// Emit input and outputs.
	int32_t inputPinCount = node->getInputPinCount();
	int32_t outputPinCount = node->getOutputPinCount();

	RefArray< CgVariable > in(inputPinCount);
	RefArray< CgVariable > out(outputPinCount);

	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const TypedOutputPin* outputPin = static_cast< const TypedOutputPin* >(node->getOutputPin(i));
		T_ASSERT (outputPin);

		out[i] = cx.emitOutput(
			node,
			outputPin->getName(),
			cg_from_parameter_type(outputPin->getType())
		);
	}

	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		in[i] = cx.emitInput(node->getInputPin(i));
		if (!in[i])
			return false;
	}

	// Define script instance.
	if (cx.getShader().defineScript(node->getName()))
	{
		StringOutputStream& fs = cx.getShader().getOutputStream(CgShader::BtScript);

		fs << L"void " << node->getName() << L"(";

		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << cg_type_name(in[i]->getType()) << L" " << node->getInputPin(i)->getName();
		}

		if (!in.empty())
			fs << L", ";

		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << L"out " << cg_type_name(out[i]->getType()) << L" " << node->getOutputPin(i)->getName();
		}

		fs << L")" << Endl;
		fs << L"{" << Endl;
		fs << IncreaseIndent;
		fs << script << Endl;
		fs << DecreaseIndent;
		fs << L"}" << Endl;
		fs << Endl;
	}

	// Emit script invocation.
	for (int32_t i = 0; i < outputPinCount; ++i)
		f << cg_type_name(out[i]->getType()) << L" " << out[i]->getName() << L";" << Endl;

	f << node->getName() << L"(";

	for (RefArray< CgVariable >::const_iterator i = in.begin(); i != in.end(); ++i)
	{
		if (i != in.begin())
			f << L", ";
		f << (*i)->getName();
	}

	if (!in.empty())
		f << L", ";

	for (RefArray< CgVariable >::const_iterator i = out.begin(); i != out.end(); ++i)
	{
		if (i != out.begin())
			f << L", ";
		f << (*i)->getName();
	}

	f << L");" << Endl;
	return true;
}

bool emitSign(CgContext& cx, Sign* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sign(" << in->getName() << L");" << Endl;
	return true;
}

bool emitSin(CgContext& cx, Sin* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != CtFloat)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	assign(f, out) << L"sin(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitSqrt(CgContext& cx, Sqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitStep(CgContext& cx, Step* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"X");
	CgVariable* in2 = cx.emitInput(node, L"Y");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"step(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitSub(CgContext& cx, Sub* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, L"Input1");
	CgVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitSum(CgContext& cx, Sum* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	CgVariable* N = cx.emitOutput(node, L"N", CtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	CgVariable* out = cx.emitOutput(node, L"Output", CtVoid);
	T_ASSERT (out);

	// Write input branch in a temporary output stream.
	StringOutputStream fs;
	cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		CgVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(CgShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	assign(f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << N->getName() << L" = " << node->getFrom() << L"; " << N->getName() << L" <= " << node->getTo() << L"; ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << fs.str();
	f << out->getName() << L" += " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;	

	return true;
}

bool emitSwizzle(CgContext& cx, Swizzle* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);

	std::wstring map = node->get();
	if (map.length() == 0)
		return false;

	const CgType types[] = { CtFloat, CtFloat2, CtFloat3, CtFloat4 };
	CgType type = types[map.length() - 1];

	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	if (
		(map == L"xyzw" && in->getType() == CtFloat4) ||
		(map == L"xyz" && in->getType() == CtFloat3) ||
		(map == L"xy" && in->getType() == CtFloat2) ||
		(map == L"x" && in->getType() == CtFloat)
	)
	{
		// No need to swizzle; pass variable further.
		cx.emitOutput(node, L"Output", in);
	}
	else
	{
		CgVariable* out = cx.emitOutput(node, L"Output", type);

		bool containConstant = false;
		for (size_t i = 0; i < map.length() && !containConstant; ++i)
		{
			if (map[i] == L'0' || map[i] == L'1')
				containConstant = true;
		}

		StringOutputStream ss;
		if (containConstant || (map.length() > 1 && in->getType() == CtFloat))
		{
			ss << cg_type_name(type) << L"(";
			for (size_t i = 0; i < map.length(); ++i)
			{
				if (i > 0)
					ss << L", ";
				switch (map[i])
				{
				case 'x':
					if (in->getType() == CtFloat)
					{
						ss << in->getName();
						break;
					}
					// Don't break, multidimensional source.
				case 'y':
				case 'z':
				case 'w':
					ss << in->getName() << L'.' << map[i];
					break;
				case '0':
					ss << L"0.0f";
					break;
				case '1':
					ss << L"1.0f";
					break;
				}
			}
			ss << L")";
		}
		else if (map.length() > 1)
		{
			ss << in->getName() << L'.';
			for (size_t i = 0; i < map.length(); ++i)
				ss << map[i];
		}
		else if (map.length() == 1)
		{
			if (map[0] == L'x' && in->getType() == CtFloat)
				ss << in->getName();
			else
				ss << in->getName() << L'.' << map[0];
		}

		assign(f, out) << ss.str() << L";" << Endl;
	}

	return true;
}

bool emitSwitch(CgContext& cx, Switch* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);

	CgVariable* in = cx.emitInput(node, L"Select");
	if (!in)
		return false;

	const std::vector< int32_t >& caseConditions = node->getCases();
	std::vector< std::wstring > caseBranches;
	RefArray< CgVariable > caseInputs;
	CgType outputType = CtVoid;

	// Conditional branches.
	for (uint32_t i = 0; i < uint32_t(caseConditions.size()); ++i)
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(i + 2);
		T_ASSERT (caseInput);

		Ref< CgVariable > caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(CgShader::BtBody);
	}

	// Default branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(CgShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(1);
		T_ASSERT (caseInput);

		Ref< CgVariable > caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(CgShader::BtBody);
	}

	// Create output variable.
	Ref< CgVariable > out = cx.emitOutput(node, L"Output", outputType);
	assign(f, out) << L"0;" << Endl;

	//if (node->getBranch() == Switch::BrStatic)
	//	f << L"[flatten]" << Endl;
	//else if (node->getBranch() == Switch::BrDynamic)
	//	f << L"[branch]" << Endl;

	for (uint32_t i = 0; i < uint32_t(caseConditions.size()); ++i)
	{
		f << (i == 0 ? L"if (" : L"else if (") << L"int(" << in->cast(CtFloat) << L") == " << caseConditions[i] << L")" << Endl;
		f << L"{" << Endl;
		f << IncreaseIndent;

		f << caseBranches[i];
		f << out->getName() << L" = " << caseInputs[i]->cast(outputType) << L";" << Endl;

		f << DecreaseIndent;
		f << L"}" << Endl;
	}

	if (!caseConditions.empty())
	{
		f << L"else" << Endl;
		f << L"{" << Endl;
		f << IncreaseIndent;
	}

	f << caseBranches.back();
	f << out->getName() << L" = " << caseInputs.back()->cast(outputType) << L";" << Endl;

	if (!caseConditions.empty())
	{
		f << DecreaseIndent;
		f << L"}" << Endl;
	}

	return true;
}

bool emitTan(CgContext& cx, Tan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != CtFloat)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat);
	assign(f, out) << L"tan(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitTargetSize(CgContext& cx, TargetSize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat2);
	assign(f, out) << L"_cg_targetSize;";
	return true;
}

bool emitTextureSize(CgContext& cx, TextureSize* node)
{
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in || in->getType() < CtTexture2D)
		return false;

	std::wstring uniformName = L"__private__" + in->getName() + L"_size";

	CgVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		uniformName,
		CtFloat3
	);

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(uniformName) == uniforms.end())
	{
		uint32_t registerIndex = cx.getShader().addUniform(uniformName, CtFloat3, 1);
		if (registerIndex == ~0U)
			return false;

		StringOutputStream& fu = cx.getShader().getOutputStream(CgShader::BtUniform);
		fu << L"uniform float3 " << uniformName << L" : register(c" << registerIndex << L");" << Endl;
	}

	return true;
}

bool emitTransform(CgContext& cx, Transform* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	CgVariable* transform = cx.emitInput(node, L"Transform");
	if (!in || !transform)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"columnMajorMul(" << transform->getName() << L", " << in->cast(CtFloat4) << L");" << Endl;
	return true;
}

bool emitTranspose(CgContext& cx, Transpose* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"transpose(" << in->getName() << L");" << Endl;
	return true;
}

bool emitTruncate(CgContext& cx, Truncate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	CgVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"trunc(" << in->getName() << L");" << Endl;
	return true;
}

bool emitUniform(CgContext& cx, Uniform* node)
{
	CgVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		cg_from_parameter_type(node->getParameterType())
	);

	if (out->getType() < CtTexture2D)
	{
		const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
		if (uniforms.find(node->getParameterName()) == uniforms.end())
		{
			uint32_t registerIndex = cx.getShader().addUniform(node->getParameterName(), out->getType(), 1);
			if (registerIndex == ~0U)
				return false;

			StringOutputStream& fu = cx.getShader().getOutputStream(CgShader::BtUniform);
			fu << L"uniform " << cg_type_name(out->getType()) << L" " << node->getParameterName() << L" : register(c" << registerIndex << L");" << Endl;
		}
	}

	return true;
}

bool emitVector(CgContext& cx, Vector* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, L"Output", CtFloat4);
	assign(f, out) << L"float4(" << node->get().x() << L", " << node->get().y() << L", " << node->get().z() << L", " << node->get().w() << L");" << Endl;
	return true;
}

bool emitVertexInput(CgContext& cx, VertexInput* node)
{
	if (!cx.inVertex())
		return false;

	CgShader& shader = cx.getShader();
	CgType type = cg_from_data_type(node->getDataType());

	// Declare input variable.
	if (!shader.haveInput(node->getName()))
	{
		std::wstring semantic = cg_semantic(node->getDataUsage(), node->getIndex());

		StringOutputStream& fi = shader.getOutputStream(CgShader::BtInput);
		fi << cg_type_name(type) << L" " << node->getName() << L" : " << semantic << L";" << Endl;

		shader.addInput(node->getName());
	}

	// Read value from input.
	if (node->getDataUsage() == DuPosition)
	{
		CgVariable* out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			CtFloat4
		);
		StringOutputStream& f = shader.getOutputStream(CgShader::BtBody);
		switch (type)
		{
		case CtFloat:
			assign(f, out) << L"float4(i." << node->getName() << L".x, 0.0f, 0.0f, 1.0f);" << Endl;
			break;

		case CtFloat2:
			assign(f, out) << L"float4(i." << node->getName() << L".xy, 0.0f, 1.0f);" << Endl;
			break;

		case CtFloat3:
			assign(f, out) << L"float4(i." << node->getName() << L".xyz, 1.0f);" << Endl;
			break;

		default:
			assign(f, out) << L"i." << node->getName() << L";" << Endl;
			break;
		}
	}
	else if (node->getDataUsage() == DuNormal)
	{
		CgVariable* out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			CtFloat4
		);
		StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
		switch (type)
		{
		case CtFloat:
			assign(f, out) << L"float4(i." << node->getName() << L".x, 0.0f, 0.0f, 0.0f);" << Endl;
			break;

		case CtFloat2:
			assign(f, out) << L"float4(i." << node->getName() << L".xy, 0.0f, 0.0f);" << Endl;
			break;

		case CtFloat3:
			assign(f, out) << L"float4(i." << node->getName() << L".xyz, 0.0f);" << Endl;
			break;

		default:
			assign(f, out) << L"i." << node->getName() << L";" << Endl;
			break;
		}
	}
	else
	{
		CgVariable* out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			type
		);
		StringOutputStream& f = cx.getShader().getOutputStream(CgShader::BtBody);
		assign(f, out) << L"i." << node->getName() << L";" << Endl;
	}

	return true;
}

bool emitVertexOutput(CgContext& cx, VertexOutput* node)
{
	cx.enterVertex();
	CgVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	StringOutputStream& fo = cx.getVertexShader().getOutputStream(CgShader::BtOutput);
	fo << L"float4 Position : POSITION0;" << Endl;

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(CgShader::BtBody);
	switch (in->getType())
	{
	case CtFloat:
		fb << L"o.Position = float4(" << in->getName() << L".x, 0.0f, 0.0f, 1.0f);" << Endl;
		break;

	case CtFloat2:
		fb << L"o.Position = float4(" << in->getName() << L".xy, 0.0f, 1.0f);" << Endl;
		break;

	case CtFloat3:
		fb << L"o.Position = float4(" << in->getName() << L".xyz, 1.0f);" << Endl;
		break;

	case CtFloat4:
		fb << L"o.Position = " << in->getName() << L";" << Endl;
		break;
	}

	return true;
}

struct Emitter
{
	virtual ~Emitter() {}

	virtual bool emit(CgContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public Emitter
{
	typedef bool (*function_t)(CgContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function)
	:	m_function(function)
	{
	}

	virtual bool emit(CgContext& c, Node* node)
	{
		T_ASSERT (is_a< NodeType >(node));
		return (*m_function)(c, static_cast< NodeType* >(node));
	}
};

		}

CgEmitter::CgEmitter()
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
	m_emitters[&type_of< Instance >()] = new EmitterCast< Instance >(emitInstance);
	m_emitters[&type_of< Interpolator >()] = new EmitterCast< Interpolator >(emitInterpolator);
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
	m_emitters[&type_of< Polynomial >()] = new EmitterCast< Polynomial >(emitPolynomial);
	m_emitters[&type_of< Pow >()] = new EmitterCast< Pow >(emitPow);
	m_emitters[&type_of< PixelOutput >()] = new EmitterCast< PixelOutput >(emitPixelOutput);
	m_emitters[&type_of< RecipSqrt >()] = new EmitterCast< RecipSqrt >(emitRecipSqrt);
	m_emitters[&type_of< Reflect >()] = new EmitterCast< Reflect >(emitReflect);
	m_emitters[&type_of< Repeat >()] = new EmitterCast< Repeat >(emitRepeat);
	m_emitters[&type_of< Round >()] = new EmitterCast< Round >(emitRound);
	m_emitters[&type_of< Sampler >()] = new EmitterCast< Sampler >(emitSampler);
	m_emitters[&type_of< Script >()] = new EmitterCast< Script >(emitScript);
	m_emitters[&type_of< Scalar >()] = new EmitterCast< Scalar >(emitScalar);
	m_emitters[&type_of< Sign >()] = new EmitterCast< Sign >(emitSign);
	m_emitters[&type_of< Sin >()] = new EmitterCast< Sin >(emitSin);
	m_emitters[&type_of< Sqrt >()] = new EmitterCast< Sqrt >(emitSqrt);
	m_emitters[&type_of< Step >()] = new EmitterCast< Step >(emitStep);
	m_emitters[&type_of< Sub >()] = new EmitterCast< Sub >(emitSub);
	m_emitters[&type_of< Sum >()] = new EmitterCast< Sum >(emitSum);
	m_emitters[&type_of< Swizzle >()] = new EmitterCast< Swizzle >(emitSwizzle);
	m_emitters[&type_of< Switch >()] = new EmitterCast< Switch >(emitSwitch);
	m_emitters[&type_of< Tan >()] = new EmitterCast< Tan >(emitTan);
	m_emitters[&type_of< TargetSize >()] = new EmitterCast< TargetSize >(emitTargetSize);
	m_emitters[&type_of< TextureSize >()] = new EmitterCast< TextureSize >(emitTextureSize);
	m_emitters[&type_of< Transform >()] = new EmitterCast< Transform >(emitTransform);
	m_emitters[&type_of< Transpose >()] = new EmitterCast< Transpose >(emitTranspose);
	m_emitters[&type_of< Truncate >()] = new EmitterCast< Truncate >(emitTruncate);
	m_emitters[&type_of< Uniform >()] = new EmitterCast< Uniform >(emitUniform);
	m_emitters[&type_of< Vector >()] = new EmitterCast< Vector >(emitVector);
	m_emitters[&type_of< VertexInput >()] = new EmitterCast< VertexInput >(emitVertexInput);
	m_emitters[&type_of< VertexOutput >()] = new EmitterCast< VertexOutput >(emitVertexOutput);
}

CgEmitter::~CgEmitter()
{
	for (std::map< const TypeInfo*, Emitter* >::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
		delete i->second;
}

bool CgEmitter::emit(CgContext& c, Node* node)
{
	// Find emitter for node.
	std::map< const TypeInfo*, Emitter* >::iterator i = m_emitters.find(&type_of(node));
	if (i == m_emitters.end())
	{
		log::error << L"No emitter for node " << type_name(node) << Endl;
		return false;
	}

	// Emit HLSL code.
	T_ASSERT (i->second);
	if (!i->second->emit(c, node))
	{
		log::error << L"Failed to emit " << type_name(node) << Endl;
		return false;
	}

	return true;
}

	}
}
