#include <cstring>
#include <iomanip>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Render/VertexElement.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Vulkan/VertexAttributesVk.h"
#include "Render/Vulkan/Editor/Glsl/GlslContext.h"
#include "Render/Vulkan/Editor/Glsl/GlslEmitter.h"
#include "Render/Vulkan/Editor/Glsl/GlslImage.h"
#include "Render/Vulkan/Editor/Glsl/GlslSampler.h"
#include "Render/Vulkan/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Vulkan/Editor/Glsl/GlslTexture.h"
#include "Render/Vulkan/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

std::wstring formatFloat(float v)
{
	std::wstring s = toString(v);
	if (s.find(L'.') == s.npos)
		s += L".0";
	return s;
}

std::wstring expandScalar(float v, GlslType type)
{
	std::wstring vs = formatFloat(v);
	switch (type)
	{
	case GtFloat2:
		return L"vec2(" + vs + L", " + vs + L")";

	case GtFloat3:
		return L"vec3(" + vs + L", " + vs + L", " + vs + L")";

	case GtFloat4:
		return L"vec4(" + vs + L", " + vs + L", " + vs + L", " + vs + L")";

	default:
		break;
	}
	return vs;
}

OutputStream& assign(OutputStream& f, Ref< GlslVariable > out)
{
	f << glsl_type_name(out->getType()) << L" " << out->getName() << L" = ";
	return f;
}

bool emitAbs(GlslContext& cx, Abs* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"abs(" << in->getName() << L");" << Endl;
	return true;
}

bool emitAdd(GlslContext& cx, Add* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitArcusCos(GlslContext& cx, ArcusCos* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"acos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitArcusTan(GlslContext& cx, ArcusTan* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > xy = cx.emitInput(node, L"XY");
	if (!xy || xy->getType() != GtFloat2)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"atan(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;
	return true;
}

bool emitClamp(GlslContext& cx, Clamp* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"clamp(" << in->getName() << L", " << formatFloat(node->getMin()) << L", " << formatFloat(node->getMax()) << L");" << Endl;
	return true;
}

bool emitColor(GlslContext& cx, Color* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat4);
	if (!out)
		return false;
	Vector4 value = node->getColor();
	assign(f, out) << L"vec4(" << formatFloat(value.x()) << L", " << formatFloat(value.y()) << L", " << formatFloat(value.z()) << L", " << formatFloat(value.w()) << L");" << Endl;
	return true;
}

bool emitComputeOutput(GlslContext& cx, ComputeOutput* node)
{
	cx.enterCompute();

	// Manually handle uniform set for "Storage" input as we need to treat it
	// as an image rather than texture.
	const Uniform* storageUniformNode = dynamic_type_cast< const Uniform* >(cx.getInputNode(node, L"Storage"));
	if (!storageUniformNode)
		return false;
	if (storageUniformNode->getParameterType() != PtTexture2D)
		return false;

	auto existing = cx.getLayout().get(storageUniformNode->getParameterName());
	if (existing != nullptr)
	{
		auto existingImage = dynamic_type_cast< const GlslImage* >(existing);
		if (!existingImage)
			return false;
	}
	else
	{
		// Image do not exist; add new image resource.
		cx.getLayout().add(new GlslImage(
			storageUniformNode->getParameterName()
		));
	}

	GlslVariable* offset = cx.emitInput(node, L"Offset");
	if (!offset)
		return false;

	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;


	//auto& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
	//fu << L"layout(rgba32f, binding = 0) uniform image2D " << storage->getName() << L";" << Endl;

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	f << L"imageStore(" << storageUniformNode->getParameterName() << L", " << offset->castToInteger(GtFloat2) << L", " << in->cast(GtFloat4) << L");" << Endl;

	// Define parameter in context.
	cx.addParameter(
		storageUniformNode->getParameterName(),
		PtTexture2D,
		1,
		UfDraw
	);

	return true;
}

