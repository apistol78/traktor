/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Render/VertexElement.h"
//#include "Render/Ps4/Platform.h"
#include "Render/Ps4/PsslContext.h"
#include "Render/Ps4/PsslEmitter.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/Script.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

StringOutputStream& assign(PsslContext& cx, StringOutputStream& f, PsslVariable* out)
{
	f << pssl_type_name(out->getType(), cx.inPixel()) << L" " << out->getName() << L" = ";
	return f;
}

bool emitAbs(PsslContext& cx, Abs* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"abs(" << in->getName() << L");" << Endl;
	return true;
}

bool emitAdd(PsslContext& cx, Add* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitArcusCos(PsslContext& cx, ArcusCos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != PsslFloat)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"acos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitArcusTan(PsslContext& cx, ArcusTan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* xy = cx.emitInput(node, L"XY");
	if (!xy || xy->getType() != PsslFloat2)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"atan2(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;
	return true;
}

bool emitClamp(PsslContext& cx, Clamp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	if (node->getMin() == 0.0f && node->getMax() == 1.0f)
		assign(cx, f, out) << L"saturate(" << in->getName() << L");" << Endl;
	else
		assign(cx, f, out) << L"clamp(" << in->getName() << L", " << node->getMin() << L", " << node->getMax() << L");" << Endl;
	return true;
}

bool emitColor(PsslContext& cx, Color* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat4);
	if (!out)
		return false;
	traktor::Color4ub color = node->getColor();
	f << L"const float4 " << out->getName() << L" = float4(" << (color.r / 255.0f) << L", " << (color.g / 255.0f) << L", " << (color.b / 255.0f) << L", " << (color.a / 255.0f) << L");" << Endl;
	return true;
}

bool emitConditional(PsslContext& cx, Conditional* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	// Emit input and reference branches.
	PsslVariable* in = cx.emitInput(node, L"Input");
	PsslVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	PsslVariable caseTrue, caseFalse;
	std::wstring caseTrueBranch, caseFalseBranch;

	// Find common output pins from both sides of branch;
	// emit those before condition in order to have them evaluated outside of conditional.
	std::vector< const OutputPin* > outputPins;
	cx.findCommonOutputs(node, L"CaseTrue", L"CaseFalse", outputPins);
	for (std::vector< const OutputPin* >::const_iterator i = outputPins.begin(); i != outputPins.end(); ++i)
		cx.emit((*i)->getNode());

	// Emit true branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
		cx.getShader().pushScope();

		PsslVariable* ct = cx.emitInput(node, L"CaseTrue");
		if (!ct)
			return false;

		caseTrue = *ct;
		caseTrueBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(PsslShader::BtBody);
	}

	// Emit false branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
		cx.getShader().pushScope();

		PsslVariable* cf = cx.emitInput(node, L"CaseFalse");
		if (!cf)
			return false;

		caseFalse = *cf;
		caseFalseBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(PsslShader::BtBody);
	}

	// Create output variable.
	PsslType outputType = std::max< PsslType >(caseTrue.getType(), caseFalse.getType());
	
	PsslVariable* out = cx.emitOutput(node, L"Output", outputType);
	f << pssl_type_name(out->getType(), cx.inPixel()) << L" " << out->getName() << L";" << Endl;

	if (node->getBranch() == Conditional::BrStatic)
		f << L"[flatten]" << Endl;
	else if (node->getBranch() == Conditional::BrDynamic)
		f << L"[branch]" << Endl;

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

bool emitCos(PsslContext& cx, Cos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != PsslFloat)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"cos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitCross(PsslContext& cx, Cross* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat3);
	assign(cx, f, out) << L"cross(" << in1->cast(PsslFloat3) << L", " << in2->cast(PsslFloat3) << L");" << Endl;
	return true;
}

bool emitDerivative(PsslContext& cx, Derivative* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* input = cx.emitInput(node, L"Input");
	if (!input)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", input->getType());
	switch (node->getAxis())
	{
	case Derivative::DaX:
		assign(cx, f, out) << L"ddx(" << input->getName() << L");" << Endl;
		break;
	case Derivative::DaY:
		assign(cx, f, out) << L"ddy(" << input->getName() << L");" << Endl;
		break;
	default:
		return false;
	}
	return true;
}

bool emitDiv(PsslContext& cx, Div* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitDiscard(PsslContext& cx, Discard* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	// Emit input and reference branches.
	PsslVariable* in = cx.emitInput(node, L"Input");
	PsslVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	f << L"[branch]" << Endl;

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

	PsslVariable* pass = cx.emitInput(node, L"Pass");
	if (!pass)
		return false;

	PsslVariable* out = cx.emitOutput(node, L"Output", pass->getType());
	assign(cx, f, out) << pass->getName() << L";" << Endl;

	return true;
}

bool emitDot(PsslContext& cx, Dot* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	assign(cx, f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitExp(PsslContext& cx, Exp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"exp(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFraction(PsslContext& cx, Fraction* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"frac(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFragmentPosition(PsslContext& cx, FragmentPosition* node)
{
	if (!cx.inPixel())
		return false;

	cx.getShader().allocateVPos();

	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat2);
	assign(cx, f, out) << L"vPos;" << Endl;

	return true;
}

bool emitFrontFace(PsslContext& cx, FrontFace* node)
{
	if (!cx.inPixel())
		return false;

	cx.getShader().allocateVFace();

	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"vFace ? 1.0f : 0.0f;" << Endl;

	return true;
}

bool emitIndexedUniform(PsslContext& cx, IndexedUniform* node)
{
	PsslVariable* index = cx.emitInput(node, L"Index");
	if (!index)
		return false;

	PsslVariable* out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		pssl_from_parameter_type(node->getParameterType())
	);

	StringOutputStream& fb = cx.getShader().getOutputStream(PsslShader::BtBody);
	assign(cx, fb, out) << node->getParameterName() << L"[" << index->getName() << L"];" << Endl;

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		const PsslShader::BlockType c_blockType[] = { PsslShader::BtCBufferOnce, PsslShader::BtCBufferFrame, PsslShader::BtCBufferDraw };
		StringOutputStream& fu = cx.getShader().getOutputStream(c_blockType[node->getFrequency()]);
		fu << pssl_type_name(out->getType(), false) << L" " << node->getParameterName() << L"[" << node->getLength() << L"];" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}

	return true;
}

bool emitInstance(PsslContext& cx, Instance* node)
{
	cx.getShader().allocateInstanceID();

	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"float(instanceID);" << Endl;

	return true;
}

bool emitInterpolator(PsslContext& cx, Interpolator* node)
{
	if (!cx.inPixel())
	{
		// We're already in vertex state; skip interpolation.
		PsslVariable* in = cx.emitInput(node, L"Input");
		if (!in)
			return false;

		PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());

		StringOutputStream& fb = cx.getShader().getOutputStream(PsslShader::BtBody);
		assign(cx, fb, out) << in->getName() << L";" << Endl;

		return true;
	}

	cx.enterVertex();

	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	cx.enterPixel();

	int32_t interpolatorWidth = pssl_type_width(in->getType());
	if (!interpolatorWidth)
		return false;

	int32_t interpolatorId;
	int32_t interpolatorOffset;

	bool declare = cx.allocateInterpolator(interpolatorWidth, interpolatorId, interpolatorOffset);

	std::wstring interpolatorName = L"Attr" + toString(interpolatorId);
	std::wstring interpolatorMask = interpolatorName + L"." + std::wstring(L"xyzw").substr(interpolatorOffset, interpolatorWidth);

	StringOutputStream& vfb = cx.getVertexShader().getOutputStream(PsslShader::BtBody);
	vfb << L"o." << interpolatorMask << L" = " << in->getName() << L";" << Endl;

	cx.getPixelShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		L"i." + interpolatorMask,
		in->getType()
	);

	if (declare)
	{
		StringOutputStream& vfo = cx.getVertexShader().getOutputStream(PsslShader::BtOutput);
		vfo << L"float4 " << interpolatorName << L" : TEXCOORD" << interpolatorId << L";" << Endl;

		StringOutputStream& pfi = cx.getPixelShader().getOutputStream(PsslShader::BtInput);
		pfi << L"float4 " << interpolatorName << L" : TEXCOORD" << interpolatorId << L";" << Endl;
	}

	return true;
}

