#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Render/VertexElement.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/Editor/Hlsl/HlslContext.h"
#include "Render/Dx11/Editor/Hlsl/HlslEmitter.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Script.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

OutputStream& assign(OutputStream& f, const HlslVariable* out)
{
	f << hlsl_type_name(out->getType()) << L" " << out->getName() << L" = ";
	return f;
}

OutputStream& comment(OutputStream& f, const Node* node)
{
	if (!node->getComment().empty())
	{
		std::wstring comment = node->getComment();
		f << L"// " << comment << Endl;
	}
	return f;
}

bool emitAbs(HlslContext& cx, Abs* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	
	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"abs(" << in->getName() << L");" << Endl;

	return true;
}

bool emitAdd(HlslContext& cx, Add* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_precedence(in1->getType(), in2->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;

	return true;
}

bool emitArcusCos(HlslContext& cx, ArcusCos* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
	assign(f, out) << L"acos(" << theta->getName() << L");" << Endl;

	return true;
}

bool emitArcusTan(HlslContext& cx, ArcusTan* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > xy = cx.emitInput(node, L"XY");
	if (!xy || hlsl_type_width(xy->getType()) < 2)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
	assign(f, out) << L"atan2(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;

	return true;
}

bool emitClamp(HlslContext& cx, Clamp* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	if (node->getMin() == 0.0f && node->getMax() == 1.0f)
		assign(f, out) << L"saturate(" << in->getName() << L");" << Endl;
	else
		assign(f, out) << L"clamp(" << in->getName() << L", " << node->getMin() << L", " << node->getMax() << L");" << Endl;

	return true;
}

bool emitColor(HlslContext& cx, Color* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat4);
	if (!out)
		return false;

	Color4f color = node->getColor();
	if (!node->getLinear())
		color = color.linear();

	comment(f, node);		
	f << L"const float4 " << out->getName() << L" = float4(" << color.getRed() << L", " << color.getGreen() << L", " << color.getBlue() << L", " << color.getAlpha() << L");" << Endl;

	return true;
}

bool emitComputeOutput(HlslContext& cx, ComputeOutput* node)
{
	cx.enterCompute();

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	return true;
}

bool emitConditional(HlslContext& cx, Conditional* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	// Emit input and reference branches.
	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	Ref< HlslVariable > ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	HlslVariable caseTrue, caseFalse;
	std::wstring caseTrueBranch, caseFalseBranch;

	// Find common output pins from both sides of branch;
	// emit those before condition in order to have them evaluated outside of conditional.
	AlignedVector< const OutputPin* > outputPins;
	cx.findCommonOutputs(node, L"CaseTrue", L"CaseFalse", outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtVoid);

	// Emit true branch.
	{
		auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
		cx.getShader().pushScope();

		Ref< HlslVariable > ct = cx.emitInput(node, L"CaseTrue");
		if (!ct)
			return false;

		caseTrue = *ct;
		caseTrueBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(HlslShader::BtBody);
	}

	// Emit false branch.
	{
		auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
		cx.getShader().pushScope();

		Ref< HlslVariable > cf = cx.emitInput(node, L"CaseFalse");
		if (!cf)
			return false;

		caseFalse = *cf;
		caseFalseBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(HlslShader::BtBody);
	}

	// Modify output variable; need to have input variable ready as it will determine output type.
	HlslType outputType = hlsl_precedence(caseTrue.getType(), caseFalse.getType());
	out->setType(outputType);

	comment(f, node);
	f << hlsl_type_name(out->getType()) << L" " << out->getName() << L";" << Endl;

	if (node->getBranch() == Conditional::BrStatic)
		f << L"[flatten]" << Endl;
	else if (node->getBranch() == Conditional::BrDynamic)
		f << L"[branch]" << Endl;

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
		T_ASSERT(0);
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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
	assign(f, out) << L"cos(" << theta->getName() << L");" << Endl;

	return true;
}

bool emitCross(HlslContext& cx, Cross* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat3);

	comment(f, node);
	assign(f, out) << L"cross(" << in1->cast(HtFloat3) << L", " << in2->cast(HtFloat3) << L");" << Endl;

	return true;
}

bool emitDerivative(HlslContext& cx, Derivative* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > input = cx.emitInput(node, L"Input");
	if (!input)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", input->getType());

	comment(f, node);
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

bool emitDiscard(HlslContext& cx, Discard* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	// Emit input and reference branches.
	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	Ref< HlslVariable > ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	f << L"[branch]" << Endl;

	// Create condition statement.
	comment(f, node);
	switch (node->getOperator())
	{
	case Discard::CoLess:
		f << L"if (" << in->getName() << L" >= " << ref->getName() << L")" << Endl;
		break;
	case Discard::CoLessEqual:
		f << L"if (" << in->getName() << L" > " << ref->getName() << L")" << Endl;
		break;
	case Discard::CoEqual:
		f << L"if (" << in->getName() << L" != " << ref->getName() << L")" << Endl;
		break;
	case Discard::CoNotEqual:
		f << L"if (" << in->getName() << L" == " << ref->getName() << L")" << Endl;
		break;
	case Discard::CoGreater:
		f << L"if (" << in->getName() << L" <= " << ref->getName() << L")" << Endl;
		break;
	case Discard::CoGreaterEqual:
		f << L"if (" << in->getName() << L" < " << ref->getName() << L")" << Endl;
		break;
	default:
		T_ASSERT(0);
	}

	f << L"\tdiscard;" << Endl;

	Ref< HlslVariable > pass = cx.emitInput(node, L"Pass");
	if (!pass)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", pass->getType());
	assign(f, out) << pass->getName() << L";" << Endl;

	return true;
}

bool emitDiv(HlslContext& cx, Div* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_promote_to_float(hlsl_precedence(in1->getType(), in2->getType()));

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;

	return true;
}

bool emitDot(HlslContext& cx, Dot* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);
	HlslType type = hlsl_precedence(in1->getType(), in2->getType());

	comment(f, node);
	assign(f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;

	return true;
}

bool emitExp(HlslContext& cx, Exp* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"exp(" << in->getName() << L");" << Endl;

	return true;
}

bool emitFraction(HlslContext& cx, Fraction* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"frac(" << in->getName() << L");" << Endl;

	return true;
}

bool emitFragmentPosition(HlslContext& cx, FragmentPosition* node)
{
	if (!cx.inPixel())
		return false;

	cx.getShader().allocateVPos();

	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat2);

	comment(f, node);
	assign(f, out) << L"vPos;" << Endl;

	return true;
}

bool emitFrontFace(HlslContext& cx, FrontFace* node)
{
	if (!cx.inPixel())
		return false;

	cx.getShader().allocateVFace();

	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
	assign(f, out) << L"vFace ? 1.0f : 0.0f;" << Endl;

	return true;
}

