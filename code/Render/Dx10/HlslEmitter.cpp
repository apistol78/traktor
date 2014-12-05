#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/VertexElement.h"
#include "Render/Dx10/Platform.h"
#include "Render/Dx10/HlslContext.h"
#include "Render/Dx10/HlslEmitter.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/Script.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

StringOutputStream& assign(StringOutputStream& f, HlslVariable* out)
{
	f << hlsl_type_name(out->getType()) << L" " << out->getName() << L" = ";
	return f;
}

bool emitAbs(HlslContext& cx, Abs* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"abs(" << in->getName() << L");" << Endl;
	return true;
}

bool emitAdd(HlslContext& cx, Add* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitArcusCos(HlslContext& cx, ArcusCos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	assign(f, out) << L"acos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitArcusTan(HlslContext& cx, ArcusTan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* xy = cx.emitInput(node, L"XY");
	if (!xy || xy->getType() != HtFloat2)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	assign(f, out) << L"atan2(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;
	return true;
}

bool emitClamp(HlslContext& cx, Clamp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	if (node->getMin() == 0.0f && node->getMax() == 1.0f)
		assign(f, out) << L"saturate(" << in->getName() << L");" << Endl;
	else
		assign(f, out) << L"clamp(" << in->getName() << L", " << node->getMin() << L", " << node->getMax() << L");" << Endl;
	return true;
}

bool emitColor(HlslContext& cx, Color* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat4);
	if (!out)
		return false;
	traktor::Color4ub color = node->getColor();
	f << L"const float4 " << out->getName() << L" = float4(" << (color.r / 255.0f) << L", " << (color.g / 255.0f) << L", " << (color.b / 255.0f) << L", " << (color.a / 255.0f) << L");" << Endl;
	return true;
}

bool emitConditional(HlslContext& cx, Conditional* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	// Emit input and reference branches.
	HlslVariable* in = cx.emitInput(node, L"Input");
	HlslVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	HlslVariable caseTrue, caseFalse;
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

		cx.getShader().pushOutputStream(HlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		HlslVariable* ct = cx.emitInput(node, L"CaseTrue");
		if (!ct)
			return false;

		caseTrue = *ct;
		caseTrueBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(HlslShader::BtBody);
	}

	// Emit false branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(HlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		HlslVariable* cf = cx.emitInput(node, L"CaseFalse");
		if (!cf)
			return false;

		caseFalse = *cf;
		caseFalseBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(HlslShader::BtBody);
	}

	// Create output variable.
	HlslType outputType = std::max< HlslType >(caseTrue.getType(), caseFalse.getType());
	
	HlslVariable* out = cx.emitOutput(node, L"Output", outputType);
	f << hlsl_type_name(out->getType()) << L" " << out->getName() << L";" << Endl;

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

bool emitCos(HlslContext& cx, Cos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	assign(f, out) << L"cos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitCross(HlslContext& cx, Cross* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat3);
	assign(f, out) << L"cross(" << in1->cast(HtFloat3) << L", " << in2->cast(HtFloat3) << L");" << Endl;
	return true;
}