bool emitIterate(PsslContext& cx, Iterate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	PsslVariable* N = cx.emitOutput(node, L"N", PsslFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslVoid);
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
	cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		PsslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		// Emit post condition if connected; break iteration if condition is false.
		PsslVariable* condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (!(bool)" << condition->cast(PsslFloat) << L")" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(PsslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	PsslVariable* initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(cx, f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(cx, f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	if (cx.inPixel())
		f << L"[unroll]" << Endl;
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

bool emitIterate2d(PsslContext& cx, Iterate2d* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	std::wstring inputName;

	// Create iterator variables.
	PsslVariable* X = cx.emitOutput(node, L"X", PsslFloat);
	T_ASSERT (X);

	PsslVariable* Y = cx.emitOutput(node, L"Y", PsslFloat);
	T_ASSERT (Y);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslVoid);
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
	cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
	cx.getShader().pushScope();

	PsslVariable* input = cx.emitInput(node, L"Input");
	if (!input)
		return false;

	// Emit post condition if connected; break iteration if condition is false.
	PsslVariable* condition = cx.emitInput(node, L"Condition");
	if (condition)
	{
		fs << L"if (!(bool)" << condition->cast(PsslFloat) << L")" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	inputName = input->getName();

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	out->setType(input->getType());

	cx.getShader().popScope();
	cx.getShader().popOutputStream(PsslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	PsslVariable* initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(cx, f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(cx, f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	if (cx.inPixel())
		f << L"[unroll]" << Endl;
	f << L"for (float " << X->getName() << L" = " << node->getFromX() << L"; " << X->getName() << L" <= " << node->getToX() << L"; ++" << X->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	if (cx.inPixel())
		f << L"[unroll]" << Endl;
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
		fs << L"if (!(bool)" << condition->cast(PsslFloat) << L")" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	f << DecreaseIndent;
	f << L"}" << Endl;	

	return true;
}

bool emitLength(PsslContext& cx, Length* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"length(" << in->getName() << L");" << Endl;
	return true;
}

bool emitLerp(PsslContext& cx, Lerp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* blend = cx.emitInput(node, L"Blend");
	if (!blend || blend->getType() != PsslFloat)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in1->getType());
	assign(cx, f, out) << L"lerp(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;
	return true;
}

bool emitLog(PsslContext& cx, Log* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	switch (node->getBase())
	{
	case Log::LbTwo:
		assign(cx, f, out) << L"log2(" << in->getName() << L");" << Endl;
		break;

	case Log::LbTen:
		assign(cx, f, out) << L"log10(" << in->getName() << L");" << Endl;
		break;

	case Log::LbNatural:
		assign(cx, f, out) << L"log(" << in->getName() << L");" << Endl;
		break;
	}
	return true;
}

bool emitMatrixIn(PsslContext& cx, MatrixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* xaxis = cx.emitInput(node, L"XAxis");
	PsslVariable* yaxis = cx.emitInput(node, L"YAxis");
	PsslVariable* zaxis = cx.emitInput(node, L"ZAxis");
	PsslVariable* translate = cx.emitInput(node, L"Translate");
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat4x4);
	assign(cx, f, out) << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;
	f << (xaxis     ? xaxis->cast(PsslFloat4)     : L"1.0f, 0.0f, 0.0f, 0.0f") << L"," << Endl;
	f << (yaxis     ? yaxis->cast(PsslFloat4)     : L"0.0f, 1.0f, 0.0f, 0.0f") << L"," << Endl;
	f << (zaxis     ? zaxis->cast(PsslFloat4)     : L"0.0f, 0.0f, 1.0f, 0.0f") << L"," << Endl;
	f << (translate ? translate->cast(PsslFloat4) : L"0.0f, 0.0f, 0.0f, 1.0f") << Endl;
	f << DecreaseIndent;
	f << L"};" << Endl;
	return true;
}

bool emitMatrixOut(PsslContext& cx, MatrixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* xaxis = cx.emitOutput(node, L"XAxis", PsslFloat4);
	if (xaxis)
		assign(cx, f, xaxis) << in->getName() << L"._11_21_31_41;" << Endl;
	PsslVariable* yaxis = cx.emitOutput(node, L"YAxis", PsslFloat4);
	if (yaxis)
		assign(cx, f, yaxis) << in->getName() << L"._12_22_32_42;" << Endl;
	PsslVariable* zaxis = cx.emitOutput(node, L"ZAxis", PsslFloat4);
	if (zaxis)
		assign(cx, f, zaxis) << in->getName() << L"._13_23_33_43;" << Endl;
	PsslVariable* translate = cx.emitOutput(node, L"Translate", PsslFloat4);
	if (translate)
		assign(cx, f, translate) << in->getName() << L"._14_24_34_44;" << Endl;
	return true;
}

bool emitMax(PsslContext& cx, Max* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << L"max(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMin(PsslContext& cx, Min* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMixIn(PsslContext& cx, MixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* x = cx.emitInput(node, L"X");
	PsslVariable* y = cx.emitInput(node, L"Y");
	PsslVariable* z = cx.emitInput(node, L"Z");
	PsslVariable* w = cx.emitInput(node, L"W");

	if (!y && !z && !w)
	{
		PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
		assign(cx, f, out) << L"float(" << (x ? x->getName() : L"0.0f") << L");" << Endl;
	}
	else if (!z && !w)
	{
		PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat2);
		assign(cx, f, out) << L"float2(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L");" << Endl;
	}
	else if (!w)
	{
		PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat3);
		assign(cx, f, out) << L"float3(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L", " << (z ? z->getName() : L"0.0f") << L");" << Endl;
	}
	else
	{
		PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat4);
		assign(cx, f, out) << L"float4(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L", " << (z ? z->getName() : L"0.0f") << L", " << (w ? w->getName() : L"0.0f") << L");" << Endl;
	}

	return true;
}