bool emitIndexedUniform(HlslContext& cx, IndexedUniform* node)
{
	const Node* indexNode = cx.getInputNode(node, L"Index");
	if (!indexNode)
		return false;

	Ref< HlslVariable > out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		hlsl_from_parameter_type(node->getParameterType())
	);

	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	if (const Scalar* scalarIndexNode = dynamic_type_cast< const Scalar* >(indexNode))
	{
		comment(f, node);
		assign(f, out) << node->getParameterName() << L"[" << int32_t(scalarIndexNode->get()) << L"];" << Endl;
	}
	else
	{
		Ref< HlslVariable > index = cx.emitInput(node, L"Index");
		if (!index)
			return false;

		comment(f, node);
		assign(f, out) << node->getParameterName() << L"[" << index->cast(HtInteger) << L"];" << Endl;
	}

	const auto& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		const HlslShader::BlockType c_blockType[] = { HlslShader::BtCBufferOnce, HlslShader::BtCBufferFrame, HlslShader::BtCBufferDraw };
		auto& fu = cx.getShader().getOutputStream(c_blockType[(int32_t)node->getFrequency()]);
		fu << hlsl_type_name(out->getType()) << L" " << node->getParameterName() << L"[" << node->getLength() << L"];" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}

	return true;
}

bool emitInstance(HlslContext& cx, Instance* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);
	
	comment(f, node);
	assign(f, out) << L"float(instanceID);" << Endl;

	cx.getShader().allocateInstanceID();
	return true;
}

bool emitInterpolator(HlslContext& cx, Interpolator* node)
{
	if (!cx.inPixel())
	{
		// We're already in vertex state; skip interpolation.
		Ref< HlslVariable > in = cx.emitInput(node, L"Input");
		if (!in)
			return false;

		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

		auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

		comment(f, node);
		assign(f, out) << in->getName() << L";" << Endl;

		return true;
	}

	cx.enterVertex();

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	cx.enterPixel();

	int32_t interpolatorWidth = hlsl_type_width(in->getType());
	if (!interpolatorWidth)
		return false;

	int32_t interpolatorId;
	int32_t interpolatorOffset;
	bool declare = cx.allocateInterpolator(interpolatorWidth, interpolatorId, interpolatorOffset);

	std::wstring interpolatorName = L"Attr" + toString(interpolatorId);
	std::wstring interpolatorMask = interpolatorName + L"." + std::wstring(L"xyzw").substr(interpolatorOffset, interpolatorWidth);

	auto& f = cx.getVertexShader().getOutputStream(HlslShader::BtBody);

	comment(f, node);
	f << L"o." << interpolatorMask << L" = " << in->getName() << L";" << Endl;

	cx.getPixelShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		L"i." + interpolatorMask,
		in->getType()
	);

	if (declare)
	{
		auto& vfo = cx.getVertexShader().getOutputStream(HlslShader::BtOutput);
		auto& pfi = cx.getPixelShader().getOutputStream(HlslShader::BtInput);

		vfo << L"float4 " << interpolatorName << L" : TEXCOORD" << interpolatorId << L";" << Endl;
		pfi << L"float4 " << interpolatorName << L" : TEXCOORD" << interpolatorId << L";" << Endl;
	}

	return true;
}