bool emitConditional(GlslContext& cx, Conditional* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Emit input and reference branches.
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	Ref< GlslVariable > ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	GlslVariable caseTrue, caseFalse;
	std::wstring caseTrueBranch, caseFalseBranch;

	// Find common output pins from both sides of branch;
	// emit those before condition in order to have them evaluated outside of conditional.
	AlignedVector< const OutputPin* > outputPins;
	cx.findCommonOutputs(node, L"CaseTrue", L"CaseFalse", outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	// Emit true branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		Ref< GlslVariable > ct = cx.emitInput(node, L"CaseTrue");
		if (!ct)
			return false;

		caseTrue = *ct;
		caseTrueBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	// Emit false branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		Ref< GlslVariable > cf = cx.emitInput(node, L"CaseFalse");
		if (!cf)
			return false;

		caseFalse = *cf;
		caseFalseBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	// Create output variable.
	GlslType outputType = std::max< GlslType >(caseTrue.getType(), caseFalse.getType());

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", outputType);
	f << glsl_type_name(out->getType()) << L" " << out->getName() << L";" << Endl;

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

bool emitCos(GlslContext& cx, Cos* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"cos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitCross(GlslContext& cx, Cross* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat3);
	assign(f, out) << L"cross(" << in1->cast(GtFloat3) << L", " << in2->cast(GtFloat3) << L");" << Endl;
	return true;
}

bool emitDerivative(GlslContext& cx, Derivative* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > input = cx.emitInput(node, L"Input");
	if (!input)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", input->getType());
	switch (node->getAxis())
	{
	case Derivative::DaX:
		assign(f, out) << L"dFdx(" << input->getName() << L");" << Endl;
		break;
	case Derivative::DaY:
		assign(f, out) << L"dFdy(" << input->getName() << L");" << Endl;
		break;
	}
	return true;
}

bool emitDiscard(GlslContext& cx, Discard* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Emit input and reference branches.
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	Ref< GlslVariable > ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

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
		T_ASSERT(0);
	}

	f << L"\tdiscard;" << Endl;

	Ref< GlslVariable > pass = cx.emitInput(node, L"Pass");
	if (!pass)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", pass->getType());
	assign(f, out) << pass->getName() << L";" << Endl;

	return true;
}

bool emitDiv(GlslContext& cx, Div* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitDot(GlslContext& cx, Dot* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	assign(f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitExp(GlslContext& cx, Exp* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"exp(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFraction(GlslContext& cx, Fraction* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"fract(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFragmentPosition(GlslContext& cx, FragmentPosition* node)
{
	if (!cx.inFragment())
		return false;

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat2);
	assign(f, out) << L"gl_FragCoord.xy;" << Endl;

	return true;
}

bool emitFrontFace(GlslContext& cx, FrontFace* node)
{
	if (!cx.inFragment())
		return false;

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"gl_FrontFacing ? 1.0 : 0.0;" << Endl;

	return true;
}

bool emitIndexedUniform(GlslContext& cx, IndexedUniform* node)
{
	const Node* indexNode = cx.getInputNode(node, L"Index");
	if (!indexNode)
		return false;

	Ref< GlslVariable > out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		glsl_from_parameter_type(node->getParameterType())
	);

	auto& fb = cx.getShader().getOutputStream(GlslShader::BtBody);

	if (const Scalar* scalarIndexNode = dynamic_type_cast< const Scalar* >(indexNode))
		assign(fb, out) << node->getParameterName() << L"[" << int32_t(scalarIndexNode->get()) << L"];" << Endl;
	else
	{
		Ref< GlslVariable > index = cx.emitInput(node, L"Index");
		if (!index)
			return false;

		assign(fb, out) << node->getParameterName() << L"[int(" << index->getName() << L")];" << Endl;
	}

	//if (!cx.getShader().haveUniform(node->getParameterName()))
	//{
	//	auto& fu = cx.getShader().getOutputStream(c_blockType[node->getFrequency()]);
	//	fu << glsl_type_name(out->getType()) << L" " << node->getParameterName() << L"[" << node->getLength() << L"];" << Endl;
	//	cx.getShader().addUniform(node->getParameterName());
	//}

	// Add uniform to layout.
	if (out->getType() < GtTexture2D)
	{
		auto ub = cx.getLayout().get< GlslUniformBuffer >(node->getFrequency());
		ub->add(
			node->getParameterName(),
			out->getType(),
			node->getLength()
		);
	}
	else
		return false;

	// Define parameter in context.
	cx.addParameter(
		node->getParameterName(),
		node->getParameterType(),
		node->getLength(),
		node->getFrequency()
	);
	return true;
}

bool emitInstance(GlslContext& cx, Instance* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"float(gl_InstanceID);" << Endl;
	return true;
}

bool emitInterpolator(GlslContext& cx, Interpolator* node)
{
	if (!cx.inFragment())
	{
		// We're already in vertex state; skip interpolation.
		Ref< GlslVariable > in = cx.emitInput(node, L"Input");
		if (!in)
		{
			log::error << L"Unable to emit interpolator input (1)" << Endl;
			return false;
		}

		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

		auto& fb = cx.getShader().getOutputStream(GlslShader::BtBody);
		assign(fb, out) << in->getName() << L";" << Endl;

		return true;
	}

	cx.enterVertex();

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
	{
		log::error << L"Unable to emit interpolator input (2)" << Endl;
		return false;
	}

	cx.enterFragment();

	int32_t interpolatorWidth = glsl_type_width(in->getType());
	if (!interpolatorWidth)
	{
		log::error << L"Unable to determine width of type " << int32_t(in->getType()) << Endl;
		return false;
	}

	int32_t interpolatorId;
	int32_t interpolatorOffset;

	bool declare = cx.allocateInterpolator(interpolatorWidth, interpolatorId, interpolatorOffset);

	std::wstring interpolatorName = L"Attr" + toString(interpolatorId);
	std::wstring interpolatorMask = interpolatorName + L"." + std::wstring(L"xyzw").substr(interpolatorOffset, interpolatorWidth);

	auto& fb = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
	fb << interpolatorMask << L" = " << in->getName() << L";" << Endl;

	cx.getFragmentShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		interpolatorMask,
		in->getType()
	);

	if (declare)
	{
		auto& fvo = cx.getVertexShader().getOutputStream(GlslShader::BtOutput);
		auto& fpi = cx.getFragmentShader().getOutputStream(GlslShader::BtInput);

		fvo << L"layout (location = " << interpolatorId << L") out vec4 " << interpolatorName << L";" << Endl;
		fpi << L"layout (location = " << interpolatorId << L") in vec4 " << interpolatorName << L";" << Endl;
	}

	return true;
}

bool emitIterate(GlslContext& cx, Iterate* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtVoid);
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
	StringOutputStream fs;
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		Ref< GlslVariable > input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		// Emit post condition if connected; break iteration if condition is false.
		Ref< GlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(GtFloat) << L" == 0.0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		*out = GlslVariable(out->getNode(), out->getName(), input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< GlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << N->getName() << L" = " << node->getFrom() << L".0; " << N->getName() << L" <= " << node->getTo() << L".0; ++" << N->getName() << L")" << Endl;
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

bool emitIterate2(GlslContext& cx, Iterate2* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputNames[4];

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT(N);

	// Create void output variables; change type later when we know
	// the type of the input branches.
	Ref< GlslVariable > out[] =
	{
		cx.getInputNode(node, L"Input0") != nullptr ? cx.emitOutput(node, L"Output0", GtVoid) : nullptr,
		cx.getInputNode(node, L"Input1") != nullptr ? cx.emitOutput(node, L"Output1", GtVoid) : nullptr,
		cx.getInputNode(node, L"Input2") != nullptr ? cx.emitOutput(node, L"Output2", GtVoid) : nullptr,
		cx.getInputNode(node, L"Input3") != nullptr ? cx.emitOutput(node, L"Output3", GtVoid) : nullptr,
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
	StringOutputStream fs;
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		Ref< GlslVariable > input[] =
		{
			cx.emitInput(node, L"Input0"),
			cx.emitInput(node, L"Input1"),
			cx.emitInput(node, L"Input2"),
			cx.emitInput(node, L"Input3")
		};
		if (!input[0])
		{
			cx.getShader().popScope();
			cx.getShader().popOutputStream(GlslShader::BtBody);
			return false;
		}

		// Emit post condition if connected; break iteration if condition is false.
		Ref< GlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (!(bool)" << condition->cast(GtFloat) << L")" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputNames[0] = input[0]->getName();
		inputNames[1] = input[1] != nullptr ? input[1]->getName() : L"";
		inputNames[2] = input[2] != nullptr ? input[2]->getName() : L"";
		inputNames[3] = input[3] != nullptr ? input[3]->getName() : L"";

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		*out[0] = GlslVariable(out[0]->getNode(), out[0]->getName(), input[0]->getType());
		if (input[1])
			*out[1] = GlslVariable(out[1]->getNode(), out[1]->getName(), input[1]->getType());
		if (input[2])
			*out[2] = GlslVariable(out[2]->getNode(), out[2]->getName(), input[2]->getType());
		if (input[3])
			*out[3] = GlslVariable(out[3]->getNode(), out[3]->getName(), input[3]->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< GlslVariable > initial[] =
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
			assign(f, out[i]) << expandScalar(0.0f, out[i]->getType()) << L";" << Endl;
	}

	// Write outer for-loop statement.
	Ref< GlslVariable > from = cx.emitInput(node, L"From");
	Ref< GlslVariable > to = cx.emitInput(node, L"To");
	if (!from || !to)
		return false;

	f << L"for (int " << N->getName() << L" = int(" << from->cast(GtFloat) << L"); " << N->getName() << L" <= int(" << to->cast(GtFloat) << L"); ++" << N->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	// Insert input branch here; it's already been generated in a temporary
	// output stream.
	f << fs.str();

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

bool emitIterate2d(GlslContext& cx, Iterate2d* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variables.
	Ref< GlslVariable > X = cx.emitOutput(node, L"X", GtFloat);
	T_ASSERT(X);

	Ref< GlslVariable > Y = cx.emitOutput(node, L"Y", GtFloat);
	T_ASSERT(Y);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtVoid);
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
	StringOutputStream fs;
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	Ref< GlslVariable > input = cx.emitInput(node, L"Input");
	if (!input)
		return false;

	// Emit post condition if connected; break iteration if condition is false.
	Ref< GlslVariable > condition = cx.emitInput(node, L"Condition");
	if (condition)
	{
		fs << L"if (" << condition->cast(GtFloat) << L" == 0.0)" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	inputName = input->getName();

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	*out = GlslVariable(out->getNode(), out->getName(), input->getType());

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< GlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << X->getName() << L" = " << node->getFromX() << L".0; " << X->getName() << L" <= " << node->getToX() << L".0; ++" << X->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	f << L"for (float " << Y->getName() << L" = " << node->getFromY() << L".0; " << Y->getName() << L" <= " << node->getToY() << L".0; ++" << Y->getName() << L")" << Endl;
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
		fs << L"if (" << condition->cast(GtFloat) << L" == 0.0)" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	f << DecreaseIndent;
	f << L"}" << Endl;

	return true;
}

bool emitLength(GlslContext& cx, Length* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"length(" << in->getName() << L");" << Endl;
	return true;
}

bool emitLerp(GlslContext& cx, Lerp* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > blend = cx.emitInput(node, L"Blend");
	if (!blend || blend->getType() != GtFloat)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"mix(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;
	return true;
}

bool emitLog(GlslContext& cx, Log* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	switch (node->getBase())
	{
	case Log::LbTwo:
		assign(f, out) << L"log2(" << in->getName() << L");" << Endl;
		break;

	case Log::LbTen:
		T_ASSERT_M (0, L"Log::LbTen not available in GLSL");
		break;

	case Log::LbNatural:
		assign(f, out) << L"log(" << in->getName() << L");" << Endl;
		break;
	}
	return true;
}

bool emitMatrixIn(GlslContext& cx, MatrixIn* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > xaxis = cx.emitInput(node, L"XAxis");
	Ref< GlslVariable > yaxis = cx.emitInput(node, L"YAxis");
	Ref< GlslVariable > zaxis = cx.emitInput(node, L"ZAxis");
	Ref< GlslVariable > translate = cx.emitInput(node, L"Translate");
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat4x4);
	f << L"mat4 " << out->getName() << L" = transpose(mat4(" << Endl;
	f << IncreaseIndent;
	f << (xaxis     ? xaxis->cast(GtFloat4)     : L"vec4(1.0, 0.0, 0.0, 0.0)") << L"," << Endl;
	f << (yaxis     ? yaxis->cast(GtFloat4)     : L"vec4(0.0, 1.0, 0.0, 0.0)") << L"," << Endl;
	f << (zaxis     ? zaxis->cast(GtFloat4)     : L"vec4(0.0, 0.0, 1.0, 0.0)") << L"," << Endl;
	f << (translate ? translate->cast(GtFloat4) : L"vec4(0.0, 0.0, 0.0, 1.0)") << Endl;
	f << DecreaseIndent;
	f << L"));" << Endl;
	return true;
}

bool emitMatrixOut(GlslContext& cx, MatrixOut* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< GlslVariable > xaxis = cx.emitOutput(node, L"XAxis", GtFloat4);
	if (xaxis)
	{
		assign(f, xaxis) <<
			L"vec4(" <<
				in->getName() << L"[0][0], " <<
				in->getName() << L"[0][1], " <<
				in->getName() << L"[0][2], " <<
				in->getName() << L"[0][3]" <<
			L");" << Endl;
	}

	Ref< GlslVariable > yaxis = cx.emitOutput(node, L"YAxis", GtFloat4);
	if (yaxis)
	{
		assign(f, yaxis) <<
			L"vec4(" <<
				in->getName() << L"[1][0], " <<
				in->getName() << L"[1][1], " <<
				in->getName() << L"[1][2], " <<
				in->getName() << L"[1][3]" <<
			L");" << Endl;
	}

	Ref< GlslVariable > zaxis = cx.emitOutput(node, L"ZAxis", GtFloat4);
	if (zaxis)
	{
		assign(f, zaxis) <<
			L"vec4(" <<
				in->getName() << L"[2][0], " <<
				in->getName() << L"[2][1], " <<
				in->getName() << L"[2][2], " <<
				in->getName() << L"[2][3]" <<
			L");" << Endl;
	}

	Ref< GlslVariable > translate = cx.emitOutput(node, L"Translate", GtFloat4);
	if (translate)
	{
		assign(f, translate) <<
			L"vec4(" <<
				in->getName() << L"[3][0], " <<
				in->getName() << L"[3][1], " <<
				in->getName() << L"[3][2], " <<
				in->getName() << L"[3][3]" <<
			L");" << Endl;
	}

	return true;
}

bool emitMax(GlslContext& cx, Max* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"max(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMin(GlslContext& cx, Min* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMixIn(GlslContext& cx, MixIn* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > x = cx.emitInput(node, L"X");
	Ref< GlslVariable > y = cx.emitInput(node, L"Y");
	Ref< GlslVariable > z = cx.emitInput(node, L"Z");
	Ref< GlslVariable > w = cx.emitInput(node, L"W");

	if (!y && !z && !w)
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
		assign(f, out) << L"(" << (x ? x->getName() : L"0.0") << L");" << Endl;
	}
	else if (!z && !w)
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat2);
		assign(f, out) << L"vec2(" << (x ? x->getName() : L"0.0") << L", " << (y ? y->getName() : L"0.0") << L");" << Endl;
	}
	else if (!w)
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat3);
		assign(f, out) << L"vec3(" << (x ? x->getName() : L"0.0") << L", " << (y ? y->getName() : L"0.0") << L", " << (z ? z->getName() : L"0.0") << L");" << Endl;
	}
	else
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat4);
		assign(f, out) << L"vec4(" << (x ? x->getName() : L"0.0") << L", " << (y ? y->getName() : L"0.0") << L", " << (z ? z->getName() : L"0.0") << L", " << (w ? w->getName() : L"0.0") << L");" << Endl;
	}

	return true;
}