bool emitMixOut(PsslContext& cx, MixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	switch (in->getType())
	{
	case PsslFloat:
		{
			PsslVariable* x = cx.emitOutput(node, L"X", PsslFloat);
			assign(cx, f, x) << in->getName() << L".x;" << Endl;
		}
		break;

	case PsslFloat2:
		{
			PsslVariable* x = cx.emitOutput(node, L"X", PsslFloat);
			PsslVariable* y = cx.emitOutput(node, L"Y", PsslFloat);
			assign(cx, f, x) << in->getName() << L".x;" << Endl;
			assign(cx, f, y) << in->getName() << L".y;" << Endl;
		}
		break;

	case PsslFloat3:
		{
			PsslVariable* x = cx.emitOutput(node, L"X", PsslFloat);
			PsslVariable* y = cx.emitOutput(node, L"Y", PsslFloat);
			PsslVariable* z = cx.emitOutput(node, L"Z", PsslFloat);
			assign(cx, f, x) << in->getName() << L".x;" << Endl;
			assign(cx, f, y) << in->getName() << L".y;" << Endl;
			assign(cx, f, z) << in->getName() << L".z;" << Endl;
		}
		break;

	case PsslFloat4:
		{
			PsslVariable* x = cx.emitOutput(node, L"X", PsslFloat);
			PsslVariable* y = cx.emitOutput(node, L"Y", PsslFloat);
			PsslVariable* z = cx.emitOutput(node, L"Z", PsslFloat);
			PsslVariable* w = cx.emitOutput(node, L"W", PsslFloat);
			assign(cx, f, x) << in->getName() << L".x;" << Endl;
			assign(cx, f, y) << in->getName() << L".y;" << Endl;
			assign(cx, f, z) << in->getName() << L".z;" << Endl;
			assign(cx, f, w) << in->getName() << L".w;" << Endl;
		}
		break;

	default:
		return false;
	}

	return true;
}