bool emitIterate(HlslContext& cx, Iterate* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< HlslVariable > N = cx.emitOutput(node, L"N", HtInteger);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtVoid);
	T_ASSERT(out);

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	AlignedVector< const OutputPin* > outputPins;
	AlignedVector< const OutputPin* > dependentOutputPins(2);
	dependentOutputPins[0] = node->findOutputPin(L"N");
	dependentOutputPins[1] = node->findOutputPin(L"Output");
	cx.findNonDependentOutputs(node, L"Input", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	// Write input branch in a temporary output stream.
	auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	{
		Ref< HlslVariable > input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		// Emit post condition if connected; break iteration if condition is false.
		Ref< HlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(HtInteger) << L" == 0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(HlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< HlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	if (cx.inPixel())
		f << L"[unroll]" << Endl;
	f << L"for (int " << N->getName() << L" = " << node->getFrom() << L"; " << N->getName() << L" <= " << node->getTo() << L"; ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << inner;
	f << out->getName() << L" = " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;

	return true;
}

bool emitIterate2(HlslContext& cx, Iterate2* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	std::wstring inputNames[4];

	// Create iterator variable.
	Ref< HlslVariable > N = cx.emitOutput(node, L"N", HtInteger);
	T_ASSERT(N);

	// Create void output variables; change type later when we know
	// the type of the input branches.
	Ref< HlslVariable > out[] =
	{
		cx.getInputNode(node, L"Input0") != nullptr ? cx.emitOutput(node, L"Output0", HtVoid) : nullptr,
		cx.getInputNode(node, L"Input1") != nullptr ? cx.emitOutput(node, L"Output1", HtVoid) : nullptr,
		cx.getInputNode(node, L"Input2") != nullptr ? cx.emitOutput(node, L"Output2", HtVoid) : nullptr,
		cx.getInputNode(node, L"Input3") != nullptr ? cx.emitOutput(node, L"Output3", HtVoid) : nullptr,
	};
	if (!out[0])
		return false;

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	AlignedVector< const OutputPin* > dependentOutputPins(5);
	AlignedVector< const OutputPin* > outputPins;

	dependentOutputPins[0] = node->findOutputPin(L"N");
	dependentOutputPins[1] = node->findOutputPin(L"Output0");
	dependentOutputPins[2] = node->findOutputPin(L"Output1");
	dependentOutputPins[3] = node->findOutputPin(L"Output2");
	dependentOutputPins[4] = node->findOutputPin(L"Output3");

	outputPins.resize(0);
	cx.findNonDependentOutputs(node, L"Input0", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	outputPins.resize(0);
	cx.findNonDependentOutputs(node, L"Input1", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	outputPins.resize(0);
	cx.findNonDependentOutputs(node, L"Input2", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	outputPins.resize(0);
	cx.findNonDependentOutputs(node, L"Input3", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	// Write input branch in a temporary output stream.
	auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	{
		Ref< HlslVariable > input[] =
		{
			cx.emitInput(node, L"Input0"),
			cx.emitInput(node, L"Input1"),
			cx.emitInput(node, L"Input2"),
			cx.emitInput(node, L"Input3")
		};
		if (!input[0])
		{
			cx.getShader().popScope();
			cx.getShader().popOutputStream(HlslShader::BtBody);
			return false;
		}

		// Emit post condition if connected; break iteration if condition is false.
		Ref< HlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(HtInteger) << L" == 0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputNames[0] = input[0]->getName();
		inputNames[1] = input[1] != nullptr ? input[1]->getName() : L"";
		inputNames[2] = input[2] != nullptr ? input[2]->getName() : L"";
		inputNames[3] = input[3] != nullptr ? input[3]->getName() : L"";

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		*out[0] = HlslVariable(out[0]->getNode(), out[0]->getName(), input[0]->getType());
		if (input[1])
			*out[1] = HlslVariable(out[1]->getNode(), out[1]->getName(), input[1]->getType());
		if (input[2])
			*out[2] = HlslVariable(out[2]->getNode(), out[2]->getName(), input[2]->getType());
		if (input[3])
			*out[3] = HlslVariable(out[3]->getNode(), out[3]->getName(), input[3]->getType());
	}

	cx.getShader().popScope();

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(HlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< HlslVariable > initial[] =
	{
		out[0] != nullptr ? cx.emitInput(node, L"Initial0") : nullptr,
		out[1] != nullptr ? cx.emitInput(node, L"Initial1") : nullptr,
		out[2] != nullptr ? cx.emitInput(node, L"Initial2") : nullptr,
		out[3] != nullptr ? cx.emitInput(node, L"Initial3") : nullptr
	};
	for (int32_t i = 0; i < 4; ++i)
	{
		if (out[i] && initial[i])
			assign(f, out[i]) << initial[i]->cast(out[i]->getType()) << L";" << Endl;
		else if (out[i])
			assign(f, out[i]) << L"0;" << Endl;
	}

	// Write outer for-loop statement.
	StringOutputStream ss;
	ss << L"for (int " << N->getName() << L" =";
	if (Scalar* scalarFrom = dynamic_type_cast< Scalar* >(cx.getInputNode(node, L"From")))
		ss << (int32_t)scalarFrom->get();
	else
	{
		Ref< HlslVariable > from = cx.emitInput(node, L"From");
		if (from)
			ss << from->cast(HtInteger);
		else
			return false;
	}
	ss << L"; " << N->getName() << L" <= ";
	if (Scalar* scalarTo = dynamic_type_cast< Scalar* >(cx.getInputNode(node, L"To")))
		ss << (int32_t)scalarTo->get();
	else
	{
		Ref< HlslVariable > to = cx.emitInput(node, L"To");
		if (to)
			ss << to->cast(HtInteger);
		else
			return false;
	}
	f << ss.str() << L"; ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << inner;

	if (out[0])
		f << out[0]->getName() << L" = " << inputNames[0] << L";" << Endl;
	if (out[1])
		f << out[1]->getName() << L" = " << inputNames[1] << L";" << Endl;
	if (out[2])
		f << out[2]->getName() << L" = " << inputNames[2] << L";" << Endl;
	if (out[3])
		f << out[3]->getName() << L" = " << inputNames[3] << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;

	return true;
}

bool emitIterate2d(HlslContext& cx, Iterate2d* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variables.
	Ref< HlslVariable > X = cx.emitOutput(node, L"X", HtInteger);
	T_ASSERT(X);

	Ref< HlslVariable > Y = cx.emitOutput(node, L"Y", HtInteger);
	T_ASSERT(Y);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtVoid);
	T_ASSERT(out);

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	AlignedVector< const OutputPin* > outputPins;
	AlignedVector< const OutputPin* > dependentOutputPins(3);
	dependentOutputPins[0] = node->findOutputPin(L"X");
	dependentOutputPins[1] = node->findOutputPin(L"Y");
	dependentOutputPins[2] = node->findOutputPin(L"Output");
	cx.findNonDependentOutputs(node, L"Input", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	// Write input branch in a temporary output stream.
	auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	Ref< HlslVariable > input = cx.emitInput(node, L"Input");
	if (!input)
		return false;

	// Emit post condition if connected; break iteration if condition is false.
	Ref< HlslVariable > condition = cx.emitInput(node, L"Condition");
	if (condition)
	{
		fs << L"if (" << condition->cast(HtInteger) << L" == 0)" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	inputName = input->getName();

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	*out = HlslVariable(out->getNode(), out->getName(), input->getType());

	cx.getShader().popScope();

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(HlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< HlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << L"0;" << Endl;

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
	f << inner;
	f << out->getName() << L" = " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;

	// Emit outer loop post condition.
	if (condition)
	{
		fs << L"if (" << condition->cast(HtInteger) << L" == 0)" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	f << DecreaseIndent;
	f << L"}" << Endl;

	return true;
}

bool emitLength(HlslContext& cx, Length* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
	assign(f, out) << L"length(" << in->getName() << L");" << Endl;

	return true;
}

bool emitLerp(HlslContext& cx, Lerp* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_promote_to_float(hlsl_precedence(in1->getType(), in2->getType()));
	if (type == HtVoid)
		return false;

	Ref< HlslVariable > blend = cx.emitInput(node, L"Blend");
	if (!blend || blend->getType() != HtFloat)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"lerp(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;

	return true;
}

bool emitLog(HlslContext& cx, Log* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > xaxis = cx.emitInput(node, L"XAxis");
	Ref< HlslVariable > yaxis = cx.emitInput(node, L"YAxis");
	Ref< HlslVariable > zaxis = cx.emitInput(node, L"ZAxis");
	Ref< HlslVariable > translate = cx.emitInput(node, L"Translate");

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat4x4);

	comment(f, node);
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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	comment(f, node);

	Ref< HlslVariable > xaxis = cx.emitOutput(node, L"XAxis", HtFloat4);
	if (xaxis)
		assign(f, xaxis) << in->getName() << L"._11_21_31_41;" << Endl;

	Ref< HlslVariable > yaxis = cx.emitOutput(node, L"YAxis", HtFloat4);
	if (yaxis)
		assign(f, yaxis) << in->getName() << L"._12_22_32_42;" << Endl;

	Ref< HlslVariable > zaxis = cx.emitOutput(node, L"ZAxis", HtFloat4);
	if (zaxis)
		assign(f, zaxis) << in->getName() << L"._13_23_33_43;" << Endl;

	Ref< HlslVariable > translate = cx.emitOutput(node, L"Translate", HtFloat4);
	if (translate)
		assign(f, translate) << in->getName() << L"._14_24_34_44;" << Endl;

	return true;
}

bool emitMax(HlslContext& cx, Max* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_precedence(in1->getType(), in2->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"max(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;

	return true;
}

bool emitMin(HlslContext& cx, Min* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_precedence(in1->getType(), in2->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;

	return true;
}

bool emitMixIn(HlslContext& cx, MixIn* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > x = cx.emitInput(node, L"X");
	Ref< HlslVariable > y = cx.emitInput(node, L"Y");
	Ref< HlslVariable > z = cx.emitInput(node, L"Z");
	Ref< HlslVariable > w = cx.emitInput(node, L"W");

	comment(f, node);
	if (!y && !z && !w)
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);
		assign(f, out) << L"float(" << (x ? x->getName() : L"0.0f") << L");" << Endl;
	}
	else if (!z && !w)
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat2);
		assign(f, out) << L"float2(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L");" << Endl;
	}
	else if (!w)
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat3);
		assign(f, out) << L"float3(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L", " << (z ? z->getName() : L"0.0f") << L");" << Endl;
	}
	else
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat4);
		assign(f, out) << L"float4(" << (x ? x->getName() : L"0.0f") << L", " << (y ? y->getName() : L"0.0f") << L", " << (z ? z->getName() : L"0.0f") << L", " << (w ? w->getName() : L"0.0f") << L");" << Endl;
	}

	return true;
}