bool emitMixOut(GlslContext& cx, MixOut* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	switch (in->getType())
	{
	case GtFloat:
		{
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", GtFloat);
			assign(f, x) << in->getName() << L";" << Endl;
		}
		break;

	case GtFloat2:
		{
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", GtFloat);
			Ref< GlslVariable > y = cx.emitOutput(node, L"Y", GtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
		}
		break;

	case GtFloat3:
		{
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", GtFloat);
			Ref< GlslVariable > y = cx.emitOutput(node, L"Y", GtFloat);
			Ref< GlslVariable > z = cx.emitOutput(node, L"Z", GtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
			assign(f, z) << in->getName() << L".z;" << Endl;
		}
		break;

	case GtFloat4:
		{
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", GtFloat);
			Ref< GlslVariable > y = cx.emitOutput(node, L"Y", GtFloat);
			Ref< GlslVariable > z = cx.emitOutput(node, L"Z", GtFloat);
			Ref< GlslVariable > w = cx.emitOutput(node, L"W", GtFloat);
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

bool emitMul(GlslContext& cx, Mul* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitMulAdd(GlslContext& cx, MulAdd* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	Ref< GlslVariable > in3 = cx.emitInput(node, L"Input3");
	if (!in1 || !in2 || !in3)
		return false;
	GlslType type = std::max< GlslType >(std::max< GlslType >(in1->getType(), in2->getType()), in3->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L" + " << in3->cast(type) << L";" << Endl;
	return true;
}

bool emitNeg(GlslContext& cx, Neg* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"-" << in->getName() << L";" << Endl;
	return true;
}

bool emitNormalize(GlslContext& cx, Normalize* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"normalize(" << in->getName() << L");" << Endl;
	return true;
}

bool emitPixelOutput(GlslContext& cx, PixelOutput* node)
{
	RenderState rs = node->getRenderState();

	cx.enterFragment();

	const wchar_t* inputs[] = { L"Input", L"Input1", L"Input2", L"Input3" };
	Ref< GlslVariable > in[4];

	for (int32_t i = 0; i < sizeof_array(in); ++i)
		in[i] = cx.emitInput(node, inputs[i]);

	if (!in[0])
		return false;

	for (int32_t i = 0; i < sizeof_array(in); ++i)
	{
		if (!in[i])
			continue;

		auto& fpb = cx.getFragmentShader().getOutputStream(GlslShader::BtBody);
		fpb << L"_gl_FragData_" << i << L" = " << in[i]->cast(GtFloat4) << L";" << Endl;

		auto& fpo = cx.getFragmentShader().getOutputStream(GlslShader::BtOutput);
		fpo << L"layout (location = " << i << L") out vec4 _gl_FragData_" << i << L";" << Endl;
	}

	cx.setRenderState(rs);
	return true;
}

bool emitPolynomial(GlslContext& cx, Polynomial* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > x = cx.emitInput(node, L"X");
	Ref< GlslVariable > coeffs = cx.emitInput(node, L"Coefficients");
	if (!x || !coeffs)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);

	assign(f, out);
	switch (coeffs->getType())
	{
	case GtFloat:
		f << coeffs->getName() << L".x * " << x->getName();
		break;

	case GtFloat2:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".y * " << x->getName();
		break;

	case GtFloat3:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".z * " << x->getName();
		break;

	case GtFloat4:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 4) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".z * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".w * " << x->getName();
		break;

	default:
		break;
	}
	f << L";" << Endl;

	return true;
}

bool emitPow(GlslContext& cx, Pow* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > exponent = cx.emitInput(node, L"Exponent");
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!exponent || !in)
		return false;
	GlslType type = std::max< GlslType >(exponent->getType(), in->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"pow(max(" << in->cast(type) << L", 0.0), " << exponent->cast(type) << L");" << Endl;
	return true;
}

bool emitReadStruct(GlslContext& cx, ReadStruct* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > strct = cx.emitInput(node, L"Struct");
	if (!strct || strct->getType() != GtStructBuffer)
		return false;

	const Struct* sn = mandatory_non_null_type_cast< const Struct* >(strct->getNode());

	DataType type = sn->getElementType(node->getName());

	Ref< GlslVariable > index = cx.emitInput(node, L"Index");
	if (!index)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", glsl_from_data_type(type));
	assign(f, out) << strct->getName() << L"_Data[int(" << index->cast(GtFloat) << L")]." << node->getName() << L";" << Endl;
	return true;
}

bool emitReflect(GlslContext& cx, Reflect* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > normal = cx.emitInput(node, L"Normal");
	Ref< GlslVariable > direction = cx.emitInput(node, L"Direction");
	if (!normal || !direction)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", direction->getType());
	assign(f, out) << L"reflect(" << direction->getName() << L", " << normal->cast(direction->getType()) << L");" << Endl;
	return true;
}

bool emitRecipSqrt(GlslContext& cx, RecipSqrt* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"inversesqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitRepeat(GlslContext& cx, Repeat* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtVoid);
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
	StringOutputStream fs;
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		// Emit pre-condition, break iteration if condition is false.
		Ref< GlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(GtFloat) << L" == 0.0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		Ref< GlslVariable > input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		*out = GlslVariable(out->getNode(), out->getName(), input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< GlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assign(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assign(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << N->getName() << L" = 0.0;; ++" << N->getName() << L")" << Endl;
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

bool emitRound(GlslContext& cx, Round* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"round(" << in->getName() << L");" << Endl;
	return true;
}

bool emitSampler(GlslContext& cx, Sampler* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > texture = cx.emitInput(node, L"Texture");
	if (!texture || texture->getType() < GtTexture2D)
		return false;

	Ref< GlslVariable > texCoord = cx.emitInput(node, L"TexCoord");
	if (!texCoord)
		return false;

	Ref< GlslVariable > mip = cx.emitInput(node, L"Mip");

	const auto& samplerState = node->getSamplerState();

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", (samplerState.compare == CfNone) ? GtFloat4 : GtFloat);

	bool needAddressW = bool(texture->getType() > GtTexture2D);
	std::wstring samplerName;

	// Check if we already have a suitable sampler in the layout.
	for (auto resource : cx.getLayout().get())
	{
		if (const auto sampler = dynamic_type_cast< GlslSampler* >(resource))
		{
			if (std::memcmp(&sampler->getState(), &samplerState, sizeof(SamplerState)) == 0)
			{
				samplerName = sampler->getName();
				break;
			}
		}
	}

	// Define new sampler in layout if no matching one was found.
	if (samplerName.empty())
	{
		samplerName = L"_vk_sampler_" + toString(cx.getLayout().count< GlslSampler >());
		cx.getLayout().add(new GlslSampler(
			samplerName,
			samplerState
		));
	}

	if (cx.inFragment())
	{
		if (samplerState.compare == CfNone)
		{
			if (!mip && !samplerState.ignoreMips)
			{
				float bias = samplerState.mipBias;
				switch (texture->getType())
				{
				case GtTexture2D:
					if (std::abs(bias) < FUZZY_EPSILON)
						assign(f, out) << L"texture(sampler2D(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat2) << L");" << Endl;
					else
						assign(f, out) << L"texture(sampler2D(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat2) << L", " << formatFloat(bias) << L");" << Endl;
					break;

				case GtTexture3D:
					assign(f, out) << L"texture(sampler3D(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat3) << L");" << Endl;
					break;

				case GtTextureCube:
					assign(f, out) << L"texture(samplerCube(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat3) << L");" << Endl;
					break;

				default:
					return false;
				}
			}
			else
			{
				switch (texture->getType())
				{
				case GtTexture2D:
					assign(f, out) << L"textureLod(sampler2D(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat2) << L", " << (mip != nullptr ? mip->cast(GtFloat) : L"0.0") << L");" << Endl;
					break;

				case GtTexture3D:
					assign(f, out) << L"textureLod(sampler3D(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat3) << L", " << (mip != nullptr ? mip->cast(GtFloat) : L"0.0") << L");" << Endl;
					break;

				case GtTextureCube:
					assign(f, out) << L"textureLod(samplerCube(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat3) << L", " << (mip != nullptr ? mip->cast(GtFloat) : L"0.0") << L");" << Endl;
					break;

				default:
					return false;
				}
			}
		}
		else	// Compare
		{
			if (!mip && !samplerState.ignoreMips)
			{
				switch (texture->getType())
				{
				case GtTexture2D:
					assign(f, out) << L"texture(sampler2DShadow(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat3) << L");" << Endl;
					break;

				case GtTexture3D:
					assign(f, out) << L"texture(sampler3DShadow(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5)));" << Endl;
					break;

				case GtTextureCube:
					assign(f, out) << L"texture(samplerCubeShadow(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5)));" << Endl;
					break;

				default:
					return false;
				}
			}
			else
			{
				switch (texture->getType())
				{
				case GtTexture2D:
					assign(f, out) << L"textureLod(sampler2DShadow(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat3) << L", " << (mip != nullptr ? mip->cast(GtFloat) : L"0.0") << L");" << Endl;
					break;

				case GtTexture3D:
					assign(f, out) << L"textureLod(sampler3DShadow(" << texture->getName() << L", " << samplerName << L"), " <<  texCoord->cast(GtFloat4) << L", " << (mip != nullptr ? mip->cast(GtFloat) : L"0.0") << L");" << Endl;
					break;

				case GtTextureCube:
					assign(f, out) << L"textureLod(samplerCubeShadow(" << texture->getName() << L", " << samplerName << L"), " <<  texCoord->cast(GtFloat4) << L", " << (mip != nullptr ? mip->cast(GtFloat) : L"0.0") << L");" << Endl;
					break;

				default:
					return false;
				}
			}
		}
	}

	if (cx.inVertex())
	{
		switch (texture->getType())
		{
		case GtTexture2D:
			assign(f, out) << L"texture(sampler2D(" << texture->getName() << L", " << samplerName << L"), " << texCoord->cast(GtFloat3) << L", 0.0);" << Endl;
			break;

		case GtTexture3D:
		case GtTextureCube:
        default:
            return false;
		}
	}

	return true;
}