bool emitMul(PsslContext& cx, Mul* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitMulAdd(PsslContext& cx, MulAdd* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	PsslVariable* in3 = cx.emitInput(node, L"Input3");
	if (!in1 || !in2 || !in3)
		return false;
	PsslType type = std::max< PsslType >(std::max< PsslType >(in1->getType(), in2->getType()), in3->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << in1->cast(type) << L" * " << in2->cast(type) << L" + " << in3->cast(type) << L";" << Endl;
	return true;
}

bool emitNeg(PsslContext& cx, Neg* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"-" << in->getName() << L";" << Endl;
	return true;
}

bool emitNormalize(PsslContext& cx, Normalize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"normalize(" << in->getName() << L");" << Endl;
	return true;
}

bool emitPixelOutput(PsslContext& cx, PixelOutput* node)
{
	//const D3D11_CULL_MODE d3dCullMode[] =
	//{
	//	D3D11_CULL_NONE,
	//	D3D11_CULL_FRONT,
	//	D3D11_CULL_BACK
	//};

	//const D3D11_BLEND_OP d3dBlendOperation[] =
	//{
	//	D3D11_BLEND_OP_ADD,
	//	D3D11_BLEND_OP_SUBTRACT,
	//	D3D11_BLEND_OP_REV_SUBTRACT,
	//	D3D11_BLEND_OP_MIN,
	//	D3D11_BLEND_OP_MAX
	//};

	//const D3D11_BLEND d3dBlendFactor[] =
	//{
	//	D3D11_BLEND_ONE,
	//	D3D11_BLEND_ZERO,
	//	D3D11_BLEND_SRC_COLOR,
	//	D3D11_BLEND_INV_SRC_COLOR,
	//	D3D11_BLEND_DEST_COLOR,
	//	D3D11_BLEND_INV_DEST_COLOR,
	//	D3D11_BLEND_SRC_ALPHA,
	//	D3D11_BLEND_INV_SRC_ALPHA,
	//	D3D11_BLEND_DEST_ALPHA,
	//	D3D11_BLEND_INV_DEST_ALPHA
	//};

	//const D3D11_COMPARISON_FUNC d3dCompareFunction[] =
	//{
	//	D3D11_COMPARISON_ALWAYS,
	//	D3D11_COMPARISON_NEVER,
	//	D3D11_COMPARISON_LESS,
	//	D3D11_COMPARISON_LESS_EQUAL,
	//	D3D11_COMPARISON_GREATER,
	//	D3D11_COMPARISON_GREATER_EQUAL,
	//	D3D11_COMPARISON_EQUAL,
	//	D3D11_COMPARISON_NOT_EQUAL
	//};

	//const D3D11_STENCIL_OP d3dStencilOperation[] =
	//{
	//	D3D11_STENCIL_OP_KEEP,
	//	D3D11_STENCIL_OP_ZERO,
	//	D3D11_STENCIL_OP_REPLACE,
	//	D3D11_STENCIL_OP_INCR_SAT,
	//	D3D11_STENCIL_OP_DECR_SAT,
	//	D3D11_STENCIL_OP_INVERT,
	//	D3D11_STENCIL_OP_INCR,
	//	D3D11_STENCIL_OP_DECR
	//};

	RenderState rs = node->getRenderState();

	cx.enterPixel();

	const wchar_t* inputs[] = { L"Input", L"Input1", L"Input2", L"Input3" };
	PsslVariable* in[4];

	for (int32_t i = 0; i < sizeof_array(in); ++i)
		in[i] = cx.emitInput(node, inputs[i]);

	if (!in[0])
		return false;

	for (int32_t i = 0; i < sizeof_array(in); ++i)
	{
		if (!in[i])
			continue;

		StringOutputStream& fpo = cx.getPixelShader().getOutputStream(PsslShader::BtOutput);
		fpo << L"half4 Color" << i << L" : SV_Target" << i << L";" << Endl;

		StringOutputStream& fpb = cx.getPixelShader().getOutputStream(PsslShader::BtBody);
		fpb << L"half4 out_Color" << i << L" = " << in[i]->cast(PsslFloat4) << L";" << Endl;

		// Emulate old fashion alpha test through "discard" instruction.
		if (i == 0 && rs.alphaTestEnable)
		{
			float alphaRef = rs.alphaTestReference / 255.0f;

			if (rs.alphaTestFunction == CfLess)
				fpb << L"if (out_Color" << i << L".w >= " << alphaRef << L")" << Endl;
			else if (rs.alphaTestFunction == CfLessEqual)
				fpb << L"if (out_Color" << i << L".w > " << alphaRef << L")" << Endl;
			else if (rs.alphaTestFunction == CfGreater)
				fpb << L"if (out_Color" << i << L".w <= " << alphaRef << L")" << Endl;
			else if (rs.alphaTestFunction == CfGreaterEqual)
				fpb << L"if (out_Color" << i << L".w < " << alphaRef << L")" << Endl;
			else if (rs.alphaTestFunction == CfEqual)
				fpb << L"if (out_Color" << i << L".w != " << alphaRef << L")" << Endl;
			else if (rs.alphaTestFunction == CfNotEqual)
				fpb << L"if (out_Color" << i << L".w == " << alphaRef << L")" << Endl;
			else
				return false;

			fpb << L"\tdiscard;" << Endl;
		}

		fpb << L"o.Color" << i << L" = out_Color" << i << L";" << Endl;
	}

	//cx.getD3DRasterizerDesc().FillMode = rs.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	//cx.getD3DRasterizerDesc().CullMode = d3dCullMode[rs.cullMode];

	//cx.getD3DDepthStencilDesc().DepthEnable = rs.depthEnable ? TRUE : FALSE;
	//cx.getD3DDepthStencilDesc().DepthWriteMask = rs.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	//cx.getD3DDepthStencilDesc().DepthFunc = rs.depthEnable ? d3dCompareFunction[rs.depthFunction] : D3D11_COMPARISON_ALWAYS;

	//if (rs.stencilEnable)
	//{
	//	cx.getD3DDepthStencilDesc().StencilEnable = TRUE;
	//	cx.getD3DDepthStencilDesc().StencilReadMask = 0xff;
	//	cx.getD3DDepthStencilDesc().StencilWriteMask = 0xff;
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilFailOp = d3dStencilOperation[rs.stencilFail];
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilDepthFailOp = d3dStencilOperation[rs.stencilZFail];
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilPassOp = d3dStencilOperation[rs.stencilPass];
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilFunc = d3dCompareFunction[rs.stencilFunction];
	//	cx.getD3DDepthStencilDesc().BackFace.StencilFailOp = d3dStencilOperation[rs.stencilFail];
	//	cx.getD3DDepthStencilDesc().BackFace.StencilDepthFailOp = d3dStencilOperation[rs.stencilZFail];
	//	cx.getD3DDepthStencilDesc().BackFace.StencilPassOp = d3dStencilOperation[rs.stencilPass];
	//	cx.getD3DDepthStencilDesc().BackFace.StencilFunc = d3dCompareFunction[rs.stencilFunction];
	//	cx.setStencilReference(rs.stencilReference);
	//}
	//else
	//{
	//	cx.getD3DDepthStencilDesc().StencilEnable = FALSE;
	//	cx.getD3DDepthStencilDesc().StencilReadMask = 0xff;
	//	cx.getD3DDepthStencilDesc().StencilWriteMask = 0xff;
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//	cx.getD3DDepthStencilDesc().FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	//	cx.getD3DDepthStencilDesc().BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//	cx.getD3DDepthStencilDesc().BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	//	cx.getD3DDepthStencilDesc().BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//	cx.getD3DDepthStencilDesc().BackFace.StencilFunc =D3D11_COMPARISON_ALWAYS;
	//	cx.setStencilReference(0);
	//}

	//cx.getD3DBlendDesc().AlphaToCoverageEnable = rs.alphaToCoverageEnable ? TRUE : FALSE;

	//if (rs.blendEnable)
	//{
	//	cx.getD3DBlendDesc().RenderTarget[0].BlendEnable = TRUE;
	//	cx.getD3DBlendDesc().RenderTarget[0].SrcBlend = d3dBlendFactor[rs.blendSource];
	//	cx.getD3DBlendDesc().RenderTarget[0].DestBlend = d3dBlendFactor[rs.blendDestination];
	//	cx.getD3DBlendDesc().RenderTarget[0].BlendOp = d3dBlendOperation[rs.blendOperation];
	//}
	//else
	//{
	//	cx.getD3DBlendDesc().RenderTarget[0].BlendEnable = FALSE;
	//	cx.getD3DBlendDesc().RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	//	cx.getD3DBlendDesc().RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	//	cx.getD3DBlendDesc().RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//}

	//UINT8 d3dWriteMask = 0;
	//if (rs.colorWriteMask & CwRed)
	//	d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;
	//if (rs.colorWriteMask & CwGreen)
	//	d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
	//if (rs.colorWriteMask & CwBlue)	
	//	d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
	//if (rs.colorWriteMask & CwAlpha)
	//	d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

	//for (int32_t i = 0; i < sizeof_array(in); ++i)
	//{
	//	if (in[i])
	//		cx.getD3DBlendDesc().RenderTarget[i].RenderTargetWriteMask = d3dWriteMask;
	//}

	return true;
}

bool emitPolynomial(PsslContext& cx, Polynomial* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	PsslVariable* x = cx.emitInput(node, L"X");
	PsslVariable* coeffs = cx.emitInput(node, L"Coefficients");
	if (!x || !coeffs)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);

	assign(cx, f, out);
	switch (coeffs->getType())
	{
	case PsslFloat:
		f << coeffs->getName() << L".x * " << x->getName();
		break;
	case PsslFloat2:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".y * " << x->getName();
		break;
	case PsslFloat3:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".z * " << x->getName();
		break;
	case PsslFloat4:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 4) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".z * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".w * " << x->getName();
		break;
	default:
		return false;
	}
	f << L";" << Endl;

	return true;
}

bool emitPow(PsslContext& cx, Pow* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* exponent = cx.emitInput(node, L"Exponent");
	
	const Node* inputNode = cx.getInputNode(node, L"Input");
	if (!inputNode)
		return false;

	if (const Scalar* inputScalar = dynamic_type_cast< const Scalar* >(inputNode))
	{
		if (abs(inputScalar->get() - 2.0f) < FUZZY_EPSILON)
		{
			// 2 as base; emit exp2 intrinsic instead of pow as it's more efficient.
			PsslType type = std::max< PsslType >(exponent->getType(), PsslFloat);
			PsslVariable* out = cx.emitOutput(node, L"Output", type);
			assign(cx, f, out) << L"exp2(" << exponent->cast(type) << L");" << Endl;
			return true;
		}
		else if (abs(inputScalar->get() - 2.718f) < FUZZY_EPSILON)
		{
			// e as base; emit exp intrinsic instead of pow as it's more efficient.
			PsslType type = std::max< PsslType >(exponent->getType(), PsslFloat);
			PsslVariable* out = cx.emitOutput(node, L"Output", type);
			assign(cx, f, out) << L"exp(" << exponent->cast(type) << L");" << Endl;
			return true;
		}
	}

	// Non-trivial base.	
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!exponent || !in)
		return false;

	PsslType type = std::max< PsslType >(exponent->getType(), in->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << L"pow(" << in->cast(type) << L", " << exponent->cast(type) << L");" << Endl;
	return true;
}