bool emitMixOut(HlslContext& cx, MixOut* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	comment(f, node);
	switch (in->getType())
	{
	case HtFloat:
		{
			Ref< HlslVariable > x = cx.emitOutput(node, L"X", HtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
		}
		break;

	case HtFloat2:
		{
			Ref< HlslVariable > x = cx.emitOutput(node, L"X", HtFloat);
			Ref< HlslVariable > y = cx.emitOutput(node, L"Y", HtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
		}
		break;

	case HtFloat3:
		{
			Ref< HlslVariable > x = cx.emitOutput(node, L"X", HtFloat);
			Ref< HlslVariable > y = cx.emitOutput(node, L"Y", HtFloat);
			Ref< HlslVariable > z = cx.emitOutput(node, L"Z", HtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
			assign(f, z) << in->getName() << L".z;" << Endl;
		}
		break;

	case HtFloat4:
		{
			Ref< HlslVariable > x = cx.emitOutput(node, L"X", HtFloat);
			Ref< HlslVariable > y = cx.emitOutput(node, L"Y", HtFloat);
			Ref< HlslVariable > z = cx.emitOutput(node, L"Z", HtFloat);
			Ref< HlslVariable > w = cx.emitOutput(node, L"W", HtFloat);
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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_precedence(in1->getType(), in2->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;

	return true;
}

bool emitMulAdd(HlslContext& cx, MulAdd* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	Ref< HlslVariable > in3 = cx.emitInput(node, L"Input3");
	if (!in1 || !in2 || !in3)
		return false;

	HlslType type = hlsl_precedence(hlsl_precedence(in1->getType(), in2->getType()), in3->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L" + " << in3->cast(type) << L";" << Endl;

	return true;
}

bool emitNeg(HlslContext& cx, Neg* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"-" << in->getName() << L";" << Endl;

	return true;
}

bool emitNormalize(HlslContext& cx, Normalize* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"normalize(" << in->getName() << L");" << Endl;

	return true;
}

bool emitPixelOutput(HlslContext& cx, PixelOutput* node)
{
	const D3D11_CULL_MODE d3dCullMode[] =
	{
		D3D11_CULL_NONE,
		D3D11_CULL_FRONT,
		D3D11_CULL_BACK
	};

	const D3D11_BLEND_OP d3dBlendOperation[] =
	{
		D3D11_BLEND_OP_ADD,
		D3D11_BLEND_OP_SUBTRACT,
		D3D11_BLEND_OP_REV_SUBTRACT,
		D3D11_BLEND_OP_MIN,
		D3D11_BLEND_OP_MAX
	};

	const D3D11_BLEND d3dBlendFactor[] =
	{
		D3D11_BLEND_ONE,
		D3D11_BLEND_ZERO,
		D3D11_BLEND_SRC_COLOR,
		D3D11_BLEND_INV_SRC_COLOR,
		D3D11_BLEND_DEST_COLOR,
		D3D11_BLEND_INV_DEST_COLOR,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA
	};

	const D3D11_COMPARISON_FUNC d3dCompareFunction[] =
	{
		D3D11_COMPARISON_ALWAYS,
		D3D11_COMPARISON_NEVER,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_GREATER_EQUAL,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_NOT_EQUAL
	};

	const D3D11_STENCIL_OP d3dStencilOperation[] =
	{
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_ZERO,
		D3D11_STENCIL_OP_REPLACE,
		D3D11_STENCIL_OP_INCR_SAT,
		D3D11_STENCIL_OP_DECR_SAT,
		D3D11_STENCIL_OP_INVERT,
		D3D11_STENCIL_OP_INCR,
		D3D11_STENCIL_OP_DECR
	};

	RenderState rs = node->getRenderState();

	cx.enterPixel();

	const wchar_t* inputs[] = { L"Input", L"Input1", L"Input2", L"Input3" };
	Ref< HlslVariable > in[4];

	for (int32_t i = 0; i < sizeof_array(in); ++i)
		in[i] = cx.emitInput(node, inputs[i]);

	if (!in[0])
		return false;

	for (int32_t i = 0; i < sizeof_array(in); ++i)
	{
		if (!in[i])
			continue;

		auto& fpo = cx.getPixelShader().getOutputStream(HlslShader::BtOutput);
		fpo << L"half4 Color" << i << L" : SV_Target" << i << L";" << Endl;

		auto& fpb = cx.getPixelShader().getOutputStream(HlslShader::BtBody);
		fpb << L"half4 out_Color" << i << L" = " << in[i]->cast(HtFloat4) << L";" << Endl;

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

	cx.getD3DRasterizerDesc().FillMode = rs.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	cx.getD3DRasterizerDesc().CullMode = d3dCullMode[(int32_t)rs.cullMode];

	cx.getD3DDepthStencilDesc().DepthEnable = rs.depthEnable ? TRUE : FALSE;
	cx.getD3DDepthStencilDesc().DepthWriteMask = rs.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	cx.getD3DDepthStencilDesc().DepthFunc = rs.depthEnable ? d3dCompareFunction[rs.depthFunction] : D3D11_COMPARISON_ALWAYS;

	if (rs.stencilEnable)
	{
		cx.getD3DDepthStencilDesc().StencilEnable = TRUE;
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
	}
	else
	{
		cx.getD3DDepthStencilDesc().StencilEnable = FALSE;
		cx.getD3DDepthStencilDesc().StencilReadMask = 0xff;
		cx.getD3DDepthStencilDesc().StencilWriteMask = 0xff;
		cx.getD3DDepthStencilDesc().FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		cx.getD3DDepthStencilDesc().FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		cx.getD3DDepthStencilDesc().FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		cx.getD3DDepthStencilDesc().FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		cx.getD3DDepthStencilDesc().BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		cx.getD3DDepthStencilDesc().BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		cx.getD3DDepthStencilDesc().BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		cx.getD3DDepthStencilDesc().BackFace.StencilFunc =D3D11_COMPARISON_ALWAYS;
		cx.setStencilReference(0);
	}

	cx.getD3DBlendDesc().AlphaToCoverageEnable = rs.alphaToCoverageEnable ? TRUE : FALSE;
	cx.getD3DBlendDesc().IndependentBlendEnable = FALSE;

	if (rs.blendEnable)
	{
		cx.getD3DBlendDesc().RenderTarget[0].BlendEnable = TRUE;
		cx.getD3DBlendDesc().RenderTarget[0].SrcBlend = d3dBlendFactor[rs.blendColorSource];
		cx.getD3DBlendDesc().RenderTarget[0].DestBlend = d3dBlendFactor[rs.blendColorDestination];
		cx.getD3DBlendDesc().RenderTarget[0].BlendOp = d3dBlendOperation[rs.blendColorOperation];
		cx.getD3DBlendDesc().RenderTarget[0].SrcBlendAlpha = d3dBlendFactor[rs.blendAlphaSource];
		cx.getD3DBlendDesc().RenderTarget[0].DestBlendAlpha = d3dBlendFactor[rs.blendAlphaDestination];
		cx.getD3DBlendDesc().RenderTarget[0].BlendOpAlpha = d3dBlendOperation[rs.blendAlphaOperation];
	}
	else
	{
		cx.getD3DBlendDesc().RenderTarget[0].BlendEnable = FALSE;
		cx.getD3DBlendDesc().RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		cx.getD3DBlendDesc().RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		cx.getD3DBlendDesc().RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		cx.getD3DBlendDesc().RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		cx.getD3DBlendDesc().RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		cx.getD3DBlendDesc().RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	}

	UINT8 d3dWriteMask = 0;
	if (rs.colorWriteMask & CwRed)
		d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;
	if (rs.colorWriteMask & CwGreen)
		d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
	if (rs.colorWriteMask & CwBlue)
		d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
	if (rs.colorWriteMask & CwAlpha)
		d3dWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

	for (int32_t i = 0; i < sizeof_array(in); ++i)
	{
		if (in[i])
			cx.getD3DBlendDesc().RenderTarget[i].RenderTargetWriteMask = d3dWriteMask;
	}

	return true;
}

bool emitPolynomial(HlslContext& cx, Polynomial* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > x = cx.emitInput(node, L"X");
	Ref< HlslVariable > coeffs = cx.emitInput(node, L"Coefficients");
	if (!x || !coeffs)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	Ref< HlslVariable > exponent = cx.emitInput(node, L"Exponent");

	const Node* inputNode = cx.getInputNode(node, L"Input");
	if (!inputNode)
		return false;

	if (const Scalar* inputScalar = dynamic_type_cast< const Scalar* >(inputNode))
	{
		if (abs(inputScalar->get() - 2.0f) < FUZZY_EPSILON)
		{
			// 2 as base; emit exp2 intrinsic instead of pow as it's more efficient.
			HlslType type = hlsl_precedence(exponent->getType(), HtFloat);
			Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);
			assign(f, out) << L"exp2(" << exponent->cast(type) << L");" << Endl;
			return true;
		}
		else if (abs(inputScalar->get() - 2.718f) < FUZZY_EPSILON)
		{
			// e as base; emit exp intrinsic instead of pow as it's more efficient.
			HlslType type = hlsl_precedence(exponent->getType(), HtFloat);
			Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);
			assign(f, out) << L"exp(" << exponent->cast(type) << L");" << Endl;
			return true;
		}
	}

	// Non-trivial base.
	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!exponent || !in)
		return false;

	HlslType type = hlsl_precedence(exponent->getType(), in->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"pow(" << in->cast(type) << L", " << exponent->cast(type) << L");" << Endl;

	return true;
}

bool emitReadStruct(HlslContext& cx, ReadStruct* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > strct = cx.emitInput(node, L"Struct");
	if (!strct || strct->getType() != HtStructBuffer)
		return false;

	const Struct* sn = mandatory_non_null_type_cast< const Struct* >(strct->getNode());
	DataType type = sn->getElementType(node->getName());

	Ref< HlslVariable > index = cx.emitInput(node, L"Index");
	if (!index)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", hlsl_from_data_type(type));

	comment(f, node);

	if (type == DtHalf2)
	{
		Ref< HlslVariable > tmp = cx.getShader().createTemporaryVariable(nullptr, HtVoid);
		f << L"const uint " << tmp->getName() << L" = " << strct->getName() << L"[" << index->cast(HtInteger) << L"]." << node->getName() << L";" << Endl;
		assign(f, out) <<
			L"float2(" <<
				L"f16tof32(" << tmp->getName() << L" & 0xffff), " <<
				L"f16tof32(" << tmp->getName() << L" >> 16)" <<
			L");" << Endl;
	}
	else if (type == DtHalf4)
	{
		Ref< HlslVariable > tmp = cx.getShader().createTemporaryVariable(nullptr, HtVoid);
		f << L"const uint2 " << tmp->getName() << L" = " << strct->getName() << L"[" << index->cast(HtInteger) << L"]." << node->getName() << L";" << Endl;
		assign(f, out) <<
			L"float4(" <<
				L"f16tof32(" << tmp->getName() << L".x & 0xffff), " <<
				L"f16tof32(" << tmp->getName() << L".x >> 16), " <<
				L"f16tof32(" << tmp->getName() << L".y & 0xffff), " <<
				L"f16tof32(" << tmp->getName() << L".y >> 16)" <<
			L");" << Endl;
	}
	else
		assign(f, out) << strct->getName() << L"[" << index->cast(HtInteger) << L"]." << node->getName() << L";" << Endl;

	return true;
}

bool emitReflect(HlslContext& cx, Reflect* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > normal = cx.emitInput(node, L"Normal");
	Ref< HlslVariable > direction = cx.emitInput(node, L"Direction");
	if (!normal || !direction)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", direction->getType());

	comment(f, node);
	assign(f, out) << L"reflect(" << direction->getName() << L", " << normal->cast(direction->getType()) << L");" << Endl;

	return true;
}

bool emitRecipSqrt(HlslContext& cx, RecipSqrt* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"rsqrt(" << in->getName() << L");" << Endl;

	return true;
}

bool emitRepeat(HlslContext& cx, Repeat* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< HlslVariable > N = cx.emitOutput(node, L"N", HtInteger);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtVoid);
	T_ASSERT(out);

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	AlignedVector< const OutputPin* > outputPins;
	AlignedVector< const OutputPin* > dependentOutputPins(2);
	dependentOutputPins[0] = node->findOutputPin(L"N");
	dependentOutputPins[1] = node->findOutputPin(L"Output");
	cx.findNonDependentOutputs(node, L"Input", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	// Write input branch in a temporary output stream.
	auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	{
		// Emit pre-condition, break iteration if condition is false.
		Ref< HlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(HtInteger) << L" == 0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		Ref< HlslVariable > input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		*out = HlslVariable(out->getNode(), out->getName(), input->getType());
	}

	cx.getShader().popScope();

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(HlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< HlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	f << L"for (int " << N->getName() << L" = 0.0f;; ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << inner;
	f << out->getName() << L" = " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;

	return true;
}

bool emitRound(HlslContext& cx, Round* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"round(" << in->getName() << L");" << Endl;

	return true;
}

bool emitSampler(HlslContext& cx, Sampler* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > texture = cx.emitInput(node, L"Texture");
	if (!texture || texture->getType() < HtTexture2D)
		return false;

	Ref< HlslVariable > texCoord = cx.emitInput(node, L"TexCoord");
	if (!texCoord)
		return false;

	Ref< HlslVariable > mip = cx.emitInput(node, L"Mip");

	// Define sampler class.
	const D3D11_TEXTURE_ADDRESS_MODE c_d3dAddress[] =
	{
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_BORDER
	};

	const D3D11_COMPARISON_FUNC c_d3dComparison[] =
	{
		D3D11_COMPARISON_ALWAYS,
		D3D11_COMPARISON_NEVER,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_GREATER_EQUAL,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_NOT_EQUAL,
		D3D11_COMPARISON_NEVER
	};

	const SamplerState& samplerState = node->getSamplerState();

	D3D11_SAMPLER_DESC dsd;
	std::memset(&dsd, 0, sizeof(dsd));

	dsd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	dsd.AddressU = c_d3dAddress[samplerState.addressU];
	dsd.AddressV = c_d3dAddress[samplerState.addressV];
	if (texture->getType() > HtTexture2D)
		dsd.AddressW = c_d3dAddress[samplerState.addressW];
	else
		dsd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	dsd.MipLODBias = samplerState.mipBias;
	dsd.MaxAnisotropy = 1;
	dsd.ComparisonFunc = c_d3dComparison[samplerState.compare];
	dsd.BorderColor[0] =
	dsd.BorderColor[1] =
	dsd.BorderColor[2] =
	dsd.BorderColor[3] = 1.0f;
	dsd.MinLOD = -D3D11_FLOAT32_MAX;
	dsd.MaxLOD =  D3D11_FLOAT32_MAX;

	switch (samplerState.mipFilter)
	{
	case FtPoint:
		break;
	case FtLinear:
		(UINT&)dsd.Filter |= 0x1;
		break;
	}

	switch (samplerState.magFilter)
	{
	case FtPoint:
		break;
	case FtLinear:
		(UINT&)dsd.Filter |= 0x4;
		break;
	}

	switch (samplerState.minFilter)
	{
	case FtPoint:
		break;
	case FtLinear:
		(UINT&)dsd.Filter |= 0x10;
		break;
	}

	if (samplerState.compare == CfNone)
	{
		if (
			samplerState.useAnisotropic &&
			dsd.Filter == (0x10 | 0x4 | 0x1)
		)
			dsd.Filter = D3D11_FILTER_ANISOTROPIC;
	}
	else
		(UINT&)dsd.Filter |= 0x80;

	Murmur3 samplerHash;
	samplerHash.begin();
	samplerHash.feed(&dsd, sizeof(dsd));
	samplerHash.end();

	std::wstring samplerName = L"S" + toString(samplerHash.get()) + L"_samplerState";
	std::wstring textureName = texture->getName();

	const std::map< std::wstring, D3D11_SAMPLER_DESC >& samplers = cx.getShader().getSamplers();
	if (samplers.find(samplerName) == samplers.end())
	{
		auto& fu = cx.getShader().getOutputStream(HlslShader::BtSamplers);

		if (samplerState.compare == CfNone)
			fu << L"SamplerState " << samplerName << L";" << Endl;
		else
			fu << L"SamplerComparisonState " << samplerName << L";" << Endl;

		cx.getShader().addSampler(samplerName, dsd);
	}

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", (samplerState.compare == CfNone) ? HtFloat4 : HtFloat);

	if (!mip && cx.inPixel() && !samplerState.ignoreMips)
	{
		if (samplerState.compare == CfNone)
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
		else
		{
			if (!samplerState.ignoreMips)
			{
				switch (texture->getType())
				{
				case HtTexture2D:
					assign(f, out) << textureName << L".SampleCmp(" << samplerName << L", " << texCoord->cast(HtFloat2) << L", " << texCoord->cast(HtFloat3) << L".z);" << Endl;
					break;
				case HtTexture3D:
				case HtTextureCube:
					assign(f, out) << textureName << L".SampleCmp(" << samplerName << L", " << texCoord->cast(HtFloat3) << L", " << texCoord->cast(HtFloat4) << L".w);" << Endl;
					break;
				}
			}
			else
			{
				switch (texture->getType())
				{
				case HtTexture2D:
					assign(f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(HtFloat2) << L", " << texCoord->cast(HtFloat3) << L".z);" << Endl;
					break;
				case HtTexture3D:
				case HtTextureCube:
					assign(f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(HtFloat3) << L", " << texCoord->cast(HtFloat4) << L".w);" << Endl;
					break;
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
			case HtTexture2D:
				assign(f, out) << textureName << L".SampleLevel(" << samplerName << L", " << texCoord->cast(HtFloat2) << L", " << (mip ? mip->cast(HtFloat) : L"0.0f") << L");" << Endl;
				break;
			case HtTexture3D:
			case HtTextureCube:
				assign(f, out) << textureName << L".SampleLevel(" << samplerName << L", " << texCoord->cast(HtFloat3) << L", " << (mip ? mip->cast(HtFloat) : L"0.0f") << L");" << Endl;
				break;
			}
		}
		else
		{
			switch (texture->getType())
			{
			case HtTexture2D:
				assign(f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(HtFloat2) << L", " << texCoord->cast(HtFloat3) << L".z);" << Endl;
				break;
			case HtTexture3D:
			case HtTextureCube:
				assign(f, out) << textureName << L".SampleCmpLevelZero(" << samplerName << L", " << texCoord->cast(HtFloat3) << L", " << texCoord->cast(HtFloat4) << L".w);" << Endl;
				break;
			}
		}
	}

	return true;
}

bool emitScalar(HlslContext& cx, Scalar* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	comment(f, node);

	float v = std::abs(node->get());
	if ((v - std::floor(v)) < FUZZY_EPSILON)
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtInteger);
		f << L"const int " << out->getName() << L" = " << (int32_t)node->get() << L";" << Endl;
	}
	else
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);
		f << L"const float " << out->getName() << L" = " << node->get() << L";" << Endl;
	}

	return true;
}

bool emitSign(HlslContext& cx, Sign* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"sign(" << in->getName() << L");" << Endl;

	return true;
}

bool emitScript(HlslContext& cx, Script* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	const std::wstring& script = node->getScript();
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
		T_ASSERT(outputPin);

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
		StringOutputStream ss;

		ss << L"void " << node->getName() << L"(";
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			if (i > 0)
				ss << L", ";

			if (in[i]->getType() != HtStructBuffer)
				ss << hlsl_type_name(in[i]->getType()) << L" " << node->getInputPin(i)->getName();
			else
				ss << L"StructuredBuffer< SBufferData0 > " << node->getInputPin(i)->getName();
		}
		if (!in.empty())
			ss << L", ";
		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			if (i > 0)
				ss << L", ";
			ss << L"out " << hlsl_type_name(out[i]->getType()) << L" " << node->getOutputPin(i)->getName();
		}
		ss << L")";

		std::wstring processedScript = replaceAll(script, L"ENTRY", ss.str());
		T_ASSERT(!processedScript.empty());

		auto& fs = cx.getShader().getOutputStream(HlslShader::BtScript);
		fs << processedScript << Endl;
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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
	assign(f, out) << L"sin(" << theta->getName() << L");" << Endl;

	return true;
}

bool emitSqrt(HlslContext& cx, Sqrt* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"sqrt(" << in->getName() << L");" << Endl;

	return true;
}

bool emitStep(HlslContext& cx, Step* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"X");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Y");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_precedence(in1->getType(), in2->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"step(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;

	return true;
}

bool emitStruct(HlslContext& cx, Struct* node)
{
	Ref< HlslVariable > out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		HtStructBuffer
	);

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		auto& fs = cx.getShader().getOutputStream(HlslShader::BtStructs);
		fs << L"struct S_" << node->getParameterName() << Endl;
		fs << L"{" << Endl;
		for (const auto& element : node->getElements())
			fs << L"\t" << hlsl_storage_type(element.type) << L" " << element.name << L";" << Endl;
		fs << L"};" << Endl;
		fs << Endl;
		fs << L"StructuredBuffer< S_" << node->getParameterName() << L" > " << node->getParameterName() << L";" << Endl;

		cx.getShader().addUniform(node->getParameterName());
	}
	return true;
}

bool emitSub(HlslContext& cx, Sub* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< HlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	HlslType type = hlsl_precedence(in1->getType(), in2->getType());
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;

	return true;
}

bool emitSum(HlslContext& cx, Sum* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< HlslVariable > N = cx.emitOutput(node, L"N", HtInteger);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtVoid);
	T_ASSERT(out);

	// Find non-dependent, external, output pins from input branch;
	// we emit those first in order to have them evaluated
	// outside of iteration.
	AlignedVector< const OutputPin* > outputPins;
	AlignedVector< const OutputPin* > dependentOutputPins(2);
	dependentOutputPins[0] = node->findOutputPin(L"N");
	dependentOutputPins[1] = node->findOutputPin(L"Output");
	cx.findNonDependentOutputs(node, L"Input", dependentOutputPins, outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	// Write input branch in a temporary output stream.
	auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	{
		Ref< HlslVariable > input = cx.emitInput(node, L"Input");
		if (!input)
		{
			cx.getShader().popScope();
			cx.getShader().popOutputStream(HlslShader::BtBody);
			return false;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		*out = HlslVariable(out->getNode(), out->getName(), input->getType());
	}

	cx.getShader().popScope();

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(HlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	comment(f, node);
	assign(f, out) << L"0;" << Endl;

	// Write outer for-loop statement.
	if (cx.inPixel())
		f << L"[unroll]" << Endl;
	f << L"for (int " << N->getName() << L" = " << node->getFrom() << L"; " << N->getName() << L" <= " << node->getTo() << L"; ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << inner;
	f << out->getName() << L" += " << inputName << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;

	return true;
}

bool emitSwizzle(HlslContext& cx, Swizzle* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	std::wstring map = toLower(node->get());
	if (map.length() == 0)
		return false;

	const HlslType types[] = { HtFloat, HtFloat2, HtFloat3, HtFloat4 };
	HlslType type = types[map.length() - 1];

	// Check if input is a constant Vector node; thus pack directly instead of swizzle.
	const Vector* constVector = dynamic_type_cast< const Vector* >(cx.getInputNode(node, L"Input"));
	if (constVector)
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);
		StringOutputStream ss;
		ss << hlsl_type_name(type) << L"(";
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
		assign(f, out) << ss.str() << L";" << Endl;
		return true;
	}

	// Not constant input; need to evaluate input further.
	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	//if (
	//	(map == L"xyzw" && in->getType() == HtFloat4) ||
	//	(map == L"xyz" && in->getType() == HtFloat3) ||
	//	(map == L"xy" && in->getType() == HtFloat2) ||
	//	(map == L"x" && in->getType() == HtFloat)
	//)
	//{
	//	// No need to swizzle; pass variable further.
	//	cx.emitOutput(node, L"Output", in);
	//}
	//else
	{
		Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Select");
	if (!in)
		return false;

	const auto& cases = node->getCases();
	const int32_t width = node->getWidth();

	// Find common output pins from both sides of switch;
	// emit those before condition in order to have them evaluated outside of conditional.
	//
	// Do not extract common output pins from default cases since
	// they are considered "unlikely" and will most probably divert from
	// regular cases in such way that no common output pins are found.

	for (int32_t channel = 0; channel < width; ++channel)
	{
		AlignedVector< const InputPin* > caseInputPins;
		for (uint32_t i = 0; i < (uint32_t)cases.size(); ++i)
		{
			const InputPin* caseInput = node->getInputPin(1 + width + i * width + channel);
			T_ASSERT(caseInput);
			caseInputPins.push_back(caseInput);
		}

		AlignedVector< const OutputPin* > outputPins;
		cx.findCommonOutputs(caseInputPins, outputPins);
		for (auto outputPin : outputPins)
			cx.emit(outputPin->getNode());
	}

	AlignedVector< std::wstring > caseBranches;
	AlignedVector< HlslVariable > caseInputs;
	AlignedVector< std::wstring > defaultBranches;
	AlignedVector< HlslVariable > defaultInputs;
	AlignedVector< HlslType > outputTypes;
	RefArray< HlslVariable > outs;

	// Emit output variables first due to scoping.
	for (int32_t i = 0; i < width; ++i)
	{
		const OutputPin* outputPin = node->getOutputPin(i);
		outs.push_back(cx.emitOutput(outputPin, HtVoid));
		outputTypes.push_back(HtVoid);
	}

	// Conditional branches.
	for (int32_t i = 0; i < (int32_t)cases.size(); ++i)
	{
		cx.getShader().pushScope();
		for (int32_t j = 0; j < width; ++j)
		{
			auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);

			const InputPin* caseInput = node->getInputPin(1 + width + i * width + j);
			if (!caseInput)
			{
				cx.getShader().popScope();
				return false;
			}

			Ref< HlslVariable > caseInputVariable = cx.emitInput(caseInput);
			if (!caseInputVariable)
			{
				cx.getShader().popScope();
				return false;
			}

			caseBranches.push_back(fs.str());
			caseInputs.push_back(*caseInputVariable);
			outputTypes[j] = std::max(outputTypes[j], caseInputVariable->getType());

			cx.getShader().popOutputStream(HlslShader::BtBody);
		}
		cx.getShader().popScope();
	}

	// Default branches.
	{
		cx.getShader().pushScope();
		for (int32_t i = 0; i < width; ++i)
		{
			auto& fs = cx.getShader().pushOutputStream(HlslShader::BtBody, T_FILE_LINE_W);
	
			const InputPin* defaultInput = node->getInputPin(1 + i);
			if (!defaultInput)
			{
				cx.getShader().popScope();
				return false;
			}

			Ref< HlslVariable > defaultInputVariable = cx.emitInput(defaultInput);
			if (!defaultInputVariable)
			{
				cx.getShader().popScope();
				return false;
			}

			defaultBranches.push_back(fs.str());
			defaultInputs.push_back(*defaultInputVariable);
			outputTypes[i] = std::max(outputTypes[i], defaultInputVariable->getType());

			cx.getShader().popOutputStream(HlslShader::BtBody);
		}
		cx.getShader().popScope();
	}

	// Modify output type to match common output type of cases,
	// initialize output variable to zero.
	for (int32_t i = 0; i < width; ++i)
	{
		outs[i]->setType(outputTypes[i]);
		assign(f, outs[i]) << L"0;" << Endl;
	}

	comment(f, node);

	if (node->getBranch() == Switch::BrStatic)
		f << L"[flatten]" << Endl;
	else if (node->getBranch() == Switch::BrDynamic)
		f << L"[branch]" << Endl;

	for (int32_t i = 0; i < (int32_t)cases.size(); ++i)
	{
		f << (i == 0 ? L"if (" : L"else if (") << in->cast(HtInteger) << L" == " << cases[i] << L")" << Endl;
		f << L"{" << Endl;
		f << IncreaseIndent;

		for (int32_t j = 0; j < width; ++j)
		{
			f << caseBranches[i * width + j];
			f << outs[j]->getName() << L" = " << caseInputs[i * width + j].cast(outputTypes[j]) << L";" << Endl;
		}

		f << DecreaseIndent;
		f << L"}" << Endl;
	}

	if (!cases.empty())
	{
		f << L"else" << Endl;
		f << L"{" << Endl;
		f << IncreaseIndent;
	}

	for (int32_t i = 0; i < width; ++i)
	{
		f << defaultBranches[i];
		f << outs[i]->getName() << L" = " << defaultInputs[i].cast(outputTypes[i]) << L";" << Endl;
	}

	if (!cases.empty())
	{
		f << DecreaseIndent;
		f << L"}" << Endl;
	}

	return true;
}

bool emitTan(HlslContext& cx, Tan* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != HtFloat)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat);

	comment(f, node);
	assign(f, out) << L"tan(" << theta->getName() << L");" << Endl;

	return true;
}

bool emitTargetSize(HlslContext& cx, TargetSize* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat2);

	comment(f, node);
	assign(f, out) << L"_dx11_targetSize.xy;" << Endl;

	cx.getShader().allocateTargetSize();
	return true;
}