bool emitScalar(GlslContext& cx, Scalar* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	f << L"const float " << out->getName() << L" = " << formatFloat(node->get()) << L";" << Endl;
	return true;
}

bool emitSign(GlslContext& cx, Sign* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sign(" << in->getName() << L");" << Endl;
	return true;
}

bool emitScript(GlslContext& cx, Script* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Get platform specific script from node.
	std::wstring script = node->getScript();
	if (script.empty())
		return false;

	// Emit input and outputs.
	int32_t inputPinCount = node->getInputPinCount();
	int32_t outputPinCount = node->getOutputPinCount();

	const std::map< std::wstring, SamplerState >& samplers = node->getSamplers();

	RefArray< GlslVariable > ins(inputPinCount);
	RefArray< GlslVariable > outs(outputPinCount);

	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const TypedOutputPin* outputPin = static_cast< const TypedOutputPin* >(node->getOutputPin(i));
		T_ASSERT(outputPin);

		outs[i] = cx.emitOutput(
			node,
			outputPin->getName(),
			glsl_from_parameter_type(outputPin->getType())
		);
	}

	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		ins[i] = cx.emitInput(node->getInputPin(i));
		if (!ins[i])
			return false;

		if (node->getInputPinType(i) >= PtTexture2D)
		{
			std::wstring samplerId = node->getInputPinSamplerId(i);
			if (samplerId.empty())
				return false;

			auto it = samplers.find(samplerId);
			if (it == samplers.end())
				return false;

			const SamplerState& samplerState = it->second;

			//const GLenum c_glFilter[] =
			//{
			//	GL_NEAREST,
			//	GL_LINEAR
			//};

			//const GLenum c_glWrap[] =
			//{
			//	GL_REPEAT,
			//	GL_REPEAT,
			//	GL_CLAMP_TO_EDGE,
			//	GL_CLAMP_TO_EDGE
			//};

			//const GLenum c_glCompare[] =
			//{
			//	GL_ALWAYS,
			//	GL_NEVER,
			//	GL_LESS,
			//	GL_LEQUAL,
			//	GL_GREATER,
			//	GL_GEQUAL,
			//	GL_EQUAL,
			//	GL_NOTEQUAL,
			//	GL_INVALID_ENUM
			//};

			Adler32 samplerHash;
			samplerHash.feed(node->getInputPin(i)->getName());
			samplerHash.feed(samplerState.minFilter);
			samplerHash.feed(samplerState.mipFilter);
			samplerHash.feed(samplerState.magFilter);
			samplerHash.feed(samplerState.addressU);
			samplerHash.feed(samplerState.addressV);
			samplerHash.feed(samplerState.addressW);
			samplerHash.feed(samplerState.compare);

			// Define sampler.
			//bool defineStates = cx.defineSampler(samplerId, samplerHash.get(), GL_TEXTURE_2D, in[i]->getName(), stage);
			//if (defineStates)
			//{
			//	RenderStateOpenGL& rs = cx.getRenderState();

			//	bool minLinear = samplerState.minFilter != FtPoint;
			//	bool mipLinear = samplerState.mipFilter != FtPoint;

			//	if (!minLinear && !mipLinear)
			//		rs.samplerStates[stage].minFilter = GL_NEAREST;
			//	else if (!minLinear && mipLinear)
			//		rs.samplerStates[stage].minFilter = GL_NEAREST_MIPMAP_LINEAR;
			//	else if (minLinear && !mipLinear)
			//		rs.samplerStates[stage].minFilter = GL_LINEAR_MIPMAP_NEAREST;
			//	else
			//		rs.samplerStates[stage].minFilter = GL_LINEAR_MIPMAP_LINEAR;

			//	rs.samplerStates[stage].magFilter = c_glFilter[samplerState.magFilter];
			//	rs.samplerStates[stage].wrapS = c_glWrap[samplerState.addressU];
			//	rs.samplerStates[stage].wrapT = c_glWrap[samplerState.addressV];
			//	rs.samplerStates[stage].wrapR = c_glWrap[samplerState.addressW];
			//	rs.samplerStates[stage].compare = c_glCompare[samplerState.compare];
			//}

			//if (!cx.getShader().haveUniform(samplerId))
			//{
			//	auto& fu = cx.getShader().getOutputStream(GlslShader::BtSamplers);
			//	fu << L"uniform sampler " << samplerId << L";" << Endl;
			//	cx.getShader().addUniform(samplerId);
			//}
		}
	}

	// Define script instance.
	if (cx.getShader().defineScript(node->getName()))
	{
		auto& fs = cx.getShader().getOutputStream(GlslShader::BtScript);

		fs << L"void " << node->getName() << L"(";

		int32_t ii = 0;
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			if (ins[i]->getType() >= GtTexture2D)
				continue;

			if (ii++ > 0)
				fs << L", ";

			fs << glsl_type_name(ins[i]->getType()) << L" " << node->getInputPin(i)->getName();
		}

		if (!ins.empty())
			fs << L", ";

		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << L"out " << glsl_type_name(outs[i]->getType()) << L" " << node->getOutputPin(i)->getName();
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
		f << glsl_type_name(outs[i]->getType()) << L" " << outs[i]->getName() << L";" << Endl;

	f << node->getName() << L"(";

	int32_t ii = 0;
	for (auto in : ins)
	{
		if (in->getType() >= GtTexture2D)
			continue;

		if (ii++ > 0)
			f << L", ";

		f << in->getName();
	}
	if (ii > 0)
		f << L", ";

	ii = 0;
	for (auto out : outs)
	{
		if (ii++)
			f << L", ";
		f << out->getName();
	}

	f << L");" << Endl;
	return true;
}