bool emitReflect(PsslContext& cx, Reflect* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* normal = cx.emitInput(node, L"Normal");
	PsslVariable* direction = cx.emitInput(node, L"Direction");
	if (!normal || !direction)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", direction->getType());
	assign(cx, f, out) << L"reflect(" << direction->getName() << L", " << normal->cast(direction->getType()) << L");" << Endl;
	return true;
}

bool emitRecipSqrt(PsslContext& cx, RecipSqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"rsqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitRepeat(PsslContext& cx, Repeat* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	PsslVariable* N = cx.emitOutput(node, L"N", PsslFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslVoid);
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
	cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		// Emit pre-condition, break iteration if condition is false.
		PsslVariable* condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (!(bool)" << condition->cast(PsslFloat) << L")" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		PsslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(PsslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	PsslVariable* initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(cx, f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(cx, f, out) << L"0;" << Endl;

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

bool emitRound(PsslContext& cx, Round* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"round(" << in->getName() << L");" << Endl;
	return true;
}

bool emitSampler(PsslContext& cx, Sampler* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	PsslVariable* texture = cx.emitInput(node, L"Texture");
	if (!texture || texture->getType() < PsslTexture2D)
		return false;

	PsslVariable* texCoord = cx.emitInput(node, L"TexCoord");
	if (!texCoord)
		return false;

	PsslVariable* mip = cx.emitInput(node, L"Mip");

	// Define sampler class.
	//const D3D11_TEXTURE_ADDRESS_MODE c_d3dAddress[] =
	//{
	//	D3D11_TEXTURE_ADDRESS_WRAP,
	//	D3D11_TEXTURE_ADDRESS_MIRROR,
	//	D3D11_TEXTURE_ADDRESS_CLAMP,
	//	D3D11_TEXTURE_ADDRESS_BORDER
	//};

	//const D3D11_COMPARISON_FUNC c_d3dComparison[] =
	//{
	//	D3D11_COMPARISON_ALWAYS,
	//	D3D11_COMPARISON_NEVER,
	//	D3D11_COMPARISON_LESS,
	//	D3D11_COMPARISON_LESS_EQUAL,
	//	D3D11_COMPARISON_GREATER,
	//	D3D11_COMPARISON_GREATER_EQUAL,
	//	D3D11_COMPARISON_EQUAL,
	//	D3D11_COMPARISON_NOT_EQUAL,
	//	D3D11_COMPARISON_NEVER
	//};

	const SamplerState& samplerState = node->getSamplerState();

	//D3D11_SAMPLER_DESC dsd;
	//std::memset(&dsd, 0, sizeof(dsd));

	//dsd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	//dsd.AddressU = c_d3dAddress[samplerState.addressU];
	//dsd.AddressV = c_d3dAddress[samplerState.addressV];
	//if (texture->getType() > PsslTexture2D)
	//	dsd.AddressW = c_d3dAddress[samplerState.addressW];
	//else
	//	dsd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//dsd.MipLODBias = samplerState.mipBias;
	//dsd.MaxAnisotropy = 1;
	//dsd.ComparisonFunc = c_d3dComparison[samplerState.compare];
	//dsd.BorderColor[0] =
	//dsd.BorderColor[1] =
	//dsd.BorderColor[2] =
	//dsd.BorderColor[3] = 1.0f;
	//dsd.MinLOD = -D3D11_FLOAT32_MAX;
	//dsd.MaxLOD =  D3D11_FLOAT32_MAX;

	//switch (samplerState.mipFilter)
	//{
	//case FtPoint:
	//	break;
	//case FtLinear:
	//	(UINT&)dsd.Filter |= 0x1;
	//	break;
	//}

	//switch (samplerState.magFilter)
	//{
	//case FtPoint:
	//	break;
	//case FtLinear:
	//	(UINT&)dsd.Filter |= 0x4;
	//	break;
	//}

	//switch (samplerState.minFilter)
	//{
	//case FtPoint:
	//	break;
	//case FtLinear:
	//	(UINT&)dsd.Filter |= 0x10;
	//	break;
	//}

	//if (samplerState.compare == CfNone)
	//{
	//	if (
	//		samplerState.useAnisotropic &&
	//		dsd.Filter == (0x10 | 0x4 | 0x1)
	//	)
	//		dsd.Filter = D3D11_FILTER_ANISOTROPIC;
	//}
	//else
	//	(UINT&)dsd.Filter |= 0x80;

	Adler32 samplerHash;
	samplerHash.begin();
	//samplerHash.feed(&dsd, sizeof(dsd));
	samplerHash.end();

	std::wstring samplerName = L"S" + toString(samplerHash.get()) + L"_samplerState";
	std::wstring textureName = texture->getName();

	//const std::map< std::wstring, D3D11_SAMPLER_DESC >& samplers = cx.getShader().getSamplers();
	//if (samplers.find(samplerName) == samplers.end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(PsslShader::BtSamplers);

		if (samplerState.compare == CfNone)
			fu << L"SamplerState " << samplerName << L";" << Endl;
		else
			fu << L"SamplerComparisonState " << samplerName << L";" << Endl;

		//cx.getShader().addSampler(samplerName, dsd);
	}

	PsslVariable* out = cx.emitOutput(node, L"Output", (samplerState.compare == CfNone) ? PsslFloat4 : PsslFloat);

	if (!mip && cx.inPixel() && !samplerState.ignoreMips)
	{
		if (samplerState.compare == CfNone)
		{
			switch (texture->getType())
			{
			case PsslTexture2D:
				assign(cx, f, out) << textureName << L".Sample(" << samplerName << L", " << texCoord->cast(PsslFloat2) << L");" << Endl;
				break;
			case PsslTexture3D:
			case PsslTextureCube:
				assign(cx, f, out) << textureName << L".Sample(" << samplerName << L", " << texCoord->cast(PsslFloat3) << L");" << Endl;
				break;
			default:
				return false;
			}
		}
		else
		{
			if (!samplerState.ignoreMips)
			{
				switch (texture->getType())
				{
				case PsslTexture2D:
					assign(cx, f, out) << textureName << L".SampleCmp(" << samplerName << L", " << texCoord->cast(PsslFloat2) << L", " << texCoord->cast(PsslFloat3) << L".z);" << Endl;
					break;
				case PsslTexture3D:
				case PsslTextureCube:
					assign(cx, f, out) << textureName << L".SampleCmp(" << samplerName << L", " << texCoord->cast(PsslFloat3) << L", " << texCoord->cast(PsslFloat4) << L".w);" << Endl;
					break;
				default:
					return false;
				}
			}
			else
			{
				switch (texture->getType())
				{
				case PsslTexture2D:
					assign(cx, f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(PsslFloat2) << L", " << texCoord->cast(PsslFloat3) << L".z);" << Endl;
					break;
				case PsslTexture3D:
				case PsslTextureCube:
					assign(cx, f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(PsslFloat3) << L", " << texCoord->cast(PsslFloat4) << L".w);" << Endl;
					break;
				default:
					return false;
				}
			}
		}
	}
	else
	{
		if (samplerState.compare == CfNone)
		{
			switch (texture->getType())
			{
			case PsslTexture2D:
				assign(cx, f, out) << textureName << L".SampleLevel(" << samplerName << L", " << texCoord->cast(PsslFloat2) << L", " << (mip ? mip->cast(PsslFloat) : L"0.0f") << L");" << Endl;
				break;
			case PsslTexture3D:
			case PsslTextureCube:
				assign(cx, f, out) << textureName << L".SampleLevel(" << samplerName << L", " << texCoord->cast(PsslFloat3) << L", " << (mip ? mip->cast(PsslFloat) : L"0.0f") << L");" << Endl;
				break;
			default:
				return false;
			}
		}
		else
		{
			switch (texture->getType())
			{
			case PsslTexture2D:
				assign(cx, f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(PsslFloat2) << L", " << texCoord->cast(PsslFloat3) << L".z);" << Endl;
				break;
			case PsslTexture3D:
			case PsslTextureCube:
				assign(cx, f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(PsslFloat3) << L", " << texCoord->cast(PsslFloat4) << L".w);" << Endl;
				break;
			default:
				return false;
			}
		}
	}

	return true;
}

bool emitScalar(PsslContext& cx, Scalar* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	f << L"const float " << out->getName() << L" = " << node->get() << L";" << Endl;
	return true;
}

bool emitScript(PsslContext& cx, Script* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	const std::wstring& script = node->getScript();
	if (script.empty())
		return false;

	// Emit input and outputs.
	int32_t inputPinCount = node->getInputPinCount();
	int32_t outputPinCount = node->getOutputPinCount();

	RefArray< PsslVariable > in(inputPinCount);
	RefArray< PsslVariable > out(outputPinCount);

	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const TypedOutputPin* outputPin = static_cast< const TypedOutputPin* >(node->getOutputPin(i));
		T_ASSERT (outputPin);

		out[i] = cx.emitOutput(
			node,
			outputPin->getName(),
			pssl_from_parameter_type(outputPin->getType())
		);
	}

	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		in[i] = cx.emitInput(node->getInputPin(i));
		if (!in[i])
			return false;
	}

	// Define samplers.
	const std::map< std::wstring, SamplerState >& samplers = node->getSamplers();
	for (std::map< std::wstring, SamplerState >::const_iterator i = samplers.begin(); i != samplers.end(); ++i)
	{
		//const D3D11_TEXTURE_ADDRESS_MODE c_d3dAddress[] =
		//{
		//	D3D11_TEXTURE_ADDRESS_WRAP,
		//	D3D11_TEXTURE_ADDRESS_MIRROR,
		//	D3D11_TEXTURE_ADDRESS_CLAMP,
		//	D3D11_TEXTURE_ADDRESS_BORDER
		//};

		//const D3D11_COMPARISON_FUNC c_d3dComparison[] =
		//{
		//	D3D11_COMPARISON_ALWAYS,
		//	D3D11_COMPARISON_NEVER,
		//	D3D11_COMPARISON_LESS,
		//	D3D11_COMPARISON_LESS_EQUAL,
		//	D3D11_COMPARISON_GREATER,
		//	D3D11_COMPARISON_GREATER_EQUAL,
		//	D3D11_COMPARISON_EQUAL,
		//	D3D11_COMPARISON_NOT_EQUAL,
		//	D3D11_COMPARISON_NEVER
		//};

		const SamplerState& samplerState = i->second;

		//D3D11_SAMPLER_DESC dsd;
		//std::memset(&dsd, 0, sizeof(dsd));

		//dsd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		//dsd.AddressU = c_d3dAddress[samplerState.addressU];
		//dsd.AddressV = c_d3dAddress[samplerState.addressV];
		//dsd.AddressW = c_d3dAddress[samplerState.addressW];
		//dsd.MipLODBias = samplerState.mipBias;
		//dsd.MaxAnisotropy = 1;
		//dsd.ComparisonFunc = c_d3dComparison[samplerState.compare];
		//dsd.BorderColor[0] =
		//dsd.BorderColor[1] =
		//dsd.BorderColor[2] =
		//dsd.BorderColor[3] = 1.0f;
		//dsd.MinLOD = -D3D11_FLOAT32_MAX;
		//dsd.MaxLOD =  D3D11_FLOAT32_MAX;

		//switch (samplerState.mipFilter)
		//{
		//case FtPoint:
		//	break;
		//case FtLinear:
		//	(UINT&)dsd.Filter |= 0x1;
		//	break;
		//}

		//switch (samplerState.magFilter)
		//{
		//case FtPoint:
		//	break;
		//case FtLinear:
		//	(UINT&)dsd.Filter |= 0x4;
		//	break;
		//}

		//switch (samplerState.minFilter)
		//{
		//case FtPoint:
		//	break;
		//case FtLinear:
		//	(UINT&)dsd.Filter |= 0x10;
		//	break;
		//}

		//if (samplerState.compare == CfNone)
		//{
		//	if (
		//		samplerState.useAnisotropic &&
		//		dsd.Filter == (0x10 | 0x4 | 0x1)
		//	)
		//		dsd.Filter = D3D11_FILTER_ANISOTROPIC;
		//}
		//else
		//	(UINT&)dsd.Filter |= 0x80;

		Adler32 samplerHash;
		samplerHash.begin();
		//samplerHash.feed(&dsd, sizeof(dsd));
		samplerHash.end();

		std::wstring samplerName = i->first;

		//const std::map< std::wstring, D3D11_SAMPLER_DESC >& samplers = cx.getShader().getSamplers();
		//if (samplers.find(samplerName) == samplers.end())
		{
			StringOutputStream& fu = cx.getShader().getOutputStream(PsslShader::BtSamplers);

			if (samplerState.compare == CfNone)
				fu << L"SamplerState " << samplerName << L";" << Endl;
			else
				fu << L"SamplerComparisonState " << samplerName << L";" << Endl;

			//cx.getShader().addSampler(samplerName, dsd);
		}
	}

	// Define script instance.
	if (cx.getShader().defineScript(node->getName()))
	{
		StringOutputStream& fs = cx.getShader().getOutputStream(PsslShader::BtScript);

		fs << L"void " << node->getName() << L"(";

		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << pssl_type_name(in[i]->getType(), false) << L" " << node->getInputPin(i)->getName();
		}

		if (!in.empty())
			fs << L", ";

		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << L"out " << pssl_type_name(out[i]->getType(), false) << L" " << node->getOutputPin(i)->getName();
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
		f << pssl_type_name(out[i]->getType(), cx.inPixel()) << L" " << out[i]->getName() << L";" << Endl;

	f << node->getName() << L"(";

	for (RefArray< PsslVariable >::const_iterator i = in.begin(); i != in.end(); ++i)
	{
		if (i != in.begin())
			f << L", ";
		f << (*i)->getName();
	}

	if (!in.empty())
		f << L", ";

	for (RefArray< PsslVariable >::const_iterator i = out.begin(); i != out.end(); ++i)
	{
		if (i != out.begin())
			f << L", ";
		f << (*i)->getName();
	}

	f << L");" << Endl;
	return true;
}

bool emitSign(PsslContext& cx, Sign* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"sign(" << in->getName() << L");" << Endl;
	return true;
}

bool emitSin(PsslContext& cx, Sin* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != PsslFloat)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"sin(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitSqrt(PsslContext& cx, Sqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"sqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitStep(PsslContext& cx, Step* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"X");
	PsslVariable* in2 = cx.emitInput(node, L"Y");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << L"step(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitSub(PsslContext& cx, Sub* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in1 = cx.emitInput(node, L"Input1");
	PsslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	PsslType type = std::max< PsslType >(in1->getType(), in2->getType());
	PsslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(cx, f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitSum(PsslContext& cx, Sum* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	PsslVariable* N = cx.emitOutput(node, L"N", PsslFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslVoid);
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
	cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		PsslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(PsslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	assign(cx, f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	if (cx.inPixel())
		f << L"[unroll]" << Endl;
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

bool emitSwizzle(PsslContext& cx, Swizzle* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	std::wstring map = toLower(node->get());
	if (map.length() == 0)
		return false;

	const PsslType types[] = { PsslFloat, PsslFloat2, PsslFloat3, PsslFloat4 };
	PsslType type = types[map.length() - 1];

	// Check if input is a constant Vector node; thus pack directly instead of swizzle.
	const Vector* constVector = dynamic_type_cast< const Vector* >(cx.getInputNode(node, L"Input"));
	if (constVector)
	{
		PsslVariable* out = cx.emitOutput(node, L"Output", type);
		StringOutputStream ss;
		ss << pssl_type_name(type, cx.inPixel()) << L"(";
		for (size_t i = 0; i < map.length(); ++i)
		{
			if (i > 0)
				ss << L", ";

			switch (map[i])
			{
			case '0':
				ss << L"0.0f";
				break;

			case '1':
				ss << L"1.0f";
				break;

			case 'x':
				ss << constVector->get().x();
				break;

			case 'y':
				ss << constVector->get().y();
				break;

			case 'z':
				ss << constVector->get().z();
				break;

			case 'w':
				ss << constVector->get().w();
				break;
			}
		}
		ss << L")";
		assign(cx, f, out) << ss.str() << L";" << Endl;
		return true;
	}

	// Not constant input; need to evaluate input further.
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	if (
		(map == L"xyzw" && in->getType() == PsslFloat4) ||
		(map == L"xyz" && in->getType() == PsslFloat3) ||
		(map == L"xy" && in->getType() == PsslFloat2) ||
		(map == L"x" && in->getType() == PsslFloat)
	)
	{
		// No need to swizzle; pass variable further.
		cx.emitOutput(node, L"Output", in);
	}
	else
	{
		PsslVariable* out = cx.emitOutput(node, L"Output", type);

		bool containConstant = false;
		for (size_t i = 0; i < map.length() && !containConstant; ++i)
		{
			if (map[i] == L'0' || map[i] == L'1')
				containConstant = true;
		}

		StringOutputStream ss;
		if (containConstant || (map.length() > 1 && in->getType() == PsslFloat))
		{
			ss << pssl_type_name(type, cx.inPixel()) << L"(";
			for (size_t i = 0; i < map.length(); ++i)
			{
				if (i > 0)
					ss << L", ";
				switch (map[i])
				{
				case 'x':
					if (in->getType() == PsslFloat)
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
			if (map[0] == L'x' && in->getType() == PsslFloat)
				ss << in->getName();
			else
				ss << in->getName() << L'.' << map[0];
		}

		assign(cx, f, out) << ss.str() << L";" << Endl;
	}

	return true;
}

bool emitSwitch(PsslContext& cx, Switch* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	PsslVariable* in = cx.emitInput(node, L"Select");
	if (!in)
		return false;

	const std::vector< int32_t >& caseConditions = node->getCases();
	std::vector< std::wstring > caseBranches;
	std::vector< PsslVariable > caseInputs;
	PsslType outputType = PsslVoid;

	// Conditional branches.
	for (uint32_t i = 0; i < uint32_t(caseConditions.size()); ++i)
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(i + 2);
		T_ASSERT (caseInput);

		PsslVariable* caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(*caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(PsslShader::BtBody);
	}

	// Default branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(PsslShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(1);
		T_ASSERT (caseInput);

		PsslVariable* caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(*caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(PsslShader::BtBody);
	}

	// Create output variable.
	PsslVariable* out = cx.emitOutput(node, L"Output", outputType);
	assign(cx, f, out) << L"0;" << Endl;

	for (uint32_t i = 0; i < uint32_t(caseConditions.size()); ++i)
	{
		f << (i == 0 ? L"if (" : L"else if (") << L"int(" << in->cast(PsslFloat) << L") == " << caseConditions[i] << L")" << Endl;
		f << L"{" << Endl;
		f << IncreaseIndent;

		f << caseBranches[i];
		f << out->getName() << L" = " << caseInputs[i].cast(outputType) << L";" << Endl;

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
	f << out->getName() << L" = " << caseInputs.back().cast(outputType) << L";" << Endl;

	if (!caseConditions.empty())
	{
		f << DecreaseIndent;
		f << L"}" << Endl;
	}

	return true;
}

bool emitTan(PsslContext& cx, Tan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != PsslFloat)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat);
	assign(cx, f, out) << L"tan(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitTargetSize(PsslContext& cx, TargetSize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat2);
	assign(cx, f, out) << L"_dx11_targetSize.xy;" << Endl;
	cx.getShader().allocateTargetSize();
	return true;
}

bool emitTextureSize(PsslContext& cx, TextureSize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);

	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	std::wstring textureName = in->getName();

	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat3);

	f << L"float3 " << out->getName() << L" = float3(0.0f, 0.0f, 0.0f);" << Endl;
	switch (in->getType())
	{
	case PsslTexture2D:
		f << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y);" << Endl;
		break;

	case PsslTexture3D:
		f << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y, " << out->getName() << L".z);" << Endl;
		break;

	case PsslTextureCube:
		f << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y);" << Endl;
		break;

	default:
		return false;
	}

	return true;
}

bool emitTransform(PsslContext& cx, Transform* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	PsslVariable* transform = cx.emitInput(node, L"Transform");
	if (!in || !transform)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"mul(" << transform->getName() << L", " << in->getName() << L");" << Endl;
	return true;
}

bool emitTranspose(PsslContext& cx, Transpose* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"transpose(" << in->getName() << L");" << Endl;
	return true;
}

bool emitTruncate(PsslContext& cx, Truncate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	PsslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(cx, f, out) << L"trunc(" << in->getName() << L");" << Endl;
	return true;
}

bool emitUniform(PsslContext& cx, Uniform* node)
{
	PsslVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		pssl_from_parameter_type(node->getParameterType())
	);

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		if (out->getType() < PsslTexture2D)
		{
			const PsslShader::BlockType c_blockType[] = { PsslShader::BtCBufferOnce, PsslShader::BtCBufferFrame, PsslShader::BtCBufferDraw };
			StringOutputStream& fu = cx.getShader().getOutputStream(c_blockType[node->getFrequency()]);
			fu << pssl_type_name(out->getType(), false) << L" " << node->getParameterName() << L";" << Endl;
		}
		else
		{
			StringOutputStream& fu = cx.getShader().getOutputStream(PsslShader::BtTextures);
			switch (node->getParameterType())
			{
			case PtTexture2D:
				fu << L"Texture2D " << node->getParameterName() << L";" << Endl;
				break;

			case PtTexture3D:
				fu << L"Texture3D " << node->getParameterName() << L";" << Endl;
				break;

			case PtTextureCube:
				fu << L"TextureCube " << node->getParameterName() << L";" << Endl;
				break;

			default:
				return false;
			}
		}

		cx.getShader().addUniform(node->getParameterName());
	}

	return true;
}