bool emitTextureSize(HlslContext& cx, TextureSize* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	std::wstring textureName = in->getName();

	Ref< HlslVariable > out;

	comment(f, node);
	switch (in->getType())
	{
	case HtTexture2D:
		out = cx.emitOutput(node, L"Output", HtFloat2);
		f << L"float2 " << out->getName() << L"; " << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y);" << Endl;
		break;

	case HtTexture3D:
		out = cx.emitOutput(node, L"Output", HtFloat3);
		f << L"float3 " << out->getName() << L"; " << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y, " << out->getName() << L".z);" << Endl;
		break;

	case HtTextureCube:
		out = cx.emitOutput(node, L"Output", HtFloat3);
		f << L"float3 " << out->getName() << L"; " << textureName << L".GetDimensions(" << out->getName() << L".x, " << out->getName() << L".y, " << out->getName() << L".z);" << Endl;
		break;

	default:
		return false;
	}

	return true;
}

bool emitTransform(HlslContext& cx, Transform* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	Ref< HlslVariable > transform = cx.emitInput(node, L"Transform");
	if (!in || !transform)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat4);

	comment(f, node);
	assign(f, out) << L"mul(" << transform->getName() << L", " << in->cast(HtFloat4) << L");" << Endl;

	return true;
}

bool emitTranspose(HlslContext& cx, Transpose* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"transpose(" << in->getName() << L");" << Endl;

	return true;
}