bool emitDerivative(HlslContext& cx, Derivative* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* input = cx.emitInput(node, L"Input");
	if (!input)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", input->getType());
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

bool emitDiv(HlslContext& cx, Div* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitDiscard(HlslContext& cx, Discard* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	// Emit input and reference branches.
	HlslVariable* in = cx.emitInput(node, L"Input");
	HlslVariable* ref = cx.emitInput(node, L"Reference");
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

	HlslVariable* pass = cx.emitInput(node, L"Pass");
	if (!pass)
		return false;

	HlslVariable* out = cx.emitOutput(node, L"Output", pass->getType());
	assign(f, out) << pass->getName() << L";" << Endl;

	return true;
}

bool emitDot(HlslContext& cx, Dot* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	assign(f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitExp(HlslContext& cx, Exp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"exp(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFraction(HlslContext& cx, Fraction* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"frac(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFragmentPosition(HlslContext& cx, FragmentPosition* node)
{
	if (!cx.inPixel())
		return false;

	cx.getShader().allocateVPos();

	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat2);
	assign(f, out) << L"vPos;" << Endl;

	return true;
}

bool emitIndexedUniform(HlslContext& cx, IndexedUniform* node)
{
	const HlslType c_parameterType[] = { HtFloat, HtFloat4, HtFloat4x4 };

	HlslVariable* index = cx.emitInput(node, L"Index");
	if (!index)
		return false;

	HlslVariable* out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		c_parameterType[node->getParameterType()]
	);

	StringOutputStream& fb = cx.getShader().getOutputStream(HlslShader::BtBody);
	assign(fb, out) << node->getParameterName() << L"[" << index->getName() << L"];" << Endl;

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(HlslShader::BtUniform);
		fu << L"uniform " << hlsl_type_name(out->getType()) << L" " << node->getParameterName() << L"[" << node->getLength() << L"];" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}

	return true;
}

bool emitInterpolator(HlslContext& cx, Interpolator* node)
{
	if (!cx.inPixel())
	{
		// We're already in vertex state; skip interpolation.
		HlslVariable* in = cx.emitInput(node, L"Input");
		if (!in)
			return false;

		HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());

		StringOutputStream& fb = cx.getShader().getOutputStream(HlslShader::BtBody);
		assign(fb, out) << in->getName() << L";" << Endl;

		return true;
	}

	cx.enterVertex();

	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	int32_t interpolatorId = cx.getShader().allocateInterpolator();
	std::wstring interpolator = L"Attr" + toString(interpolatorId);

	StringOutputStream& fo = cx.getVertexShader().getOutputStream(HlslShader::BtOutput);
	fo << hlsl_type_name(in->getType()) << L" " << interpolator << L" : TEXCOORD" << interpolatorId << L";" << Endl;

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(HlslShader::BtBody);
	fb << L"o." << interpolator << L" = " << in->getName() << L";" << Endl;

	cx.enterPixel();

	cx.getPixelShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		L"i." + interpolator,
		in->getType()
	);

	StringOutputStream& fpi = cx.getPixelShader().getOutputStream(HlslShader::BtInput);
	fpi << hlsl_type_name(in->getType()) << L" " << interpolator << L" : TEXCOORD" << interpolatorId << L";" << Endl;

	return true;
}

bool emitIterate(HlslContext& cx, Iterate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	HlslVariable* N = cx.emitOutput(node, L"N", HtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	HlslVariable* out = cx.emitOutput(node, L"Output", HtVoid);
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
	cx.getShader().pushOutputStream(HlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		HlslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		// Emit post condition if connected; break iteration if condition is false.
		HlslVariable* condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (!(bool)" << condition->cast(HtFloat) << L")" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(HlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	HlslVariable* initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << L"0;" << Endl;

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

bool emitLength(HlslContext& cx, Length* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	assign(f, out) << L"length(" << in->getName() << L");" << Endl;
	return true;
}

bool emitLerp(HlslContext& cx, Lerp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* blend = cx.emitInput(node, L"Blend");
	if (!blend || blend->getType() != HtFloat)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in1->getType());
	assign(f, out) << L"lerp(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;
	return true;
}

bool emitLog(HlslContext& cx, Log* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
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

bool emitMatrixIn(HlslContext& cx, MatrixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* xaxis = cx.emitInput(node, L"XAxis");
	HlslVariable* yaxis = cx.emitInput(node, L"YAxis");
	HlslVariable* zaxis = cx.emitInput(node, L"ZAxis");
	HlslVariable* translate = cx.emitInput(node, L"Translate");
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat4x4);
	assign(f, out) << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;
	f << (xaxis     ? xaxis->cast(HtFloat4)     : L"1.0f, 0.0f, 0.0f, 0.0f") << L"," << Endl;
	f << (yaxis     ? yaxis->cast(HtFloat4)     : L"0.0f, 1.0f, 0.0f, 0.0f") << L"," << Endl;
	f << (zaxis     ? zaxis->cast(HtFloat4)     : L"0.0f, 0.0f, 1.0f, 0.0f") << L"," << Endl;
	f << (translate ? translate->cast(HtFloat4) : L"0.0f, 0.0f, 0.0f, 1.0f") << Endl;
	f << DecreaseIndent;
	f << L"};" << Endl;
	return true;
}

bool emitMatrixOut(HlslContext& cx, MatrixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* xaxis = cx.emitOutput(node, L"XAxis", HtFloat4);
	if (xaxis)
		assign(f, xaxis) << in->getName() << L"._11_21_31_41;" << Endl;
	HlslVariable* yaxis = cx.emitOutput(node, L"YAxis", HtFloat4);
	if (yaxis)
		assign(f, yaxis) << in->getName() << L"._12_22_32_42;" << Endl;
	HlslVariable* zaxis = cx.emitOutput(node, L"ZAxis", HtFloat4);
	if (zaxis)
		assign(f, zaxis) << in->getName() << L"._13_23_33_43;" << Endl;
	HlslVariable* translate = cx.emitOutput(node, L"Translate", HtFloat4);
	if (translate)
		assign(f, translate) << in->getName() << L"._14_24_34_44;" << Endl;
	return true;
}

bool emitMax(HlslContext& cx, Max* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"max(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMin(HlslContext& cx, Min* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMixIn(HlslContext& cx, MixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* x = cx.emitInput(node, L"X");
	HlslVariable* y = cx.emitInput(node, L"Y");
	HlslVariable* z = cx.emitInput(node, L"Z");
	HlslVariable* w = cx.emitInput(node, L"W");

	if (!y && !z && !w)
	{
		HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
		assign(f, out) << L"float(" << (x ? x->getName() : L"0.0f") << L");" << Endl;
	}
	else if (!z && !w)
	{
		HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat2);
		assign(f, out) << L"float2(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L");" << Endl;
	}
	else if (!w)
	{
		HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat3);
		assign(f, out) << L"float3(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L", " << (z ? z->getName() : L"0.0f") << L");" << Endl;
	}
	else
	{
		HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat4);
		assign(f, out) << L"float4(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L", " << (z ? z->getName() : L"0.0f") << L", " << (w ? w->getName() : L"0.0f") << L");" << Endl;
	}

	return true;
}