bool emitVector(PsslContext& cx, Vector* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
	PsslVariable* out = cx.emitOutput(node, L"Output", PsslFloat4);
	assign(cx, f, out) << L"float4(" << node->get().x() << L", " << node->get().y() << L", " << node->get().z() << L", " << node->get().w() << L");" << Endl;
	return true;
}

bool emitVertexInput(PsslContext& cx, VertexInput* node)
{
	if (!cx.inVertex())
		return false;

	PsslShader& shader = cx.getShader();
	PsslType type = pssl_from_data_type(node->getDataType());

	// Declare input variable.
	if (!shader.haveInput(node->getName()))
	{
		std::wstring semantic = pssl_semantic(node->getDataUsage(), node->getIndex());

		StringOutputStream& fi = shader.getOutputStream(PsslShader::BtInput);
		fi << pssl_type_name(type, cx.inPixel()) << L" " << node->getName() << L" : " << semantic << L";" << Endl;

		shader.addInput(node->getName());
	}

	// Read value from input.
	if (node->getDataUsage() == DuPosition)
	{
		PsslVariable* out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			PsslFloat4
		);
		StringOutputStream& f = shader.getOutputStream(PsslShader::BtBody);
		switch (type)
		{
		case PsslFloat:
			assign(cx, f, out) << L"float4(i." << node->getName() << L".x, 0.0f, 0.0f, 1.0f);" << Endl;
			break;

		case PsslFloat2:
			assign(cx, f, out) << L"float4(i." << node->getName() << L".xy, 0.0f, 1.0f);" << Endl;
			break;

		case PsslFloat3:
			assign(cx, f, out) << L"float4(i." << node->getName() << L".xyz, 1.0f);" << Endl;
			break;

		default:
			assign(cx, f, out) << L"i." << node->getName() << L";" << Endl;
			break;
		}
	}
	else if (node->getDataUsage() == DuNormal)
	{
		PsslVariable* out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			PsslFloat4
		);
		StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
		switch (type)
		{
		case PsslFloat:
			assign(cx, f, out) << L"float4(i." << node->getName() << L".x, 0.0f, 0.0f, 0.0f);" << Endl;
			break;

		case PsslFloat2:
			assign(cx, f, out) << L"float4(i." << node->getName() << L".xy, 0.0f, 0.0f);" << Endl;
			break;

		case PsslFloat3:
			assign(cx, f, out) << L"float4(i." << node->getName() << L".xyz, 0.0f);" << Endl;
			break;

		default:
			assign(cx, f, out) << L"i." << node->getName() << L";" << Endl;
			break;
		}
	}
	else
	{
		PsslVariable* out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			type
		);
		StringOutputStream& f = cx.getShader().getOutputStream(PsslShader::BtBody);
		assign(cx, f, out) << L"i." << node->getName() << L";" << Endl;
	}

	return true;
}