bool emitTruncate(HlslContext& cx, Truncate* node)
{
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	HlslType type = hlsl_degrade_to_integer(in->getType());
	if (type == HtVoid)
		return false;

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << hlsl_type_name(type) << L"(trunc(" << in->getName() << L"));" << Endl;

	return true;
}

bool emitUniform(HlslContext& cx, Uniform* node)
{
	Ref< HlslVariable > out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		hlsl_from_parameter_type(node->getParameterType())
	);

	const auto& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		if (out->getType() < HtTexture2D)
		{
			const HlslShader::BlockType c_blockType[] = { HlslShader::BtCBufferOnce, HlslShader::BtCBufferFrame, HlslShader::BtCBufferDraw };
			auto& fu = cx.getShader().getOutputStream(c_blockType[(int32_t)node->getFrequency()]);
			fu << hlsl_type_name(out->getType()) << L" " << node->getParameterName() << L";" << Endl;
		}
		else
		{
			auto& fu = cx.getShader().getOutputStream(HlslShader::BtTextures);
			switch (node->getParameterType())
			{
			case ParameterType::Texture2D:
				fu << L"Texture2D " << node->getParameterName() << L";" << Endl;
				break;

			case ParameterType::Texture3D:
				fu << L"Texture3D " << node->getParameterName() << L";" << Endl;
				break;

			case ParameterType::TextureCube:
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
	auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);

	Ref< HlslVariable > out = cx.emitOutput(node, L"Output", HtFloat4);
	if (!out)
		return false;

	comment(f, node);
	assign(f, out) << L"float4(" << node->get().x() << L", " << node->get().y() << L", " << node->get().z() << L", " << node->get().w() << L");" << Endl;

	return true;
}