bool emitMixOut(HlslContext& cx, MixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	switch (in->getType())
	{
	case HtFloat:
		{
			HlslVariable* x = cx.emitOutput(node, L"X", HtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
		}
		break;

	case HtFloat2:
		{
			HlslVariable* x = cx.emitOutput(node, L"X", HtFloat);
			HlslVariable* y = cx.emitOutput(node, L"Y", HtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
		}
		break;

	case HtFloat3:
		{
			HlslVariable* x = cx.emitOutput(node, L"X", HtFloat);
			HlslVariable* y = cx.emitOutput(node, L"Y", HtFloat);
			HlslVariable* z = cx.emitOutput(node, L"Z", HtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
			assign(f, z) << in->getName() << L".z;" << Endl;
		}
		break;

	case HtFloat4:
		{
			HlslVariable* x = cx.emitOutput(node, L"X", HtFloat);
			HlslVariable* y = cx.emitOutput(node, L"Y", HtFloat);
			HlslVariable* z = cx.emitOutput(node, L"Z", HtFloat);
			HlslVariable* w = cx.emitOutput(node, L"W", HtFloat);
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

bool emitMul(HlslContext& cx, Mul* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitMulAdd(HlslContext& cx, MulAdd* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	HlslVariable* in3 = cx.emitInput(node, L"Input3");
	if (!in1 || !in2 || !in3)
		return false;
	HlslType type = std::max< HlslType >(std::max< HlslType >(in1->getType(), in2->getType()), in3->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L" + " << in3->cast(type) << L";" << Endl;
	return true;
}

bool emitNeg(HlslContext& cx, Neg* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"-" << in->getName() << L";" << Endl;
	return true;
}

bool emitNormalize(HlslContext& cx, Normalize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"normalize(" << in->getName() << L");" << Endl;
	return true;
}

bool emitPixelOutput(HlslContext& cx, PixelOutput* node)
{
	const  D3D10_CULL_MODE d3dCullMode[] =
	{
		D3D10_CULL_NONE,
		D3D10_CULL_FRONT,
		D3D10_CULL_BACK
	};

	const D3D10_BLEND_OP d3dBlendOperation[] =
	{
		D3D10_BLEND_OP_ADD,
		D3D10_BLEND_OP_SUBTRACT,
		D3D10_BLEND_OP_REV_SUBTRACT,
		D3D10_BLEND_OP_MIN,
		D3D10_BLEND_OP_MAX
	};

	const D3D10_BLEND d3dBlendFactor[] =
	{
		D3D10_BLEND_ONE,
		D3D10_BLEND_ZERO,
		D3D10_BLEND_SRC_COLOR,
		D3D10_BLEND_INV_SRC_COLOR,
		D3D10_BLEND_DEST_COLOR,
		D3D10_BLEND_INV_DEST_COLOR,
		D3D10_BLEND_SRC_ALPHA,
		D3D10_BLEND_INV_SRC_ALPHA,
		D3D10_BLEND_DEST_ALPHA,
		D3D10_BLEND_INV_DEST_ALPHA
	};

	const D3D10_COMPARISON_FUNC d3dCompareFunction[] =
	{
		D3D10_COMPARISON_ALWAYS,
		D3D10_COMPARISON_NEVER,
		D3D10_COMPARISON_LESS,
		D3D10_COMPARISON_LESS_EQUAL,
		D3D10_COMPARISON_GREATER,
		D3D10_COMPARISON_GREATER_EQUAL,
		D3D10_COMPARISON_EQUAL,
		D3D10_COMPARISON_NOT_EQUAL
	};

	const D3D10_STENCIL_OP d3dStencilOperation[] =
	{
		D3D10_STENCIL_OP_KEEP,
		D3D10_STENCIL_OP_ZERO,
		D3D10_STENCIL_OP_REPLACE,
		D3D10_STENCIL_OP_INCR_SAT,
		D3D10_STENCIL_OP_DECR_SAT,
		D3D10_STENCIL_OP_INVERT,
		D3D10_STENCIL_OP_INCR,
		D3D10_STENCIL_OP_DECR
	};

	RenderState rs = node->getRenderState();

	cx.enterPixel();

	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	StringOutputStream& fpo = cx.getPixelShader().getOutputStream(HlslShader::BtOutput);
	fpo << L"float4 Color0 : SV_Target0;" << Endl;

	StringOutputStream& fpb = cx.getPixelShader().getOutputStream(HlslShader::BtBody);
	fpb << L"float4 out_Color = " << in->cast(HtFloat4) << L";" << Endl;

	// Emulate old fashion alpha test through "discard" instruction.
	if (rs.alphaTestEnable)
	{
		float alphaRef = rs.alphaTestReference / 255.0f;

		if (rs.alphaTestFunction == CfLess)
			fpb << L"if (out_Color.w >= " << alphaRef << L")" << Endl;
		else if (rs.alphaTestFunction == CfLessEqual)
			fpb << L"if (out_Color.w > " << alphaRef << L")" << Endl;
		else if (rs.alphaTestFunction == CfGreater)
			fpb << L"if (out_Color.w <= " << alphaRef << L")" << Endl;
		else if (rs.alphaTestFunction == CfGreaterEqual)
			fpb << L"if (out_Color.w < " << alphaRef << L")" << Endl;
		else if (rs.alphaTestFunction == CfEqual)
			fpb << L"if (out_Color.w != " << alphaRef << L")" << Endl;
		else if (rs.alphaTestFunction == CfNotEqual)
			fpb << L"if (out_Color.w == " << alphaRef << L")" << Endl;
		else
			return false;

		fpb << L"\tdiscard;" << Endl;
	}

	fpb << L"o.Color0 = out_Color;" << Endl;

	cx.getD3DRasterizerDesc().FillMode = rs.wireframe ? D3D10_FILL_WIREFRAME : D3D10_FILL_SOLID;
	cx.getD3DRasterizerDesc().CullMode = d3dCullMode[rs.cullMode];

	cx.getD3DDepthStencilDesc().DepthEnable = rs.depthEnable ? TRUE : FALSE;
	cx.getD3DDepthStencilDesc().DepthWriteMask = rs.depthWriteEnable ? D3D10_DEPTH_WRITE_MASK_ALL : D3D10_DEPTH_WRITE_MASK_ZERO;
	cx.getD3DDepthStencilDesc().DepthFunc = d3dCompareFunction[rs.depthFunction];
	cx.getD3DDepthStencilDesc().StencilEnable = rs.stencilEnable ? TRUE : FALSE;
	cx.getD3DDepthStencilDesc().StencilReadMask = 0xff;
	cx.getD3DDepthStencilDesc().StencilWriteMask = 0xff;
	cx.getD3DDepthStencilDesc().FrontFace.StencilFailOp = d3dStencilOperation[rs.stencilFail];
	cx.getD3DDepthStencilDesc().FrontFace.StencilDepthFailOp = d3dStencilOperation[rs.stencilZFail];
	cx.getD3DDepthStencilDesc().FrontFace.StencilPassOp = d3dStencilOperation[rs.stencilPass];
	cx.getD3DDepthStencilDesc().FrontFace.StencilFunc = d3dCompareFunction[rs.stencilFunction];
	cx.getD3DDepthStencilDesc().BackFace.StencilFailOp = d3dStencilOperation[rs.stencilFail];
	cx.getD3DDepthStencilDesc().BackFace.StencilDepthFailOp = d3dStencilOperation[rs.stencilZFail];
	cx.getD3DDepthStencilDesc().BackFace.StencilPassOp = d3dStencilOperation[rs.stencilPass];
	cx.getD3DDepthStencilDesc().BackFace.StencilFunc = d3dCompareFunction[rs.stencilFunction];
	cx.setStencilReference(rs.stencilReference);

	cx.getD3DBlendDesc().AlphaToCoverageEnable = rs.alphaToCoverageEnable ? TRUE : FALSE;
	cx.getD3DBlendDesc().BlendEnable[0] = rs.blendEnable ? TRUE : FALSE;
	cx.getD3DBlendDesc().SrcBlend = d3dBlendFactor[rs.blendSource];
	cx.getD3DBlendDesc().DestBlend = d3dBlendFactor[rs.blendDestination];
	cx.getD3DBlendDesc().BlendOp = d3dBlendOperation[rs.blendOperation];

	UINT8 d3dWriteMask = 0;
	if (rs.colorWriteMask & CwRed)
		d3dWriteMask |= D3D10_COLOR_WRITE_ENABLE_RED;
	if (rs.colorWriteMask & CwGreen)
		d3dWriteMask |= D3D10_COLOR_WRITE_ENABLE_GREEN;
	if (rs.colorWriteMask & CwBlue)	
		d3dWriteMask |= D3D10_COLOR_WRITE_ENABLE_BLUE;
	if (rs.colorWriteMask & CwAlpha)
		d3dWriteMask |= D3D10_COLOR_WRITE_ENABLE_ALPHA;

	cx.getD3DBlendDesc().RenderTargetWriteMask[0] = d3dWriteMask;

	return true;
}

bool emitPolynomial(HlslContext& cx, Polynomial* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	HlslVariable* x = cx.emitInput(node, L"X");
	HlslVariable* coeffs = cx.emitInput(node, L"Coefficients");
	if (!x || !coeffs)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);

	assign(f, out);
	switch (coeffs->getType())
	{
	case HtFloat:
		f << coeffs->getName() << L".x * " << x->getName();
		break;
	case HtFloat2:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".y * " << x->getName();
		break;
	case HtFloat3:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".z * " << x->getName();
		break;
	case HtFloat4:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 4) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".z * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".w * " << x->getName();
		break;
	}
	f << L";" << Endl;

	return true;
}

bool emitPow(HlslContext& cx, Pow* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* exponent = cx.emitInput(node, L"Exponent");
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!exponent || !in)
		return false;
	HlslType type = std::max< HlslType >(exponent->getType(), in->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"pow(" << in->cast(type) << L", " << exponent->cast(type) << L");" << Endl;
	return true;
}

bool emitRecipSqrt(HlslContext& cx, RecipSqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"rsqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitReflect(HlslContext& cx, Reflect* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* normal = cx.emitInput(node, L"Normal");
	HlslVariable* direction = cx.emitInput(node, L"Direction");
	if (!normal || !direction)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", direction->getType());
	assign(f, out) << L"reflect(" << direction->getName() << L", " << normal->cast(direction->getType()) << L");" << Endl;
	return true;
}

bool emitRound(HlslContext& cx, Round* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"round(" << in->getName() << L");" << Endl;
	return true;
}

bool emitSampler(HlslContext& cx, Sampler* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	HlslVariable* texture = cx.emitInput(node, L"Texture");
	if (!texture || texture->getType() < HtTexture2D)
		return false;

	HlslVariable* texCoord = cx.emitInput(node, L"TexCoord");
	if (!texCoord)
		return false;

	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat4);

	std::wstring textureName = texture->getName();
	std::wstring samplerName = out->getName() + L"_samplerState";

	if (cx.inPixel())
	{
		switch (texture->getType())
		{
		case HtTexture2D:
			assign(f, out) << textureName << L".Sample(" << samplerName << L", " << texCoord->cast(HtFloat2) << L");" << Endl;
			break;
		case HtTexture3D:
		case HtTextureCube:
			assign(f, out) << textureName << L".Sample(" << samplerName << L", " << texCoord->cast(HtFloat3) << L");" << Endl;
			break;
		}
	}
	if (cx.inVertex())
	{
		switch (texture->getType())
		{
		case HtTexture2D:
			assign(f, out) << textureName << L".SampleLevel(" << samplerName << L", " << texCoord->cast(HtFloat2) << L", 0.0f);" << Endl;
			break;
		case HtTexture3D:
		case HtTextureCube:
			assign(f, out) << textureName << L".SampleLevel(" << samplerName << L", " << texCoord->cast(HtFloat3) << L", 0.0f);" << Endl;
			break;
		}
	}

	// Define sampler class.
	const std::map< std::wstring, D3D10_SAMPLER_DESC >& samplers = cx.getShader().getSamplers();
	if (samplers.find(samplerName) == samplers.end())
	{
		int sampler = int(samplers.size());

		const D3D10_TEXTURE_ADDRESS_MODE c_d3dAddress[] =
		{
			D3D10_TEXTURE_ADDRESS_WRAP,
			D3D10_TEXTURE_ADDRESS_MIRROR,
			D3D10_TEXTURE_ADDRESS_CLAMP,
			D3D10_TEXTURE_ADDRESS_BORDER
		};

		D3D10_SAMPLER_DESC dsd;
		dsd.Filter = D3D10_FILTER_MIN_MAG_MIP_POINT;
		dsd.AddressU = c_d3dAddress[node->getAddressU()];
		dsd.AddressV = c_d3dAddress[node->getAddressV()];
		dsd.AddressW = c_d3dAddress[node->getAddressW()];
		dsd.MipLODBias = 0.0f;
		dsd.MaxAnisotropy = 1;
		dsd.ComparisonFunc = D3D10_COMPARISON_NEVER;
		dsd.BorderColor[0] =
		dsd.BorderColor[1] =
		dsd.BorderColor[2] =
		dsd.BorderColor[3] = 1.0f;
		dsd.MinLOD = -D3D10_FLOAT32_MAX;
		dsd.MaxLOD =  D3D10_FLOAT32_MAX;

		switch (node->getMipFilter())
		{
		case Sampler::FtPoint:
			break;
		case Sampler::FtLinear:
			(UINT&)dsd.Filter |= 0x1;
			break;
		}

		switch (node->getMagFilter())
		{
		case Sampler::FtPoint:
			break;
		case Sampler::FtLinear:
			(UINT&)dsd.Filter |= 0x4;
			break;
		}

		switch (node->getMinFilter())
		{
		case Sampler::FtPoint:
			break;
		case Sampler::FtLinear:
			(UINT&)dsd.Filter |= 0x10;
			break;
		}

		StringOutputStream& fu = cx.getShader().getOutputStream(HlslShader::BtUniform);
		fu << L"SamplerState " << samplerName << L";" << Endl;
		
		cx.getShader().addSampler(samplerName, dsd);
	}

	return true;
}

bool emitScalar(HlslContext& cx, Scalar* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	f << L"const float " << out->getName() << L" = " << node->get() << L";" << Endl;
	return true;
}

bool emitScript(HlslContext& cx, Script* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	// Get platform specific script from node.
	std::wstring script = node->getScript();
	if (script.empty())
		return false;

	// Emit input and outputs.
	int32_t inputPinCount = node->getInputPinCount();
	int32_t outputPinCount = node->getOutputPinCount();

	RefArray< HlslVariable > in(inputPinCount);
	RefArray< HlslVariable > out(outputPinCount);

	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const TypedOutputPin* outputPin = static_cast< const TypedOutputPin* >(node->getOutputPin(i));
		T_ASSERT (outputPin);

		out[i] = cx.emitOutput(
			node,
			outputPin->getName(),
			hlsl_from_parameter_type(outputPin->getType())
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
		StringOutputStream& fs = cx.getShader().getOutputStream(HlslShader::BtScript);

		fs << L"void " << node->getName() << L"(";

		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << hlsl_type_name(in[i]->getType()) << L" " << node->getInputPin(i)->getName();
		}

		if (!in.empty())
			fs << L", ";

		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << L"out " << hlsl_type_name(out[i]->getType()) << L" " << node->getOutputPin(i)->getName();
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
		f << hlsl_type_name(out[i]->getType()) << L" " << out[i]->getName() << L";" << Endl;

	f << node->getName() << L"(";

	for (RefArray< HlslVariable >::const_iterator i = in.begin(); i != in.end(); ++i)
	{
		if (i != in.begin())
			f << L", ";
		f << (*i)->getName();
	}

	if (!in.empty())
		f << L", ";

	for (RefArray< HlslVariable >::const_iterator i = out.begin(); i != out.end(); ++i)
	{
		if (i != out.begin())
			f << L", ";
		f << (*i)->getName();
	}

	f << L");" << Endl;
	return true;
}

bool emitSin(HlslContext& cx, Sin* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	assign(f, out) << L"sin(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitSqrt(HlslContext& cx, Sqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitStep(HlslContext& cx, Step* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"X");
	HlslVariable* in2 = cx.emitInput(node, L"Y");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"step(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitSub(HlslContext& cx, Sub* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in1 = cx.emitInput(node, L"Input1");
	HlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	HlslType type = std::max< HlslType >(in1->getType(), in2->getType());
	HlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitSum(HlslContext& cx, Sum* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	HlslVariable* N = cx.emitOutput(node, L"N", HtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	HlslVariable* out = cx.emitOutput(node, L"Output", HtVoid);
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
	cx.getShader().pushOutputStream(HlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		HlslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(HlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	assign(f, out) << L"0;" << Endl;

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

bool emitSwizzle(HlslContext& cx, Swizzle* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	std::wstring map = node->get();
	if (map.length() == 0)
		return false;

	const HlslType types[] = { HtFloat, HtFloat2, HtFloat3, HtFloat4 };
	HlslType type = types[map.length() - 1];

	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	if (
		(map == L"xyzw" && in->getType() == HtFloat4) ||
		(map == L"xyz" && in->getType() == HtFloat3) ||
		(map == L"xy" && in->getType() == HtFloat2) ||
		(map == L"x" && in->getType() == HtFloat)
	)
	{
		// No need to swizzle; pass variable further.
		cx.emitOutput(node, L"Output", in);
	}
	else
	{
		HlslVariable* out = cx.emitOutput(node, L"Output", type);

		bool containConstant = false;
		for (size_t i = 0; i < map.length() && !containConstant; ++i)
		{
			if (map[i] == L'0' || map[i] == L'1')
				containConstant = true;
		}

		StringOutputStream ss;
		if (containConstant || (map.length() > 1 && in->getType() == HtFloat))
		{
			ss << hlsl_type_name(type) << L"(";
			for (size_t i = 0; i < map.length(); ++i)
			{
				if (i > 0)
					ss << L", ";
				switch (map[i])
				{
				case 'x':
					if (in->getType() == HtFloat)
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
			if (map[0] == L'x' && in->getType() == HtFloat)
				ss << in->getName();
			else
				ss << in->getName() << L'.' << map[0];
		}

		assign(f, out) << ss.str() << L";" << Endl;
	}

	return true;
}

bool emitSwitch(HlslContext& cx, Switch* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	HlslVariable* in = cx.emitInput(node, L"Select");
	if (!in)
		return false;

	const std::vector< int32_t >& caseConditions = node->getCases();
	std::vector< std::wstring > caseBranches;
	std::vector< HlslVariable > caseInputs;
	HlslType outputType = HtVoid;

	// Conditional branches.
	for (uint32_t i = 0; i < uint32_t(caseConditions.size()); ++i)
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(HlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(i + 2);
		T_ASSERT (caseInput);

		HlslVariable* caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(*caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(HlslShader::BtBody);
	}

	// Default branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(HlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(1);
		T_ASSERT (caseInput);

		HlslVariable* caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(*caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(HlslShader::BtBody);
	}

	// Create output variable.
	HlslVariable* out = cx.emitOutput(node, L"Output", outputType);
	assign(f, out) << L"0;" << Endl;

	for (uint32_t i = 0; i < uint32_t(caseConditions.size()); ++i)
	{
		f << (i == 0 ? L"if (" : L"else if (") << L"int(" << in->cast(HtFloat) << L") == " << caseConditions[i] << L")" << Endl;
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

bool emitTan(HlslContext& cx, Tan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat);
	assign(f, out) << L"tan(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitTargetSize(HlslContext& cx, TargetSize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat2);
	assign(f, out) << L"_dx10_targetSize.xy;" << Endl;
	return true;
}

bool emitTextureSize(HlslContext& cx, TextureSize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	std::wstring textureName = in->getName();

	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat3);

	f << L"float3 " << out->getName() << L" = float3(0.0f, 0.0f, 0.0f);" << Endl;
	switch (in->getType())
	{
	case HtTexture2D:
		f << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y);" << Endl;
		break;

	case HtTexture3D:
		f << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y, " << out->getName() << L".z);" << Endl;
		break;

	case HtTextureCube:
		f << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y);" << Endl;
		break;

	default:
		return false;
	}

	return true;
}

bool emitTransform(HlslContext& cx, Transform* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	HlslVariable* transform = cx.emitInput(node, L"Transform");
	if (!in || !transform)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"mul(" << transform->getName() << L", " << in->getName() << L");" << Endl;
	return true;
}

bool emitTranspose(HlslContext& cx, Transpose* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"transpose(" << in->getName() << L");" << Endl;
	return true;
}

bool emitTruncate(HlslContext& cx, Truncate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	HlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"trunc(" << in->getName() << L");" << Endl;
	return true;
}

bool emitUniform(HlslContext& cx, Uniform* node)
{
	HlslVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		hlsl_from_parameter_type(node->getParameterType())
	);

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(HlslShader::BtUniform);

		if (out->getType() < HtTexture2D)
			fu << L"uniform " << hlsl_type_name(out->getType()) << L" " << node->getParameterName() << L";" << Endl;
		else
		{
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
			}
		}

		cx.getShader().addUniform(node->getParameterName());
	}

	return true;
}

bool emitVector(HlslContext& cx, Vector* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	HlslVariable* out = cx.emitOutput(node, L"Output", HtFloat4);
	assign(f, out) << L"float4(" << node->get().x() << L", " << node->get().y() << L", " << node->get().z() << L", " << node->get().w() << L");" << Endl;
	return true;
}

bool emitVertexInput(HlslContext& cx, VertexInput* node)
{
	if (!cx.inVertex())
		return false;

	HlslVariable* out = cx.getShader().getInputVariable(node->getName());
	if (!out)
	{
		HlslType type = hlsl_from_data_type(node->getDataType());
		std::wstring semantic = hlsl_semantic(node->getDataUsage(), node->getIndex());

		StringOutputStream& fi = cx.getVertexShader().getOutputStream(HlslShader::BtInput);
		fi << hlsl_type_name(type) << L" " << node->getName() << L" : " << semantic << L";" << Endl;

		if (node->getDataUsage() == DuPosition && type != HtFloat4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				HtFloat4
			);
			StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
			switch (type)
			{
			case HtFloat:
				assign(f, out) << L"float4(i." << node->getName() << L".x, 0, 0, 1);" << Endl;
				break;

			case HtFloat2:
				assign(f, out) << L"float4(i." << node->getName() << L".xy, 0, 1);" << Endl;
				break;

			case HtFloat3:
				assign(f, out) << L"float4(i." << node->getName() << L".xyz, 1);" << Endl;
				break;
			}
		}
		else if (node->getDataUsage() == DuNormal && type != HtFloat4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				HtFloat4
			);
			StringOutputStream& f = cx.getShader().getOutputStream(HlslShader::BtBody);
			switch (type)
			{
			case HtFloat:
				assign(f, out) << L"float4(i." << node->getName() << L".x, 0, 0, 0);" << Endl;
				break;

			case HtFloat2:
				assign(f, out) << L"float4(i." << node->getName() << L".xy, 0, 0);" << Endl;
				break;

			case HtFloat3:
				assign(f, out) << L"float4(i." << node->getName() << L".xyz, 0);" << Endl;
				break;
			}
		}
		else
		{
			out = cx.getShader().createVariable(
				node->findOutputPin(L"Output"),
				L"i." + node->getName(),
				type
			);
		}

		cx.getShader().addInputVariable(node->getName(), out);
	}
	else
	{
		out = cx.getShader().createVariable(
			node->findOutputPin(L"Output"),
			out->getName(),
			out->getType()
		);
	}

	return true;
}

bool emitVertexOutput(HlslContext& cx, VertexOutput* node)
{
	cx.enterVertex();

	HlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	StringOutputStream& fo = cx.getVertexShader().getOutputStream(HlslShader::BtOutput);
	fo << L"float4 Position : SV_Position;" << Endl;

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(HlslShader::BtBody);
	switch (in->getType())
	{
	case HtFloat:
		fb << L"o.Position = float4(" << in->getName() << L".x, 0, 0, 1);" << Endl;
		break;

	case HtFloat2:
		fb << L"o.Position = float4(" << in->getName() << L".xy, 0, 1);" << Endl;
		break;

	case HtFloat3:
		fb << L"o.Position = float4(" << in->getName() << L".xyz, 1);" << Endl;
		break;

	case HtFloat4:
		fb << L"o.Position = " << in->getName() << L";" << Endl;
		break;
	}

	StringOutputStream& fpi = cx.getPixelShader().getOutputStream(HlslShader::BtInput);
	fpi << L"float4 Position : SV_Position;" << Endl;

	return true;
}

struct Emitter
{
	virtual bool emit(HlslContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public Emitter
{
	typedef bool (*function_t)(HlslContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function) :
		m_function(function)
	{
	}

	virtual bool emit(HlslContext& c, Node* node)
	{
		T_ASSERT (is_a< NodeType >(node));
		return (*m_function)(c, static_cast< NodeType* >(node));
	}
};

		}

HlslEmitter::HlslEmitter()
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
	m_emitters[&type_of< IndexedUniform >()] = new EmitterCast< IndexedUniform >(emitIndexedUniform);
	m_emitters[&type_of< Interpolator >()] = new EmitterCast< Interpolator >(emitInterpolator);
	m_emitters[&type_of< Iterate >()] = new EmitterCast< Iterate >(emitIterate);
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
	m_emitters[&type_of< Round >()] = new EmitterCast< Round >(emitRound);
	m_emitters[&type_of< Sampler >()] = new EmitterCast< Sampler >(emitSampler);
	m_emitters[&type_of< Scalar >()] = new EmitterCast< Scalar >(emitScalar);
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

HlslEmitter::~HlslEmitter()
{
	for (std::map< const TypeInfo*, Emitter* >::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
		delete i->second;
}

bool HlslEmitter::emit(HlslContext& c, Node* node)
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