bool emitVertexOutput(PsslContext& cx, VertexOutput* node)
{
	cx.enterVertex();

	PsslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	StringOutputStream& fo = cx.getVertexShader().getOutputStream(PsslShader::BtOutput);
	fo << L"float4 Position : SV_Position;" << Endl;

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(PsslShader::BtBody);
	switch (in->getType())
	{
	case PsslFloat:
		fb << L"o.Position = float4(" << in->getName() << L".x, 0, 0, 1);" << Endl;
		break;

	case PsslFloat2:
		fb << L"o.Position = float4(" << in->getName() << L".xy, 0, 1);" << Endl;
		break;

	case PsslFloat3:
		fb << L"o.Position = float4(" << in->getName() << L".xyz, 1);" << Endl;
		break;

	case PsslFloat4:
		fb << L"o.Position = " << in->getName() << L";" << Endl;
		break;

	default:
		return false;
	}

	return true;
}

struct Emitter
{
	virtual bool emit(PsslContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public Emitter
{
	typedef bool (*function_t)(PsslContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function) :
		m_function(function)
	{
	}

	virtual bool emit(PsslContext& c, Node* node)
	{
		T_ASSERT (is_a< NodeType >(node));
		return (*m_function)(c, static_cast< NodeType* >(node));
	}
};

		}

PsslEmitter::PsslEmitter()
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
	m_emitters[&type_of< Div >()] = new EmitterCast< Div >(emitDiv);
	m_emitters[&type_of< Discard >()] = new EmitterCast< Discard >(emitDiscard);
	m_emitters[&type_of< Dot >()] = new EmitterCast< Dot >(emitDot);
	m_emitters[&type_of< Exp >()] = new EmitterCast< Exp >(emitExp);
	m_emitters[&type_of< Fraction >()] = new EmitterCast< Fraction >(emitFraction);
	m_emitters[&type_of< FragmentPosition >()] = new EmitterCast< FragmentPosition >(emitFragmentPosition);
	m_emitters[&type_of< FrontFace >()] = new EmitterCast< FrontFace >(emitFrontFace);
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
	m_emitters[&type_of< Reflect >()] = new EmitterCast< Reflect >(emitReflect);
	m_emitters[&type_of< RecipSqrt >()] = new EmitterCast< RecipSqrt >(emitRecipSqrt);
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

PsslEmitter::~PsslEmitter()
{
	for (std::map< const TypeInfo*, Emitter* >::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
		delete i->second;
}

bool PsslEmitter::emit(PsslContext& c, Node* node)
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
	return i->second->emit(c, node);
}

	}
}