bool emitVertexInput(HlslContext& cx, VertexInput* node)
{
	if (!cx.inVertex())
		return false;

	HlslShader& shader = cx.getShader();
	HlslType type = hlsl_from_data_type(node->getDataType());

	// Declare input variable.
	if (!shader.haveInput(node->getName()))
	{
		std::wstring semantic = hlsl_semantic(node->getDataUsage(), node->getIndex());

		auto& fi = shader.getOutputStream(HlslShader::BtInput);
		fi << hlsl_type_name(type) << L" " << node->getName() << L" : " << semantic << L";" << Endl;

		shader.addInput(node->getName());
	}

	// Read value from input.
	if (node->getDataUsage() == DuPosition)
	{
		Ref< HlslVariable > out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			HtFloat4
		);
		auto& f = shader.getOutputStream(HlslShader::BtBody);
		switch (type)
		{
		case HtFloat:
			assign(f, out) << L"float4(i." << node->getName() << L".x, 0.0f, 0.0f, 1.0f);" << Endl;
			break;

		case HtFloat2:
			assign(f, out) << L"float4(i." << node->getName() << L".xy, 0.0f, 1.0f);" << Endl;
			break;

		case HtFloat3:
			assign(f, out) << L"float4(i." << node->getName() << L".xyz, 1.0f);" << Endl;
			break;

		default:
			assign(f, out) << L"i." << node->getName() << L";" << Endl;
			break;
		}
	}
	else if (node->getDataUsage() == DuNormal)
	{
		Ref< HlslVariable > out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			HtFloat4
		);
		auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
		switch (type)
		{
		case HtFloat:
			assign(f, out) << L"float4(i." << node->getName() << L".x, 0.0f, 0.0f, 0.0f);" << Endl;
			break;

		case HtFloat2:
			assign(f, out) << L"float4(i." << node->getName() << L".xy, 0.0f, 0.0f);" << Endl;
			break;

		case HtFloat3:
			assign(f, out) << L"float4(i." << node->getName() << L".xyz, 0.0f);" << Endl;
			break;

		default:
			assign(f, out) << L"i." << node->getName() << L";" << Endl;
			break;
		}
	}
	else
	{
		Ref< HlslVariable > out = shader.createTemporaryVariable(
			node->findOutputPin(L"Output"),
			type
		);
		auto& f = cx.getShader().getOutputStream(HlslShader::BtBody);
		assign(f, out) << L"i." << node->getName() << L";" << Endl;
	}

	return true;
}