bool emitSin(GlslContext& cx, Sin* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"sin(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitSqrt(GlslContext& cx, Sqrt* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitStep(GlslContext& cx, Step* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"X");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Y");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"step(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitStruct(GlslContext& cx, Struct* node)
{
	Ref< GlslVariable > out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		GtStructBuffer
	);

	auto existing = cx.getLayout().get(node->getParameterName());
	if (existing != nullptr)
	{
		if (auto existingStorageBuffer = dynamic_type_cast< const GlslStorageBuffer* >(existing))
		{
			// Storage buffer already exist; \tbd ensure elements match.
			return false;
		}
		else
		{
			// Resource already exist but is not a storage buffer.
			return false;
		}
	}
	else
	{
		// Storage buffer do not exist; add new storage buffer resource.
		Ref< GlslStorageBuffer > storageBuffer = new GlslStorageBuffer(node->getParameterName());
		for (const auto& element : node->getElements())
			storageBuffer->add(element.name, glsl_from_data_type(element.type));
		cx.getLayout().add(storageBuffer);
	}

	// Define parameter in context.
	cx.addParameter(
		node->getParameterName(),
		PtStructBuffer,
		1,
		UfDraw
	);

	return true;
}

bool emitSub(GlslContext& cx, Sub* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitSum(GlslContext& cx, Sum* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtVoid);
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
	StringOutputStream fs;
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		Ref< GlslVariable > input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		*out = GlslVariable(out->getNode(), out->getName(), input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	assign(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"for (float " << N->getName() << L" = " << node->getFrom() << L".0; " << N->getName() << L" <= " << node->getTo() << L".0; ++" << N->getName() << L")" << Endl;
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

bool emitSwizzle(GlslContext& cx, Swizzle* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	std::wstring map = toLower(node->get());
	if (map.length() == 0)
		return false;

	const GlslType types[] = { GtFloat, GtFloat2, GtFloat3, GtFloat4 };
	GlslType type = types[map.length() - 1];

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	//if (
	//	(map == L"xyzw" && in->getType() == GtFloat4) ||
	//	(map == L"xyz" && in->getType() == GtFloat3) ||
	//	(map == L"xy" && in->getType() == GtFloat2) ||
	//	(map == L"x" && in->getType() == GtFloat)
	//)
	//{
	//	// No need to swizzle; pass variable further.
	//	cx.emitOutput(node, L"Output", in);
	//}
	//else
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

		bool containConstant = false;
		for (size_t i = 0; i < map.length() && !containConstant; ++i)
		{
			if (map[i] == L'0' || map[i] == L'1')
				containConstant = true;
		}

		StringOutputStream ss;
		if (containConstant || (map.length() > 1 && in->getType() == GtFloat))
		{
			ss << glsl_type_name(type) << L"(";
			for (size_t i = 0; i < map.length(); ++i)
			{
				if (i > 0)
					ss << L", ";
				switch (map[i])
				{
				case 'x':
					if (in->getType() == GtFloat)
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
					ss << L"0.0";
					break;
				case '1':
					ss << L"1.0";
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
			if (map[0] == L'x' && in->getType() == GtFloat)
				ss << in->getName();
			else
				ss << in->getName() << L'.' << map[0];
		}

		assign(f, out) << ss.str() << L";" << Endl;
	}

	return true;
}

bool emitSwitch(GlslContext& cx, Switch* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Select");
	if (!in)
		return false;

	const std::vector< int32_t >& caseConditions = node->getCases();
	std::vector< std::wstring > caseBranches;
	std::vector< GlslVariable > caseInputs;
	GlslType outputType = GtVoid;

	// Conditional branches.
	for (uint32_t i = 0; i < uint32_t(caseConditions.size()); ++i)
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(i + 2);
		T_ASSERT(caseInput);

		Ref< GlslVariable > caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT(caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(*caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	// Default branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		const InputPin* caseInput = node->getInputPin(1);
		T_ASSERT(caseInput);

		Ref< GlslVariable > caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT(caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(*caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", outputType);
	assign(f, out) << expandScalar(0.0f, outputType) << L";" << Endl;

	const std::vector< int32_t >& cases = node->getCases();

	for (uint32_t i = 0; i < uint32_t(cases.size()); ++i)
	{
		f << (i == 0 ? L"if (" : L"else if (") << L"int(" << in->cast(GtFloat) << L") == " << cases[i] << L")" << Endl;
		f << L"{" << Endl;
		f << IncreaseIndent;

		f << caseBranches[i];
		f << out->getName() << L" = " << caseInputs[i].cast(outputType) << L";" << Endl;

		f << DecreaseIndent;
		f << L"}" << Endl;
	}

	if (!cases.empty())
	{
		f << L"else" << Endl;
		f << L"{" << Endl;
		f << IncreaseIndent;
	}

	f << caseBranches.back();
	f << out->getName() << L" = " << caseInputs.back().cast(outputType) << L";" << Endl;

	if (!cases.empty())
	{
		f << DecreaseIndent;
		f << L"}" << Endl;
	}

	return true;
}

bool emitTan(GlslContext& cx, Tan* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"tan(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitTargetSize(GlslContext& cx, TargetSize* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat2);

	auto ub = cx.getLayout().get< GlslUniformBuffer >(UfOnce);
	if (!ub->add(
		L"_vk_targetSize",
		GtFloat4,
		1
	))
		return false;

	cx.addParameter(L"_vk_targetSize", PtVector, 1, UfOnce);

	assign(f, out) << L"_vk_targetSize.xy;" << Endl;
	return true;
}

bool emitTextureSize(GlslContext& cx, TextureSize* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in || in->getType() < GtTexture2D)
		return false;

	std::wstring textureName = in->getName();

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat3);
	switch (in->getType())
	{
	case GtTexture2D:
		f << L"vec3 " << out->getName() << L" = vec3(textureSize(" << textureName << L", 0).xy, 0.0f);" << Endl;
		break;

	case GtTexture3D:
		f << L"vec3 " << out->getName() << L" = textureSize(" << textureName << L", 0);" << Endl;
		break;

	case GtTextureCube:
		f << L"vec3 " << out->getName() << L" = textureSize(" << textureName << L", 0);" << Endl;
		break;

	default:
		return false;
	}

	return true;
}

bool emitTransform(GlslContext& cx, Transform* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	Ref< GlslVariable > transform = cx.emitInput(node, L"Transform");
	if (!in || !transform)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat4);
	assign(f, out) << transform->getName() << L" * " << in->cast(GtFloat4) << L";" << Endl;
	return true;
}

bool emitTranspose(GlslContext& cx, Transpose* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"transpose(" << in->getName() << L");" << Endl;
	return true;
}

bool emitTruncate(GlslContext& cx, Truncate* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << in->getName() << L" - fract(" << in->getName() << L");" << Endl;
	return true;
}

bool emitUniform(GlslContext& cx, Uniform* node)
{
	Ref< GlslVariable > out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		glsl_from_parameter_type(node->getParameterType())
	);
	if (!out)
		return false;

	// Add uniform to layout.
	if (out->getType() < GtTexture2D)
	{
		auto ub = cx.getLayout().get< GlslUniformBuffer >(node->getFrequency());
		if (!ub->add(
			node->getParameterName(),
			out->getType(),
			1
		))
			return false;
	}
	else
	{
		auto existing = cx.getLayout().get(node->getParameterName());
		if (existing != nullptr)
		{
			if (auto existingTexture = dynamic_type_cast< const GlslTexture* >(existing))
			{
				// Texture already exist; ensure type match.
				if (existingTexture->getUniformType() != out->getType())
					return false;
			}
			else
			{
				// Resource already exist but is not a texture.
				return false;
			}
		}
		else
		{
			// Texture do not exist; add new texture resource.
			cx.getLayout().add(new GlslTexture(
				node->getParameterName(),
				out->getType()
			));
		}
	}

	// Define parameter in context.
	cx.addParameter(
		node->getParameterName(),
		node->getParameterType(),
		1,
		node->getFrequency()
	);

	return true;
}

bool emitVector(GlslContext& cx, Vector* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GtFloat4);
	assign(f, out) << L"vec4(" << formatFloat(node->get().x()) << L", " << formatFloat(node->get().y()) << L", " << formatFloat(node->get().z()) << L", " << formatFloat(node->get().w()) << L");" << Endl;
	return true;
}

bool emitVertexInput(GlslContext& cx, VertexInput* node)
{
	if (!cx.inVertex())
		return false;

	Ref< GlslVariable > out = cx.getShader().getInputVariable(node->getName());
	if (!out)
	{
		GlslType type = glsl_from_data_type(node->getDataType());
		std::wstring attributeName = VertexAttributesVk::getName(node->getDataUsage(), node->getIndex());

		auto& fi = cx.getVertexShader().getOutputStream(GlslShader::BtInput);
		fi << L"layout (location = " << VertexAttributesVk::getLocation(node->getDataUsage(), node->getIndex()) << L") in " << glsl_type_name(type) << L" " << attributeName << L";" << Endl;

		if (node->getDataUsage() == DuPosition && type != GtFloat4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				GtFloat4
			);
			auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
			switch (type)
			{
			case GtFloat:
				assign(f, out) << L"vec4(" << attributeName << L", 0.0, 0.0, 1.0);" << Endl;
				break;

			case GtFloat2:
				assign(f, out) << L"vec4(" << attributeName << L".xy, 0.0, 1.0);" << Endl;
				break;

			case GtFloat3:
				assign(f, out) << L"vec4(" << attributeName << L".xyz, 1.0);" << Endl;
				break;

			default:
				break;
			}
		}
		else if (node->getDataUsage() == DuNormal && type != GtFloat4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				GtFloat4
			);
			auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
			switch (type)
			{
			case GtFloat:
				assign(f, out) << L"vec4(" << attributeName << L", 0.0, 0.0, 0.0);" << Endl;
				break;

			case GtFloat2:
				assign(f, out) << L"vec4(" << attributeName << L".xy, 0.0, 0.0);" << Endl;
				break;

			case GtFloat3:
				assign(f, out) << L"vec4(" << attributeName << L".xyz, 0.0);" << Endl;
				break;

			default:
				break;
			}
		}
		else if (node->getDataUsage() == DuColor)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				type
			);
			auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
			switch (type)
			{
			case GtFloat:
				assign(f, out) << attributeName << L".z;" << Endl;
				break;

			case GtFloat2:
				assign(f, out) << attributeName << L".zy;" << Endl;
				break;

			case GtFloat3:
				assign(f, out) << attributeName << L".zyx;" << Endl;
				break;

			case GtFloat4:
				assign(f, out) << attributeName << L".zyxw;" << Endl;
				break;

			default:
				break;
			}
		}
		else
		{
			out = cx.getShader().createVariable(
				node->findOutputPin(L"Output"),
				attributeName,
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

bool emitVertexOutput(GlslContext& cx, VertexOutput* node)
{
	cx.enterVertex();

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	auto& fb = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
	switch (in->getType())
	{
	case GtFloat:
		fb << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * vec4(" << in->getName() << L", 0.0, 0.0, 1.0);" << Endl;
		break;

	case GtFloat2:
		fb << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * vec4(" << in->getName() << L".xy, 0.0, 1.0);" << Endl;
		break;

	case GtFloat3:
		fb << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * vec4(" << in->getName() << L".xyz, 1.0);" << Endl;
		break;

	case GtFloat4:
		fb << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * " << in->getName() << L";" << Endl;
		break;

	default:
		break;
	}

	return true;
}

		}

struct Emitter
{
	virtual ~Emitter() {}

	virtual bool emit(GlslContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public Emitter
{
	typedef bool (*function_t)(GlslContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function)
	:	m_function(function)
	{
	}

	virtual bool emit(GlslContext& c, Node* node) override final
	{
		T_ASSERT(is_a< NodeType >(node));
		return (*m_function)(c, static_cast< NodeType* >(node));
	}
};

GlslEmitter::GlslEmitter()
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

GlslEmitter::~GlslEmitter()
{
	for (auto emitter : m_emitters)
		delete emitter.second;
}

bool GlslEmitter::emit(GlslContext& c, Node* node)
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