bool emitVertexOutput(HlslContext& cx, VertexOutput* node)
{
	cx.enterVertex();

	Ref< HlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	auto& fo = cx.getVertexShader().getOutputStream(HlslShader::BtOutput);
	fo << L"float4 Position : SV_Position;" << Endl;

	auto& fb = cx.getVertexShader().getOutputStream(HlslShader::BtBody);
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

	return true;
}

struct Emitter
{
	virtual ~Emitter() {}

	virtual bool emit(HlslContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public Emitter
{
	typedef bool (*function_t)(HlslContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function)
	:	m_function(function)
	{
	}

	virtual bool emit(HlslContext& c, Node* node) override final
	{
		return (*m_function)(c, mandatory_non_null_type_cast< NodeType* >(node));
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
	m_emitters[&type_of< ComputeOutput >()] = new EmitterCast< ComputeOutput >(emitComputeOutput);
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
	m_emitters[&type_of< Iterate2 >()] = new EmitterCast< Iterate2 >(emitIterate2);
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
	m_emitters[&type_of< ReadStruct >()] = new EmitterCast< ReadStruct >(emitReadStruct);
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
	m_emitters[&type_of< Struct >()] = new EmitterCast< Struct >(emitStruct);
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
	for (auto emitter : m_emitters)
		delete emitter.second;
}

bool HlslEmitter::emit(HlslContext& c, Node* node)
{
	auto i = m_emitters.find(&type_of(node));
	if (i == m_emitters.end() || i->second == nullptr)
	{
		log::error << L"No emitter for node " << type_name(node) << L"." << Endl;
		return false;
	}
	return i->second->emit(c, node);
}

	}
}
