/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <iomanip>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Render/VertexElement.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Glsl/GlslContext.h"
#include "Render/Editor/Glsl/GlslEmitter.h"
#include "Render/Editor/Glsl/GlslImage.h"
#include "Render/Editor/Glsl/GlslSampler.h"
#include "Render/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Editor/Glsl/GlslTexture.h"
#include "Render/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor::render
{
	namespace
	{

const wchar_t* c_uniformBufferNames[] = { L"UbOnce", L"UbFrame", L"UbDraw" };

uint8_t getBindStage(const GlslContext& cx)
{
	if (cx.inVertex())
		return GlslResource::BsVertex;
	else if (cx.inFragment())
		return GlslResource::BsFragment;
	else if (cx.inCompute())
		return GlslResource::BsCompute;
	else
		return GlslResource::BsVertex | GlslResource::BsFragment | GlslResource::BsCompute;
}

std::wstring formatFloat(float v)
{
	std::wstring s = toString(v);
	if (s.find(L'.') == s.npos)
		s += L".0";
	return s;
}

std::wstring expandScalar(float v, GlslType type)
{
	if (type >= GlslType::Float && type <= GlslType::Float4)
	{
		const std::wstring vs = formatFloat(v);
		switch (type)
		{
		case GlslType::Float:
			return vs;

		case GlslType::Float2:
			return L"vec2(" + vs + L", " + vs + L")";

		case GlslType::Float3:
			return L"vec3(" + vs + L", " + vs + L", " + vs + L")";

		case GlslType::Float4:
			return L"vec4(" + vs + L", " + vs + L", " + vs + L", " + vs + L")";

		default:
			break;
		}
	}

	if (type >= GlslType::Integer && type <= GlslType::Integer4)
	{
		const std::wstring vs = toString((int32_t)v);
		switch (type)
		{
		case GlslType::Integer:
			return vs;

		case GlslType::Integer2:
			return L"ivec2(" + vs + L", " + vs + L")";

		case GlslType::Integer3:
			return L"ivec3(" + vs + L", " + vs + L", " + vs + L")";

		case GlslType::Integer4:
			return L"ivec4(" + vs + L", " + vs + L", " + vs + L", " + vs + L")";

		default:
			break;
		}
	}

	return L"";
}

OutputStream& assign(OutputStream& f, const GlslVariable* out)
{
	f << L"const " << glsl_type_name(out->getType()) << L" " << out->getName() << L" = ";
	return f;
}

OutputStream& assignMutable(OutputStream& f, const GlslVariable* out)
{
	f << glsl_type_name(out->getType()) << L" " << out->getName() << L" = ";
	return f;
}

OutputStream& comment(OutputStream& f, const Node* node)
{
	const std::wstring& comment = node->getComment();
	if (!comment.empty())
		f << L"// " << comment << Endl;
	return f;
}

bool emitAbs(GlslContext& cx, Abs* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"abs(" << in->getName() << L");" << Endl;

	return true;
}

bool emitAdd(GlslContext& cx, Add* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
	{
		cx.pushError(L"Inputs not connected.");
		return false;
	}

	const GlslType type = glsl_precedence(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;

	return true;
}

bool emitArcusCos(GlslContext& cx, ArcusCos* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GlslType::Float)
	{
		cx.pushError(L"Theta not connected or incorrect type.");
		return false;
	}
	
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);
	
	comment(f, node);
	assign(f, out) << L"acos(" << theta->getName() << L");" << Endl;

	return true;
}

bool emitArcusTan(GlslContext& cx, ArcusTan* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > xy = cx.emitInput(node, L"XY");
	if (!xy || glsl_type_width(xy->getType()) < 2)
	{
		cx.pushError(L"XY not connected or incorrect width.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
	assign(f, out) << L"atan(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;

	return true;
}

bool emitClamp(GlslContext& cx, Clamp* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"clamp(" << in->getName() << L", " << formatFloat(node->getMin()) << L", " << formatFloat(node->getMax()) << L");" << Endl;

	return true;
}

bool emitColor(GlslContext& cx, Color* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float4);

	Color4f value = node->getColor();
	if (!node->getLinear())
		value = value.linear();
	
	comment(f, node);
	assign(f, out) << L"vec4(" << formatFloat(value.getRed()) << L", " << formatFloat(value.getGreen()) << L", " << formatFloat(value.getBlue()) << L", " << formatFloat(value.getAlpha()) << L");" << Endl;

	return true;
}

bool emitComputeOutput(GlslContext& cx, ComputeOutput* node)
{
	cx.enterCompute();

	const GlslVariable* offset = cx.emitInput(node, L"Offset");
	if (!offset)
	{
		cx.pushError(L"Offset not connected.");
		return false;
	}

	const GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	const Node* storage = cx.getInputNode(node, L"Storage");
	if (!storage)
	{
		cx.pushError(L"Storage not connected.");
		return false;
	}

	if (const Uniform* storageUniformNode = dynamic_type_cast< const Uniform* >(storage))
	{
		if (!(storageUniformNode->getParameterType() >= ParameterType::Image2D && storageUniformNode->getParameterType() <= ParameterType::ImageCube))
		{
			cx.pushError(L"Incorrect parameter type.");
			return false;
		}

		// Check if image needs to be defined.
		const auto existing = cx.getLayout().getByName(storageUniformNode->getParameterName());
		if (existing != nullptr)
		{
			auto existingImage = dynamic_type_cast< GlslImage* >(existing);
			if (!existingImage)
			{
				cx.pushError(L"Image do not exist.");
				return false;
			}

			existingImage->addStage(GlslResource::BsCompute);
		}
		else
		{
			// Image do not exist; add new image resource.
			cx.getLayout().addBindless(
				new GlslImage(
					storageUniformNode->getParameterName(),
					GlslResource::Set::Default,
					GlslResource::BsCompute,
					glsl_from_parameter_type(storageUniformNode->getParameterType()),
					false
				)
			);
		}

		auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
		if (storageUniformNode->getParameterType() == ParameterType::Image2D)
			f << L"imageStore(__bindlessImages2D__[" << storageUniformNode->getParameterName() << L"], " << offset->cast(GlslType::Integer2) << L", " << in->cast(GlslType::Float4) << L");" << Endl;
		else if (storageUniformNode->getParameterType() == ParameterType::Image3D)
			f << L"imageStore(__bindlessImages3D__[" << storageUniformNode->getParameterName() << L"], " << offset->cast(GlslType::Integer3) << L", " << in->cast(GlslType::Float4) << L");" << Endl;
		else
			f << L"imageStore(__bindlessImagesCube__[" << storageUniformNode->getParameterName() << L"], " << offset->cast(GlslType::Integer3) << L", " << in->cast(GlslType::Float4) << L");" << Endl;

		// Image parameter; since resource index is passed to shader we define an integer uniform.
		auto ub = cx.getLayout().getByName< GlslUniformBuffer >(L"UbDraw"); // c_uniformBufferNames[(int32_t)node->getFrequency()]);
		ub->addStage(getBindStage(cx));
		if (!ub->add(storageUniformNode->getParameterName(), GlslType::Integer, 1))
		{
			cx.pushError(L"Failed to register uniform.");
			return false;
		}

		// Define parameter in context.
		cx.addParameter(
			storageUniformNode->getParameterName(),
			storageUniformNode->getParameterType(),
			1,
			UpdateFrequency::Draw
		);
	}
	else if (const Struct* storageStructNode = dynamic_type_cast< const Struct* >(storage))
	{
		// Check if storage buffer needs to be defined.
		auto existing = cx.getLayout().getByName(storageStructNode->getParameterName());
		if (existing != nullptr)
		{
			auto existingBuffer = dynamic_type_cast< GlslStorageBuffer* >(existing);
			if (!existingBuffer)
			{
				cx.pushError(L"Buffer do not exist.");
				return false;
			}

			existingBuffer->addStage(getBindStage(cx));
		}
		else
		{
			// Storage buffer do not exist; add new storage buffer resource.
			Ref< GlslStorageBuffer > storageBuffer = new GlslStorageBuffer(
				storageStructNode->getParameterName(),
				GlslResource::Set::Default,
				getBindStage(cx)
			);
			for (const auto& element : storageStructNode->getElements())
				storageBuffer->add(element.name, element.type);
			cx.getLayout().add(storageBuffer);

			auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
			f << storageStructNode->getParameterName() << L"_Data[" << offset->cast(GlslType::Integer) << L"]." << /*node->getName()*/L"fieldName" << L" = " << in->cast(GlslType::Float4) << L";" << Endl;

			// Define parameter in context.
			cx.addParameter(
				storageStructNode->getParameterName(),
				ParameterType::StructBuffer,
				1,
				UpdateFrequency::Draw
			);
		}
	}
	else
	{
		cx.pushError(L"Unsupported storage type.");
		return false;
	}

	cx.requirements().localSize[0] = node->getLocalSize()[0];
	cx.requirements().localSize[1] = node->getLocalSize()[1];
	cx.requirements().localSize[2] = node->getLocalSize()[2];

	return true;
}

bool emitConditional(GlslContext& cx, Conditional* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Emit input and reference branches.
	const GlslVariable* in = cx.emitInput(node, L"Input");
	const GlslVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
	{
		cx.pushError(L"Input or Reference not connected.");
		return false;
	}

	GlslVariable caseTrue, caseFalse;
	std::wstring caseTrueBranch, caseFalseBranch;

	// Find common output pins from both sides of branch;
	// emit those before condition in order to have them evaluated outside of conditional.
	AlignedVector< const OutputPin* > outputPins;
	cx.findCommonOutputs(node, L"CaseTrue", L"CaseFalse", outputPins);
	for (auto outputPin : outputPins)
		cx.emit(outputPin->getNode());

	GlslVariable* out = cx.emitOutput(node, L"Output", GlslType::Void);

	// Emit true branch.
	{
		auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
		cx.getShader().pushScope();

		const GlslVariable* ct = cx.emitInput(node, L"CaseTrue");
		if (!ct)
		{
			cx.pushError(L"CaseTrue not connected.");
			return false;
		}

		caseTrue = *ct;
		caseTrueBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	// Emit false branch.
	{
		auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
		cx.getShader().pushScope();

		const GlslVariable* cf = cx.emitInput(node, L"CaseFalse");
		if (!cf)
		{
			cx.pushError(L"CaseFalse not connected.");
			return false;
		}

		caseFalse = *cf;
		caseFalseBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	// Create output variable.
	GlslType outputType = glsl_precedence(caseTrue.getType(), caseFalse.getType());
	out->setType(outputType);

	comment(f, node);
	f << glsl_type_name(out->getType()) << L" " << out->getName() << L";" << Endl;

	const bool supportControlFlowAttributes = (cx.getSettings() != nullptr ? cx.getSettings()->getProperty< bool >(L"Glsl.Vulkan.ControlFlowAttributes", true) : true);
	if (supportControlFlowAttributes)
	{
		if (node->getBranch() == Conditional::BrStatic)
			f << L"[[flatten]]" << Endl;
		else if (node->getBranch() == Conditional::BrDynamic)
			f << L"[[branch]]" << Endl;
	}

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

	const GlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GlslType::Float)
	{
		cx.pushError(L"Theta not connected or incorrect type.");
		return false;
	}

	const GlslVariable* out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
	assign(f, out) << L"cos(" << theta->getName() << L");" << Endl;

	return true;
}

bool emitCross(GlslContext& cx, Cross* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
	{
		cx.pushError(L"Input1 or Input2 not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float3);

	comment(f, node);
	assign(f, out) << L"cross(" << in1->cast(GlslType::Float3) << L", " << in2->cast(GlslType::Float3) << L");" << Endl;

	return true;
}

bool emitDerivative(GlslContext& cx, Derivative* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > input = cx.emitInput(node, L"Input");
	if (!input)
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", input->getType());

	comment(f, node);
	switch (node->getAxis())
	{
	case Derivative::Axis::X:
		assign(f, out) << L"dFdx(" << input->getName() << L");" << Endl;
		break;

	case Derivative::Axis::Y:
		assign(f, out) << L"dFdy(" << input->getName() << L");" << Endl;
		break;

	default:
		return false;
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
	{
		cx.pushError(L"Input or Reference not connected.");
		return false;
	}

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

	Ref< GlslVariable > pass = cx.emitInput(node, L"Pass");
	if (!pass)
	{
		cx.pushError(L"Pass not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", pass->getType());
	assign(f, out) << pass->getName() << L";" << Endl;

	return true;
}

bool emitDispatchIndex(GlslContext& cx, DispatchIndex* node)
{
	if (!cx.inCompute())
	{
		cx.pushError(L"DispatchIndex must be emitted in compute scope.");
		return false;
	}

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Integer3);

	comment(f, node);

	switch (node->getScope())
	{
	case DispatchIndex::Scope::Global:
		assign(f, out) << L"ivec3(gl_GlobalInvocationID.xyz);" << Endl;
		break;
	case DispatchIndex::Scope::Local:
		assign(f, out) << L"ivec3(gl_LocalInvocationID.xyz);" << Endl;
		break;
	case DispatchIndex::Scope::Group:
		assign(f, out) << L"ivec3(gl_WorkGroupID.xyz);" << Endl;
		break;
	}

	return true;
}

bool emitDiv(GlslContext& cx, Div* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
	{
		cx.pushError(L"Input1 or Input2 not connected.");
		return false;
	}

	GlslType type = glsl_promote_to_float(glsl_precedence(in1->getType(), in2->getType()));
	
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;

	return true;
}

bool emitDot(GlslContext& cx, Dot* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
	{
		cx.pushError(L"Input1 or Input2 not connected.");
		return false;
	}

	GlslType type = glsl_precedence(in1->getType(), in2->getType());
	if (type >= GlslType::Float && type <= GlslType::Float4)
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);
		comment(f, node);
		assign(f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	}
	else if (type >= GlslType::Integer && type <= GlslType::Integer4)
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Integer);
		comment(f, node);
		f << L"int " << out->getName() << L";" << Endl;
		f << L"{" << Endl;
		f << L"\t" << glsl_type_name(type) << L" tmp = " << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;
		switch (type)
		{
		case GlslType::Integer:
			f << L"\t" << out->getName() << L" = tmp;" << Endl;
			break;

		case GlslType::Integer2:
			f << L"\t" << out->getName() << L" = tmp.x + tmp.y;" << Endl;
			break;

		case GlslType::Integer3:
			f << L"\t" << out->getName() << L" = tmp.x + tmp.y + tmp.z;" << Endl;
			break;

		case GlslType::Integer4:
			f << L"\t" << out->getName() << L" = tmp.x + tmp.y + tmp.z + tmp.w;" << Endl;
			break;

		default:
			break;
		}
		f << L"}" << Endl;
	}
	else
	{
		cx.pushError(L"Incorrect type.");
		return false;
	}

	return true;
}

bool emitExp(GlslContext& cx, Exp* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"exp(" << in->getName() << L");" << Endl;

	return true;
}

bool emitFraction(GlslContext& cx, Fraction* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"fract(" << in->getName() << L");" << Endl;

	return true;
}

bool emitFragmentPosition(GlslContext& cx, FragmentPosition* node)
{
	if (!cx.inFragment())
	{
		cx.pushError(L"FragmentPosition must be emitted in pixel scope.");
		return false;
	}

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float2);

	comment(f, node);
	assign(f, out) << L"gl_FragCoord.xy;" << Endl;

	return true;
}

bool emitFrontFace(GlslContext& cx, FrontFace* node)
{
	if (!cx.inFragment())
	{
		cx.pushError(L"FrontFace must be emitted in pixel scope.");
		return false;
	}

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
	assign(f, out) << L"gl_FrontFacing ? 1.0 : 0.0;" << Endl;

	return true;
}

bool emitIndexedUniform(GlslContext& cx, IndexedUniform* node)
{
	const Node* indexNode = cx.getInputNode(node, L"Index");
	if (!indexNode)
	{
		cx.pushError(L"Index not connected.");
		return false;
	}

	Ref< GlslVariable > out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		glsl_from_parameter_type(node->getParameterType())
	);

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	if (const Scalar* scalarIndexNode = dynamic_type_cast< const Scalar* >(indexNode))
	{
		comment(f, node);
		assign(f, out) << node->getParameterName() << L"[" << int32_t(scalarIndexNode->get()) << L"];" << Endl;
	}
	else
	{
		Ref< GlslVariable > index = cx.emitInput(node, L"Index");
		if (!index)
		{
			cx.pushError(L"Index not connected.");
			return false;
		}

		comment(f, node);
		assign(f, out) << node->getParameterName() << L"[" << index->cast(GlslType::Integer) << L"];" << Endl;
	}

	// Add uniform to layout.
	if (out->getType() < GlslType::Texture2D)
	{
		auto ub = cx.getLayout().getByName< GlslUniformBuffer >(c_uniformBufferNames[(int32_t)node->getFrequency()]);
		ub->addStage(getBindStage(cx));
		ub->add(node->getParameterName(), out->getType(), node->getLength());
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
	if (cx.inVertex())
	{
		auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Integer);

		comment(f, node);
		assign(f, out) << L"gl_InstanceIndex;" << Endl;
		return true;
	}
	else if (cx.inFragment())
	{
		int32_t interpolatorId;
		int32_t interpolatorOffset;
		bool declare = cx.allocateInterpolator(1, interpolatorId, interpolatorOffset);

		std::wstring interpolatorName = L"Attr" + toString(interpolatorId);
		std::wstring interpolatorMask = interpolatorName + L"." + std::wstring(L"xyzw").substr(interpolatorOffset, 1);

		auto& fv = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
		comment(fv, node) << interpolatorMask << L" = float(gl_InstanceIndex);" << Endl;

		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

		auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
		comment(f, node);

		const bool supportBallot = (cx.getSettings() != nullptr ? cx.getSettings()->getProperty< bool >(L"Glsl.Vulkan.Ballot", true) : true);
		if (supportBallot)
			assign(f, out) << L"readFirstInvocationARB(" << interpolatorMask << L");" << Endl;
		else
			assign(f, out) << interpolatorMask << L";" << Endl;

		if (declare)
		{
			auto& fvo = cx.getVertexShader().getOutputStream(GlslShader::BtOutput);
			auto& fpi = cx.getFragmentShader().getOutputStream(GlslShader::BtInput);

			fvo << L"layout (location = " << interpolatorId << L") out vec4 " << interpolatorName << L";" << Endl;
			fpi << L"layout (location = " << interpolatorId << L") in vec4 " << interpolatorName << L";" << Endl;
		}

		return true;
	}
	else
	{
		cx.pushError(L"Instance must be emitted in either vertex or pixel scope.");
		return false;
	}
}

bool emitInterpolator(GlslContext& cx, Interpolator* node)
{
	if (!cx.inFragment())
	{
		// We're already in vertex state; skip interpolation.
		Ref< GlslVariable > in = cx.emitInput(node, L"Input");
		if (!in)
		{
			cx.pushError(L"Input not connected.");
			return false;
		}

		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

		auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

		comment(f, node);
		assign(f, out) << in->getName() << L";" << Endl;

		return true;
	}

	cx.enterVertex();

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	cx.enterFragment();

	const int32_t interpolatorWidth = glsl_type_width(in->getType());
	if (!interpolatorWidth)
	{
		cx.pushError(L"Incorrect type.");
		return false;
	}

	int32_t interpolatorId;
	int32_t interpolatorOffset;
	const bool declare = cx.allocateInterpolator(interpolatorWidth, interpolatorId, interpolatorOffset);

	const std::wstring interpolatorName = L"Attr" + toString(interpolatorId);
	const std::wstring interpolatorMask = interpolatorName + L"." + std::wstring(L"xyzw").substr(interpolatorOffset, interpolatorWidth);

	auto& f = cx.getVertexShader().getOutputStream(GlslShader::BtBody);

	comment(f, node);
	f << interpolatorMask << L" = " << in->getName() << L";" << Endl;

	cx.getFragmentShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		interpolatorMask,
		glsl_promote_to_float(in->getType())
	);

	if (declare)
	{
		auto& fvo = cx.getVertexShader().getOutputStream(GlslShader::BtOutput);
		auto& fpi = cx.getFragmentShader().getOutputStream(GlslShader::BtInput);

		fvo << L"layout (location = " << interpolatorId << L") out highp vec4 " << interpolatorName << L";" << Endl;
		fpi << L"layout (location = " << interpolatorId << L") in highp vec4 " << interpolatorName << L";" << Endl;
	}

	return true;
}

bool emitIterate(GlslContext& cx, Iterate* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GlslType::Integer);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Void);
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
	auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	{
		Ref< GlslVariable > input = cx.emitInput(node, L"Input");
		if (!input)
		{
			cx.pushError(L"Input not connected.");
			return false;
		}

		// Emit post condition if connected; break iteration if condition is false.
		Ref< GlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(GlslType::Integer) << L" == 0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< GlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assignMutable(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assignMutable(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"int " << N->getName() << L";" << Endl;
	f << L"for (" << N->getName() << L" = " << node->getFrom() << L"; " << N->getName() << L" <= " << node->getTo() << L"; ++" << N->getName() << L")" << Endl;
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

bool emitIterate2(GlslContext& cx, Iterate2* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputNames[4];

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GlslType::Integer);
	T_ASSERT(N);

	// Create void output variables; change type later when we know
	// the type of the input branches.
	Ref< GlslVariable > out[] =
	{
		cx.getInputNode(node, L"Input0") != nullptr ? cx.emitOutput(node, L"Output0", GlslType::Void) : nullptr,
		cx.getInputNode(node, L"Input1") != nullptr ? cx.emitOutput(node, L"Output1", GlslType::Void) : nullptr,
		cx.getInputNode(node, L"Input2") != nullptr ? cx.emitOutput(node, L"Output2", GlslType::Void) : nullptr,
		cx.getInputNode(node, L"Input3") != nullptr ? cx.emitOutput(node, L"Output3", GlslType::Void) : nullptr,
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
	auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
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
			fs << L"if (" << condition->cast(GlslType::Integer) << L" == 0)" << Endl;
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

	std::wstring inner = fs.str();
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
			assignMutable(f, out[i]) << initial[i]->cast(out[i]->getType()) << L";" << Endl;
		else if (out[i])
			assignMutable(f, out[i]) << expandScalar(0.0f, out[i]->getType()) << L";" << Endl;
	}

	// Write outer for-loop statement.
	StringOutputStream ss;
	ss << L"int " << N->getName() << L";" << Endl;
	ss << L"for (" << N->getName() << L" = ";
	if (Scalar* scalarFrom = dynamic_type_cast< Scalar* >(cx.getInputNode(node, L"From")))
		ss << (int32_t)scalarFrom->get();
	else
	{
		Ref< GlslVariable > from = cx.emitInput(node, L"From");
		if (from)
			ss << from->cast(GlslType::Integer);
		else
			return false;
	}
	ss << L"; " << N->getName() << L" <= ";
	if (Scalar* scalarTo = dynamic_type_cast< Scalar* >(cx.getInputNode(node, L"To")))
		ss << (int32_t)scalarTo->get();
	else
	{
		Ref< GlslVariable > to = cx.emitInput(node, L"To");
		if (to)
			ss << to->cast(GlslType::Integer);
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

bool emitIterate2d(GlslContext& cx, Iterate2d* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variables.
	Ref< GlslVariable > X = cx.emitOutput(node, L"X", GlslType::Integer);
	T_ASSERT(X);

	Ref< GlslVariable > Y = cx.emitOutput(node, L"Y", GlslType::Integer);
	T_ASSERT(Y);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Void);
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
	auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	Ref< GlslVariable > input = cx.emitInput(node, L"Input");
	if (!input)
		return false;

	// Emit post condition if connected; break iteration if condition is false.
	Ref< GlslVariable > condition = cx.emitInput(node, L"Condition");
	if (condition)
	{
		fs << L"if (" << condition->cast(GlslType::Integer) << L" == 0)" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	inputName = input->getName();

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	*out = GlslVariable(out->getNode(), out->getName(), input->getType());

	cx.getShader().popScope();

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< GlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assignMutable(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assignMutable(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"for (int " << X->getName() << L" = " << node->getFromX() << L"; " << X->getName() << L" <= " << node->getToX() << L"; ++" << X->getName() << L")" << Endl;
	f << L"{" << Endl;
	f << IncreaseIndent;

	f << L"for (int " << Y->getName() << L" = " << node->getFromY() << L"; " << Y->getName() << L" <= " << node->getToY() << L"; ++" << Y->getName() << L")" << Endl;
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
		f << L"if (" << condition->cast(GlslType::Integer) << L" == 0)" << Endl;
		f << L"\tbreak;" << Endl;
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

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
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

	GlslType type = glsl_promote_to_float(glsl_precedence(in1->getType(), in2->getType()));
	if (type == GlslType::Void)
		return false;

	Ref< GlslVariable > blend = cx.emitInput(node, L"Blend");
	if (!blend || blend->getType() != GlslType::Float)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"mix(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;

	return true;
}

bool emitLog(GlslContext& cx, Log* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
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

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float4x4);

	comment(f, node);
	f << L"mat4 " << out->getName() << L" = mat4(" << Endl;
	f << IncreaseIndent;
	f << (xaxis     ? xaxis->cast(GlslType::Float4)     : L"vec4(1.0, 0.0, 0.0, 0.0)") << L"," << Endl;
	f << (yaxis     ? yaxis->cast(GlslType::Float4)     : L"vec4(0.0, 1.0, 0.0, 0.0)") << L"," << Endl;
	f << (zaxis     ? zaxis->cast(GlslType::Float4)     : L"vec4(0.0, 0.0, 1.0, 0.0)") << L"," << Endl;
	f << (translate ? translate->cast(GlslType::Float4) : L"vec4(0.0, 0.0, 0.0, 1.0)") << Endl;
	f << DecreaseIndent;
	f << L");" << Endl;

	return true;
}

bool emitMatrixOut(GlslContext& cx, MatrixOut* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	comment(f, node);

	Ref< GlslVariable > xaxis = cx.emitOutput(node, L"XAxis", GlslType::Float4);
	if (xaxis)
		assign(f, xaxis) << in->getName() << L"[0];" << Endl;

	Ref< GlslVariable > yaxis = cx.emitOutput(node, L"YAxis", GlslType::Float4);
	if (yaxis)
		assign(f, yaxis) << in->getName() << L"[1];" << Endl;

	Ref< GlslVariable > zaxis = cx.emitOutput(node, L"ZAxis", GlslType::Float4);
	if (zaxis)
		assign(f, zaxis) << in->getName() << L"[2];" << Endl;

	Ref< GlslVariable > translate = cx.emitOutput(node, L"Translate", GlslType::Float4);
	if (translate)
		assign(f, translate) << in->getName() << L"[3];" << Endl;

	return true;
}

bool emitMax(GlslContext& cx, Max* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in1 = cx.emitInput(node, L"Input1");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;

	GlslType type = glsl_precedence(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
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

	GlslType type = glsl_precedence(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;

	return true;
}

bool emitMixIn(GlslContext& cx, MixIn* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in[4] =
	{
		cx.emitInput(node, L"X"),
		cx.emitInput(node, L"Y"),
		cx.emitInput(node, L"Z"),
		cx.emitInput(node, L"W")
	};

	comment(f, node);

	bool integer = true;
	int32_t width = 0;

	for (int32_t i = 0; i < 4; ++i)
	{
		if (!in[i])
			continue;
		if (in[i]->getType() == GlslType::Float)
			integer = false;
		else if (in[i]->getType() != GlslType::Integer)
			return false;
		width = i + 1;
	}

	switch (width)
	{
	case 0:
		return false;

	case 1:
		{
			Ref< GlslVariable > out = cx.emitOutput(node, L"Output", integer ? GlslType::Integer : GlslType::Float);
			assign(f, out) << in[0]->getName() << L";" << Endl;
		}
		break;

	case 2:
		{
			Ref< GlslVariable > out = cx.emitOutput(node, L"Output", integer ? GlslType::Integer2 : GlslType::Float2);
			assign(f, out);
			f << (integer ? L"ivec2" : L"vec2") << L"(";
			f << ((in[0] != nullptr) ? in[0]->getName() : (integer ? L"0" : L"0.0")) << L", ";
			f << ((in[1] != nullptr) ? in[1]->getName() : (integer ? L"0" : L"0.0"));
			f << L");" << Endl;
		}
		break;

	case 3:
		{
			Ref< GlslVariable > out = cx.emitOutput(node, L"Output", integer ? GlslType::Integer3 : GlslType::Float3);
			assign(f, out);
			f << (integer ? L"ivec3" : L"vec3") << L"(";
			f << ((in[0] != nullptr) ? in[0]->getName() : (integer ? L"0" : L"0.0")) << L", ";
			f << ((in[1] != nullptr) ? in[1]->getName() : (integer ? L"0" : L"0.0")) << L", ";
			f << ((in[2] != nullptr) ? in[2]->getName() : (integer ? L"0" : L"0.0"));
			f << L");" << Endl;
		}
		break;

	case 4:
		{
			Ref< GlslVariable > out = cx.emitOutput(node, L"Output", integer ? GlslType::Integer4 : GlslType::Float4);
			assign(f, out);
			f << (integer ? L"ivec4" : L"vec4") << L"(";
			f << ((in[0] != nullptr) ? in[0]->getName() : (integer ? L"0" : L"0.0")) << L", ";
			f << ((in[1] != nullptr) ? in[1]->getName() : (integer ? L"0" : L"0.0")) << L", ";
			f << ((in[2] != nullptr) ? in[2]->getName() : (integer ? L"0" : L"0.0")) << L", ";
			f << ((in[3] != nullptr) ? in[3]->getName() : (integer ? L"0" : L"0.0"));
			f << L");" << Endl;
		}
		break;
	}

	return true;
}

bool emitMixOut(GlslContext& cx, MixOut* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	comment(f, node);
	switch (in->getType())
	{
	case GlslType::Integer:
	case GlslType::Float:
		{
			GlslType outputType = (in->getType() == GlslType::Integer) ? GlslType::Integer : GlslType::Float;
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", outputType);
			assign(f, x) << in->getName() << L";" << Endl;
		}
		break;

	case GlslType::Integer2:
	case GlslType::Float2:
		{
			GlslType outputType = (in->getType() == GlslType::Integer2) ? GlslType::Integer : GlslType::Float;
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", outputType);
			Ref< GlslVariable > y = cx.emitOutput(node, L"Y", outputType);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
		}
		break;

	case GlslType::Integer3:
	case GlslType::Float3:
		{
			GlslType outputType = (in->getType() == GlslType::Integer3) ? GlslType::Integer : GlslType::Float;
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", outputType);
			Ref< GlslVariable > y = cx.emitOutput(node, L"Y", outputType);
			Ref< GlslVariable > z = cx.emitOutput(node, L"Z", outputType);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
			assign(f, z) << in->getName() << L".z;" << Endl;
		}
		break;

	case GlslType::Integer4:
	case GlslType::Float4:
		{
			GlslType outputType = (in->getType() == GlslType::Integer4) ? GlslType::Integer : GlslType::Float;
			Ref< GlslVariable > x = cx.emitOutput(node, L"X", outputType);
			Ref< GlslVariable > y = cx.emitOutput(node, L"Y", outputType);
			Ref< GlslVariable > z = cx.emitOutput(node, L"Z", outputType);
			Ref< GlslVariable > w = cx.emitOutput(node, L"W", outputType);
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

	GlslType type = glsl_precedence(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
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

	GlslType type = glsl_precedence(glsl_precedence(in1->getType(), in2->getType()), in3->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"fma(" << in1->cast(type) << L", " << in2->cast(type) << L", " << in3->cast(type) << L");" << Endl;

	return true;
}

bool emitNeg(GlslContext& cx, Neg* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
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

	comment(f, node);
	assign(f, out) << L"normalize(" << in->getName() << L");" << Endl;

	return true;
}

bool emitPixelOutput(GlslContext& cx, PixelOutput* node)
{
	RenderState rs = node->getRenderState();

	const static RenderState defaultState;
	if (!rs.blendEnable)
	{
		rs.blendColorOperation = defaultState.blendColorOperation;
		rs.blendColorSource = defaultState.blendColorSource;
		rs.blendColorDestination = defaultState.blendColorDestination;
		rs.blendAlphaOperation = defaultState.blendAlphaOperation;
		rs.blendAlphaSource = defaultState.blendAlphaSource;
		rs.blendAlphaDestination = defaultState.blendAlphaDestination;
	}
	if (!rs.depthEnable)
		rs.depthFunction = defaultState.depthFunction;
	if (!rs.alphaTestEnable)
	{
		rs.alphaTestEnable = defaultState.alphaTestEnable;
		rs.alphaTestReference = defaultState.alphaTestReference;
	}
	if (!rs.stencilEnable)
	{
		rs.stencilFail = defaultState.stencilFail;
		rs.stencilZFail = defaultState.stencilZFail;
		rs.stencilPass = defaultState.stencilPass;
		rs.stencilFunction = defaultState.stencilFunction;
		rs.stencilReference = defaultState.stencilReference;
		rs.stencilMask = defaultState.stencilMask;
	}

	cx.enterFragment();

	const wchar_t* inputs[] = { L"Input", L"Input1", L"Input2", L"Input3" };
	Ref< GlslVariable > in[4];

	for (int32_t i = 0; i < sizeof_array(in); ++i)
		in[i] = cx.emitInput(node, inputs[i]);

	if (!in[0])
		return false;

	if (rs.colorWriteMask != 0)
	{
		auto& fpb = cx.getFragmentShader().getOutputStream(GlslShader::BtBody);
		auto& fpo = cx.getFragmentShader().getOutputStream(GlslShader::BtOutput);

		comment(fpb, node);
		for (int32_t i = 0; i < sizeof_array(in); ++i)
		{
			if (!in[i])
				continue;

			fpb << L"_gl_FragData_" << i << L" = " << in[i]->cast(GlslType::Float4) << L";" << Endl;
			fpo << L"layout (location = " << i << L") out vec4 _gl_FragData_" << i << L";" << Endl;
		}
	}

	cx.setRenderState(rs);
	cx.requirements().fragmentPrecisionHint = node->getPrecisionHint();
	return true;
}

bool emitPolynomial(GlslContext& cx, Polynomial* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > x = cx.emitInput(node, L"X");
	Ref< GlslVariable > coeffs = cx.emitInput(node, L"Coefficients");
	if (!x || !coeffs)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
	assign(f, out);
	switch (coeffs->getType())
	{
	case GlslType::Float:
		f << coeffs->getName() << L".x * " << x->getName();
		break;

	case GlslType::Float2:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".y * " << x->getName();
		break;

	case GlslType::Float3:
		f << coeffs->getName() << L".x * pow(" << x->getName() << L", 3) + " << coeffs->getName() << L".y * pow(" << x->getName() << L", 2) + " << coeffs->getName() << L".z * " << x->getName();
		break;

	case GlslType::Float4:
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

	GlslType type = glsl_precedence(exponent->getType(), in->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"pow(max(" << in->cast(type) << L", 0.0), " << exponent->cast(type) << L");" << Endl;

	return true;
}

bool emitReadStruct(GlslContext& cx, ReadStruct* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > strct = cx.emitInput(node, L"Struct");
	if (!strct || strct->getType() != GlslType::StructBuffer)
		return false;

	const Struct* sn = mandatory_non_null_type_cast< const Struct* >(strct->getNode());
	DataType type = sn->getElementType(node->getName());

	Ref< GlslVariable > index = cx.emitInput(node, L"Index");
	if (!index)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", glsl_from_data_type(type));

	comment(f, node);
	f << L"const " << glsl_type_name(out->getType()) << L" " << out->getName() << L" = " << glsl_type_name(out->getType()) << L"(" << strct->getName() << L"[" << index->cast(GlslType::Integer) << L"]." << node->getName() << L");" << Endl;

	return true;
}

bool emitReadStruct2(GlslContext& cx, ReadStruct2* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	if (node->getOutputPinCount() <= 0)
		return false;

	Ref< GlslVariable > strct = cx.emitInput(node, L"Struct");
	if (!strct || strct->getType() != GlslType::StructBuffer)
		return false;

	Ref< GlslVariable > index = cx.emitInput(node, L"Index");
	if (!index)
		return false;

	const Struct* sn = mandatory_non_null_type_cast< const Struct* >(strct->getNode());
	for (int32_t i = 0; i < node->getOutputPinCount(); ++i)
	{
		const OutputPin* outputPin = node->getOutputPin(i);
		if (!cx.isConnected(outputPin))
			continue;

		const DataType type = sn->getElementType(outputPin->getName());
		Ref< GlslVariable > out = cx.emitOutput(node, outputPin->getName(), glsl_from_data_type(type));
		comment(f, node);
		f << L"const " << glsl_type_name(out->getType()) << L" " << out->getName() << L" = " << glsl_type_name(out->getType()) << L"(" << strct->getName() << L"[" << index->cast(GlslType::Integer) << L"]." << outputPin->getName() << L");" << Endl;
	}

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

	comment(f, node);
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

	comment(f, node);
	assign(f, out) << L"inversesqrt(" << in->getName() << L");" << Endl;

	return true;
}

bool emitRepeat(GlslContext& cx, Repeat* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GlslType::Integer);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Void);
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
	auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
	cx.getShader().pushScope();

	{
		// Emit pre-condition, break iteration if condition is false.
		Ref< GlslVariable > condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(GlslType::Integer) << L" == 0)" << Endl;
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

	const std::wstring inner = fs.str();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	Ref< GlslVariable > initial = cx.emitInput(node, L"Initial");
	if (initial)
		assignMutable(f, out) << initial->cast(out->getType()) << L";" << Endl;
	else
		assignMutable(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"for (int " << N->getName() << L" = 0;; ++" << N->getName() << L")" << Endl;
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

bool emitRound(GlslContext& cx, Round* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"round(" << in->getName() << L");" << Endl;

	return true;
}

bool emitSampler(GlslContext& cx, Sampler* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > texture = cx.emitInput(node, L"Texture");
	if (!texture || texture->getType() < GlslType::Texture2D)
		return false;

	Ref< GlslVariable > texCoord = cx.emitInput(node, L"TexCoord");
	if (!texCoord || texCoord->getType() < GlslType::Integer || texCoord->getType() > GlslType::Float4)
		return false;

	Ref< GlslVariable > mip = cx.emitInput(node, L"Mip");

	SamplerState samplerState = node->getSamplerState();
	if (samplerState.ignoreMips)
		samplerState.mipFilter = Filter::Point;
	if (texture->getType() == GlslType::Texture2D)
		samplerState.addressW = Address::Wrap;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", (samplerState.compare == CompareFunction::None) ? GlslType::Float4 : GlslType::Float);

	const std::wstring textureName = texture->getName();
	const bool needAddressW = bool(texture->getType() > GlslType::Texture2D);
	std::wstring samplerName;

	// Check if we already have a suitable sampler in the layout.
	for (auto resource : cx.getLayout().get())
	{
		if (auto sampler = dynamic_type_cast< GlslSampler* >(resource))
		{
			const auto& rh = sampler->getState();
			if (
				rh.minFilter == samplerState.minFilter &&
				rh.mipFilter == samplerState.mipFilter &&
				rh.magFilter == samplerState.magFilter &&
				rh.addressU == samplerState.addressU &&
				rh.addressV == samplerState.addressV &&
				rh.addressW == samplerState.addressW &&
				rh.compare == samplerState.compare &&
				abs(rh.mipBias - samplerState.mipBias) <= FUZZY_EPSILON &&
				rh.ignoreMips == samplerState.ignoreMips &&
				rh.useAnisotropic == samplerState.useAnisotropic
			)
			{
				samplerName = sampler->getName();
				sampler->addStage(getBindStage(cx));
				break;
			}
		}
	}

	// Define new sampler in layout if no matching one was found.
	if (samplerName.empty())
	{
		samplerName = str(L"_gl_sampler%d", cx.getLayout().count< GlslSampler >());
		cx.getLayout().add(
			new GlslSampler(
				samplerName,
				GlslResource::Set::Default,
				getBindStage(cx),
				samplerState
			)
		);
	}

	comment(f, node);
	if (cx.inFragment())
	{
		if (samplerState.compare == CompareFunction::None)
		{
			if (!mip && !samplerState.ignoreMips)
			{
				const float bias = samplerState.mipBias;
				switch (texture->getType())
				{
				case GlslType::Texture2D:
					if (std::abs(bias) < FUZZY_EPSILON)
						assign(f, out) << L"texture(sampler2D(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float2) << L");" << Endl;
					else
						assign(f, out) << L"texture(sampler2D(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float2) << L", " << formatFloat(bias) << L");" << Endl;
					break;

				case GlslType::Texture3D:
					assign(f, out) << L"texture(sampler3D(__bindlessTextures3D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L");" << Endl;
					break;

				case GlslType::TextureCube:
					assign(f, out) << L"texture(samplerCube(__bindlessTexturesCube__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L");" << Endl;
					break;

				default:
					return false;
				}
			}
			else
			{
				switch (texture->getType())
				{
				case GlslType::Texture2D:
					assign(f, out) << L"textureLod(sampler2D(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float2) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
					break;

				case GlslType::Texture3D:
					assign(f, out) << L"textureLod(sampler3D(__bindlessTextures3D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
					break;

				case GlslType::TextureCube:
					assign(f, out) << L"textureLod(samplerCube(__bindlessTexturesCube__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
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
				case GlslType::Texture2D:
					assign(f, out) << L"texture(sampler2DShadow(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L");" << Endl;
					break;

				case GlslType::Texture3D:
					assign(f, out) << L"texture(sampler3DShadow(__bindlessTextures3D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5)));" << Endl;
					break;

				case GlslType::TextureCube:
					assign(f, out) << L"texture(samplerCubeShadow(__bindlessTexturesCube__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5)));" << Endl;
					break;

				default:
					return false;
				}
			}
			else
			{
				switch (texture->getType())
				{
				case GlslType::Texture2D:
					assign(f, out) << L"textureLod(sampler2DShadow(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
					break;

				case GlslType::Texture3D:
					assign(f, out) << L"textureLod(sampler3DShadow(__bindlessTextures3D__[" << textureName << L"], " << samplerName << L"), " <<  texCoord->cast(GlslType::Float4) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
					break;

				case GlslType::TextureCube:
					assign(f, out) << L"textureLod(samplerCubeShadow(__bindlessTexturesCube__[" << textureName << L"], " << samplerName << L"), " <<  texCoord->cast(GlslType::Float4) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
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
		case GlslType::Texture2D:
			assign(f, out) << L"textureLod(sampler2D(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float2) << L", 0.0);" << Endl;
			break;

		case GlslType::Texture3D:
		case GlslType::TextureCube:
        default:
            return false;
		}
	}

	if (cx.inCompute())
	{
		if (samplerState.compare == CompareFunction::None)
		{
			switch (texture->getType())
			{
			case GlslType::Texture2D:
				assign(f, out) << L"textureLod(sampler2D(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float2) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
				break;

			case GlslType::Texture3D:
				assign(f, out) << L"textureLod(sampler3D(__bindlessTextures3D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
				break;

			case GlslType::TextureCube:
				assign(f, out) << L"textureLod(samplerCube(__bindlessTexturesCube__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
				break;

			default:
				return false;
			}
		}
		else	// Compare
		{
			switch (texture->getType())
			{
			case GlslType::Texture2D:
				assign(f, out) << L"textureLod(sampler2DShadow(__bindlessTextures2D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float3) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
				break;

			case GlslType::Texture3D:
				assign(f, out) << L"textureLod(sampler3DShadow(__bindlessTextures3D__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float4) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
				break;

			case GlslType::TextureCube:
				assign(f, out) << L"textureLod(samplerCubeShadow(__bindlessTexturesCube__[" << textureName << L"], " << samplerName << L"), " << texCoord->cast(GlslType::Float4) << L", " << (mip != nullptr ? mip->cast(GlslType::Float) : L"0.0") << L");" << Endl;
				break;

			default:
				return false;
			}
		}
	}

	return true;
}

bool emitScalar(GlslContext& cx, Scalar* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	comment(f, node);

	const float v = std::abs(node->get());
	if ((v - std::floor(v)) < FUZZY_EPSILON)
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Integer);
		f << L"const int " << out->getName() << L" = " << (int32_t)node->get() << L";" << Endl;
	}
	else
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);
		f << L"const float " << out->getName() << L" = " << formatFloat(node->get()) << L";" << Endl;
	}

	return true;
}

bool emitSign(GlslContext& cx, Sign* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"sign(" << in->getName() << L");" << Endl;

	return true;
}

bool emitScript(GlslContext& cx, Script* node)
{
	// If script is a root node then enter appropriate shader stage.
	if (!node->getTechnique().empty())
	{
		cx.enterCompute();
		cx.requirements().localSize[0] = node->getLocalSize()[0];
		cx.requirements().localSize[1] = node->getLocalSize()[1];
		cx.requirements().localSize[2] = node->getLocalSize()[2];
	}

	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	std::wstring script = node->getScript();
	if (script.empty())
		return false;

	// Register modules.
	for (const auto& moduleId : node->getIncludes())
		cx.getShader().addModule(moduleId);

	// Emit input and outputs.
	const int32_t inputPinCount = node->getInputPinCount();
	const int32_t outputPinCount = node->getOutputPinCount();

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
		f << glsl_type_name(outs[i]->getType()) << L" " << outs[i]->getName() << L";" << Endl;
	}

	RefArray< GlslVariable > ins(inputPinCount);
	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		Node* inputNode = cx.getInputNode(inputPin);
		if (auto state = dynamic_type_cast< TextureState* >(inputNode))
		{
			SamplerState samplerState = state->getSamplerState();
			if (samplerState.ignoreMips)
				samplerState.mipFilter = Filter::Point;

			std::wstring samplerName;

			// Check if we already have a suitable sampler in the layout.
			for (auto resource : cx.getLayout().get())
			{
				if (auto sampler = dynamic_type_cast< GlslSampler* >(resource))
				{
					if (std::memcmp(&sampler->getState(), &samplerState, sizeof(SamplerState)) == 0)
					{
						samplerName = sampler->getName();
						sampler->addStage(getBindStage(cx));
						break;
					}
				}
			}

			// Define new sampler in layout if no matching one was found.
			if (samplerName.empty())
			{
				samplerName = str(L"_gl_sampler%d", cx.getLayout().count< GlslSampler >());
				cx.getLayout().add(
					new GlslSampler(
						samplerName,
						GlslResource::Set::Default,
						getBindStage(cx),
						samplerState
					)
				);
			}

			ins[i] = new GlslVariable(nullptr, samplerName, GlslType::Void);
		}
		else if (auto indexedUniform = dynamic_type_cast< IndexedUniform* >(inputNode))
		{
			const Node* indexNode = cx.getInputNode(node, L"Index");
			if (!indexNode)
			{
				const auto parameterType = glsl_from_parameter_type(indexedUniform->getParameterType());

				// Add uniform to layout.
				if (parameterType < GlslType::Texture2D)
				{
					auto ub = cx.getLayout().getByName< GlslUniformBuffer >(c_uniformBufferNames[(int32_t)indexedUniform->getFrequency()]);
					ub->addStage(getBindStage(cx));
					ub->add(indexedUniform->getParameterName(), parameterType, indexedUniform->getLength());
				}
				else
					return false;

				// Define parameter in context.
				cx.addParameter(
					indexedUniform->getParameterName(),
					indexedUniform->getParameterType(),
					indexedUniform->getLength(),
					indexedUniform->getFrequency()
				);

				ins[i] = new GlslVariable(nullptr, indexedUniform->getParameterName(), parameterType);
			}
			else
			{
				ins[i] = cx.emitInput(node->getInputPin(i));
				if (!ins[i])
					return false;
			}
		}
		else
		{
			ins[i] = cx.emitInput(node->getInputPin(i));
			if (!ins[i])
				return false;
		}
	}

	typedef std::pair< std::wstring, std::wstring > repair_t;
	AlignedVector< repair_t > reps;

	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		const std::wstring variableName = L"$" + node->getInputPin(i)->getName();
		switch (ins[i]->getType())
		{
		case GlslType::Texture2D:
			reps.push_back({ variableName, L"__bindlessTextures2D__[" + ins[i]->getName() + L"]" });
			break;

		case GlslType::Texture3D:
			reps.push_back({ variableName, L"__bindlessTextures3D__[" + ins[i]->getName() + L"]" });
			break;

		case GlslType::TextureCube:
			reps.push_back({ variableName, L"__bindlessTexturesCube__[" + ins[i]->getName() + L"]" });
			break;

		case GlslType::Image2D:
			reps.push_back({ variableName, L"__bindlessImages2D__[" + ins[i]->getName() + L"]" });
			break;

		case GlslType::Image3D:
			reps.push_back({ variableName, L"__bindlessImages3D__[" + ins[i]->getName() + L"]" });
			break;

		case GlslType::ImageCube:
			reps.push_back({ variableName, L"__bindlessImagesCube__[" + ins[i]->getName() + L"]" });
			break;

		default:
			reps.push_back({ variableName, ins[i]->getName() });
			break;
		}
	}

	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const std::wstring variableName = L"$" + node->getOutputPin(i)->getName();
		reps.push_back({ variableName, outs[i]->getName() });
	}

	// Sort to ensure longer names get replaced first.
	std::sort(reps.begin(), reps.end(), [](const repair_t& lh, const repair_t& rh) {
		return lh.first.length() > rh.first.length();
	});

	for (const auto& rep : reps)
		script = replaceAll(script, rep.first, rep.second);

	f << L"{" << Endl;
	f << IncreaseIndent;
	f << script << Endl;
	f << DecreaseIndent;
	f << L"}" << Endl;
	return true;
}

bool emitSin(GlslContext& cx, Sin* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GlslType::Float)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
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

	comment(f, node);
	assign(f, out) << L"sqrt(" << in->getName() << L");" << Endl;

	return true;
}

bool emitStep(GlslContext& cx, Step* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in1 = cx.emitInput(node, L"Edge");
	Ref< GlslVariable > in2 = cx.emitInput(node, L"X");
	if (!in1 || !in2)
		return false;

	const GlslType type = glsl_precedence(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << L"step(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;

	return true;
}

bool emitStruct(GlslContext& cx, Struct* node)
{
	Ref< GlslVariable > out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName() + L"_Data",
		GlslType::StructBuffer
	);

	const auto existing = cx.getLayout().getByName(node->getParameterName());
	if (existing != nullptr)
	{
		if (auto existingStorageBuffer = dynamic_type_cast< GlslStorageBuffer* >(existing))
		{
			// Storage buffer already exist; \tbd ensure elements match.
			existingStorageBuffer->addStage(getBindStage(cx));
			return true;
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
		Ref< GlslStorageBuffer > storageBuffer = new GlslStorageBuffer(
			node->getParameterName(),
			GlslResource::Set::Default,
			getBindStage(cx)
		);
		for (const auto& element : node->getElements())
			storageBuffer->add(element.name, element.type);
		cx.getLayout().add(storageBuffer);
	}

	// Define parameter in context.
	cx.addParameter(
		node->getParameterName(),
		ParameterType::StructBuffer,
		1,
		UpdateFrequency::Draw
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

	const GlslType type = glsl_precedence(in1->getType(), in2->getType());
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;

	return true;
}

bool emitSum(GlslContext& cx, Sum* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	Ref< GlslVariable > N = cx.emitOutput(node, L"N", GlslType::Integer);
	T_ASSERT(N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Void);
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
	auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
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

	std::wstring inner = fs.str();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	comment(f, node);
	assignMutable(f, out) << expandScalar(0.0f, out->getType()) << L";" << Endl;

	// Write outer for-loop statement.
	f << L"int " << N->getName() << L";" << Endl;
	f << L"for (" << N->getName() << L" = " << node->getFrom() << L"; " << N->getName() << L" <= " << node->getTo() << L"; ++" << N->getName() << L")" << Endl;
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

bool emitSwizzle(GlslContext& cx, Swizzle* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	const std::wstring map = toLower(node->get());
	if (map.length() == 0)
		return false;

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	GlslType type = GlslType::Void;
	if (in->getType() >= GlslType::Integer && in->getType() <= GlslType::Integer4)
	{
		const GlslType types[] = { GlslType::Integer, GlslType::Integer2, GlslType::Integer3, GlslType::Integer4 };
		type = types[map.length() - 1];
	}
	else
	{
		const GlslType types[] = { GlslType::Float, GlslType::Float2, GlslType::Float3, GlslType::Float4 };
		type = types[map.length() - 1];
	}

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	bool containConstant = false;
	for (size_t i = 0; i < map.length() && !containConstant; ++i)
	{
		if (map[i] == L'0' || map[i] == L'1')
			containConstant = true;
	}

	StringOutputStream ss;
	if (containConstant || (map.length() > 1 && in->getType() == GlslType::Float))
	{
		ss << glsl_type_name(type) << L"(";
		for (size_t i = 0; i < map.length(); ++i)
		{
			if (i > 0)
				ss << L", ";
			switch (map[i])
			{
			case 'x':
				if (in->getType() == GlslType::Float)
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
		if (map[0] == L'x' && in->getType() == GlslType::Float)
			ss << in->getName();
		else
			ss << in->getName() << L'.' << map[0];
	}

	comment(f, node);
	assign(f, out) << ss.str() << L";" << Endl;
	return true;
}

bool emitSwitch(GlslContext& cx, Switch* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Select");
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
	AlignedVector< GlslVariable > caseInputs;
	AlignedVector< std::wstring > defaultBranches;
	AlignedVector< GlslVariable > defaultInputs;
	AlignedVector< GlslType > outputTypes;
	RefArray< GlslVariable > outs;

	// Emit output variables first due to scoping.
	for (int32_t i = 0; i < width; ++i)
	{
		const OutputPin* outputPin = node->getOutputPin(i);
		outs.push_back(cx.emitOutput(outputPin, GlslType::Void));
		outputTypes.push_back(GlslType::Void);
	}

	// Conditional branches.
	for (int32_t i = 0; i < (int32_t)cases.size(); ++i)
	{
		cx.getShader().pushScope();
		for (int32_t j = 0; j < width; ++j)
		{
			auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);

			const InputPin* caseInput = node->getInputPin(1 + width + i * width + j);
			if (!caseInput)
			{
				cx.getShader().popScope();
				return false;
			}

			Ref< GlslVariable > caseInputVariable = cx.emitInput(caseInput);
			if (!caseInputVariable)
			{
				cx.getShader().popScope();
				return false;
			}

			caseBranches.push_back(fs.str());
			caseInputs.push_back(*caseInputVariable);
			outputTypes[j] = std::max(outputTypes[j], caseInputVariable->getType());

			cx.getShader().popOutputStream(GlslShader::BtBody);
		}
		cx.getShader().popScope();
	}

	// Default branches.
	{
		cx.getShader().pushScope();
		for (int32_t i = 0; i < width; ++i)
		{
			auto& fs = cx.getShader().pushOutputStream(GlslShader::BtBody, T_FILE_LINE_W);
	
			const InputPin* defaultInput = node->getInputPin(1 + i);
			if (!defaultInput)
			{
				cx.getShader().popScope();
				return false;
			}

			Ref< GlslVariable > defaultInputVariable = cx.emitInput(defaultInput);
			if (!defaultInputVariable)
			{
				cx.getShader().popScope();
				return false;
			}

			defaultBranches.push_back(fs.str());
			defaultInputs.push_back(*defaultInputVariable);
			outputTypes[i] = std::max(outputTypes[i], defaultInputVariable->getType());

			cx.getShader().popOutputStream(GlslShader::BtBody);
		}
		cx.getShader().popScope();
	}

	// Modify output type to match common output type of cases,
	// initialize output variable to zero.
	for (int32_t i = 0; i < width; ++i)
	{
		outs[i]->setType(outputTypes[i]);
		assignMutable(f, outs[i]) << expandScalar(0.0f, outputTypes[i]) << L";" << Endl;
	}

	comment(f, node);

	const bool supportControlFlowAttributes = (cx.getSettings() != nullptr ? cx.getSettings()->getProperty< bool >(L"Glsl.Vulkan.ControlFlowAttributes", true) : true);
	if (supportControlFlowAttributes)
	{
		if (node->getBranch() == Switch::Branch::Static)
			f << L"[[flatten]]" << Endl;
		else if (node->getBranch() == Switch::Branch::Dynamic)
			f << L"[[branch]]" << Endl;
	}

	for (int32_t i = 0; i < (int32_t)cases.size(); ++i)
	{
		f << (i == 0 ? L"if (" : L"else if (") << in->cast(GlslType::Integer) << L" == " << cases[i] << L")" << Endl;
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

bool emitTan(GlslContext& cx, Tan* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GlslType::Float)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float);

	comment(f, node);
	assign(f, out) << L"tan(" << theta->getName() << L");" << Endl;

	return true;
}

bool emitTargetSize(GlslContext& cx, TargetSize* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float2);

	cx.requirements().useTargetSize = true;

	comment(f, node);
	assign(f, out) << L"_vk_targetSize.xy;" << Endl;

	return true;
}

bool emitTextureSize(GlslContext& cx, TextureSize* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	const std::wstring textureName = in->getName();
	Ref< GlslVariable > out;

	comment(f, node);
	switch (in->getType())
	{
	case GlslType::Texture2D:
		out = cx.emitOutput(node, L"Output", GlslType::Float2);
		f << L"const vec2 " << out->getName() << L" = textureSize(__bindlessTextures2D__[" << textureName << L"], 0);" << Endl;
		break;

	case GlslType::Texture3D:
		out = cx.emitOutput(node, L"Output", GlslType::Float3);
		f << L"const vec3 " << out->getName() << L" = textureSize(__bindlessTextures3D__[" << textureName << L"], 0);" << Endl;
		break;

	case GlslType::TextureCube:
		out = cx.emitOutput(node, L"Output", GlslType::Float3);
		f << L"const vec3 " << out->getName() << L" = textureSize(__bindlessTexturesCube__[" << textureName << L"], 0);" << Endl;
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

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float4);

	comment(f, node);
	assign(f, out) << transform->getName() << L" * " << in->cast(GlslType::Float4) << L";" << Endl;

	return true;
}

bool emitTranspose(GlslContext& cx, Transpose* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", in->getType());

	comment(f, node);
	assign(f, out) << L"transpose(" << in->getName() << L");" << Endl;

	return true;
}

bool emitTruncate(GlslContext& cx, Truncate* node)
{
	auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	Ref< GlslVariable > in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	const GlslType type = glsl_degrade_to_integer(in->getType());
	if (type == GlslType::Void)
		return false;

	Ref< GlslVariable > out = cx.emitOutput(node, L"Output", type);

	comment(f, node);
	assign(f, out) << glsl_type_name(type) << L"(trunc(" << in->getName() << L"));" << Endl;

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
	if (out->getType() < GlslType::Texture2D)
	{
		// Scalar parameter, add to uniform buffer.
		auto ub = cx.getLayout().getByName< GlslUniformBuffer >(c_uniformBufferNames[(int32_t)node->getFrequency()]);
		ub->addStage(getBindStage(cx));
		if (!ub->add(node->getParameterName(), out->getType(), 1))
			return false;
	}
	else if (out->getType() >= GlslType::Texture2D && out->getType() <= GlslType::TextureCube)
	{
		const auto existing = cx.getLayout().getByName(node->getParameterName());
		if (existing != nullptr)
		{
			if (auto existingTexture = dynamic_type_cast< GlslTexture* >(existing))
			{
				// Texture already exist; ensure type match.
				if (existingTexture->getUniformType() != out->getType())
					return false;
				existingTexture->addStage(getBindStage(cx));
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
			cx.getLayout().addBindless(
				new GlslTexture(
					node->getParameterName(),
					GlslResource::Set::Default,
					getBindStage(cx),
					out->getType(),
					false
				)
			);
		}

		// Texture parameter; since resource index is passed to shader we define an integer uniform.
		auto ub = cx.getLayout().getByName< GlslUniformBuffer >(L"UbDraw"); // c_uniformBufferNames[(int32_t)node->getFrequency()]);
		ub->addStage(getBindStage(cx));
		if (!ub->add(node->getParameterName(), GlslType::Integer, 1))
			return false;
	}
	else if (out->getType() >= GlslType::Image2D && out->getType() <= GlslType::ImageCube)
	{
		const auto existing = cx.getLayout().getByName(node->getParameterName());
		if (existing != nullptr)
		{
			if (auto existingImage = dynamic_type_cast< GlslImage* >(existing))
			{
				// Image already exist; ensure type match.
				if (existingImage->getUniformType() != out->getType())
					return false;
				existingImage->addStage(getBindStage(cx));
			}
			else
			{
				// Resource already exist but is not an image.
				return false;
			}
		}
		else
		{
			// Image do not exist; add new image resource.
			cx.getLayout().addBindless(
				new GlslImage(
					node->getParameterName(),
					GlslResource::Set::Default,
					getBindStage(cx),
					out->getType(),
					false
				)
			);
		}

		// Image parameter; since resource index is passed to shader we define an integer uniform.
		auto ub = cx.getLayout().getByName< GlslUniformBuffer >(L"UbDraw"); // c_uniformBufferNames[(int32_t)node->getFrequency()]);
		ub->addStage(getBindStage(cx));
		if (!ub->add(node->getParameterName(), GlslType::Integer, 1))
			return false;
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
	comment(f, node);

	float T_MATH_ALIGN16 e[4];
	node->get().storeAligned(e);

	bool integer = true;
	for (int32_t i = 0; i < 4; ++i)
	{
		if ((e[i] - std::floor(e[i])) >= FUZZY_EPSILON)
			integer = false;
	}

	if (integer)
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Integer4);
		f << L"const ivec4 " << out->getName() << L" = ivec4(" << (int32_t)e[0] << L", " << (int32_t)e[1] << L", " << (int32_t)e[2] << L", " << (int32_t)e[3] << L");" << Endl;
	}
	else
	{
		Ref< GlslVariable > out = cx.emitOutput(node, L"Output", GlslType::Float4);
		f << L"const vec4 " << out->getName() << L" = vec4(" << formatFloat(e[0]) << L", " << formatFloat(e[1]) << L", " << formatFloat(e[2]) << L", " << formatFloat(e[3]) << L");" << Endl;
	}

	return true;
}

bool emitVertexInput(GlslContext& cx, VertexInput* node)
{
	if (!cx.inVertex())
	{
		cx.pushError(str(L"VertexInput \"%s\" must be emitted in vertex scope.", node->getName().c_str()));
		return false;
	}

	Ref< GlslVariable > out = cx.getShader().getInputVariable(node->getName());
	if (!out)
	{
		const GlslType type = glsl_from_data_type(node->getDataType());
		const std::wstring attributeName = glsl_vertex_attribute_name(node->getDataUsage(), node->getIndex());

		auto& fi = cx.getVertexShader().getOutputStream(GlslShader::BtInput);
		fi << L"layout (location = " << glsl_vertex_attribute_location(node->getDataUsage(), node->getIndex()) << L") in " << glsl_type_name(type) << L" " << attributeName << L";" << Endl;

		if (node->getDataUsage() == DataUsage::Position && type != GlslType::Float4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				GlslType::Float4
			);
			auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
			switch (type)
			{
			case GlslType::Float:
				comment(f, node);
				assign(f, out) << L"vec4(" << attributeName << L", 0.0, 0.0, 1.0);" << Endl;
				break;

			case GlslType::Float2:
				comment(f, node);
				assign(f, out) << L"vec4(" << attributeName << L".xy, 0.0, 1.0);" << Endl;
				break;

			case GlslType::Float3:
				comment(f, node);
				assign(f, out) << L"vec4(" << attributeName << L".xyz, 1.0);" << Endl;
				break;

			default:
				break;
			}
		}
		else if (node->getDataUsage() == DataUsage::Normal && type != GlslType::Float4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				GlslType::Float4
			);
			auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
			switch (type)
			{
			case GlslType::Float:
				comment(f, node);
				assign(f, out) << L"vec4(" << attributeName << L", 0.0, 0.0, 0.0);" << Endl;
				break;

			case GlslType::Float2:
				comment(f, node);
				assign(f, out) << L"vec4(" << attributeName << L".xy, 0.0, 0.0);" << Endl;
				break;

			case GlslType::Float3:
				comment(f, node);
				assign(f, out) << L"vec4(" << attributeName << L".xyz, 0.0);" << Endl;
				break;

			default:
				break;
			}
		}
		else if (node->getDataUsage() == DataUsage::Color)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				type
			);
			auto& f = cx.getShader().getOutputStream(GlslShader::BtBody);
			switch (type)
			{
			case GlslType::Float:
				comment(f, node);
				assign(f, out) << attributeName << L".z;" << Endl;
				break;

			case GlslType::Float2:
				comment(f, node);
				assign(f, out) << attributeName << L".zy;" << Endl;
				break;

			case GlslType::Float3:
				comment(f, node);
				assign(f, out) << attributeName << L".zyx;" << Endl;
				break;

			case GlslType::Float4:
				comment(f, node);
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
	{
		cx.pushError(L"Input not connected.");
		return false;
	}

	auto& f = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
	switch (in->getType())
	{
	case GlslType::Float:
		comment(f, node);
		f << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * vec4(" << in->getName() << L", 0.0, 0.0, 1.0);" << Endl;
		break;

	case GlslType::Float2:
		comment(f, node);
		f << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * vec4(" << in->getName() << L".xy, 0.0, 1.0);" << Endl;
		break;

	case GlslType::Float3:
		comment(f, node);
		f << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * vec4(" << in->getName() << L".xyz, 1.0);" << Endl;
		break;

	case GlslType::Float4:
		comment(f, node);
		f << L"gl_Position = vec4(1.0f, -1.0f, 1.0f, 1.0f) * " << in->getName() << L";" << Endl;
		break;

	default:
		break;
	}

	cx.requirements().vertexPrecisionHint = node->getPrecisionHint();
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

	explicit EmitterCast(function_t function)
	:	m_function(function)
	{
	}

	virtual bool emit(GlslContext& c, Node* node) override final
	{
		T_ASSERT(is_a< NodeType >(node));
		const bool result = (*m_function)(c, static_cast< NodeType* >(node));
		return result;
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
	m_emitters[&type_of< Discard >()] = new EmitterCast< Discard >(emitDiscard);
	m_emitters[&type_of< DispatchIndex >()] = new EmitterCast< DispatchIndex >(emitDispatchIndex);
	m_emitters[&type_of< Div >()] = new EmitterCast< Div >(emitDiv);
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
	m_emitters[&type_of< ReadStruct2 >()] = new EmitterCast< ReadStruct2 >(emitReadStruct2);
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

#pragma optimize( "", off )

bool GlslEmitter::emit(GlslContext& c, Node* node)
{
	auto i = m_emitters.find(&type_of(node));
	if (i == m_emitters.end() || i->second == nullptr)
	{
		log::error << L"No emitter for node " << type_name(node) << L"." << Endl;
		return false;
	}
	const bool result = i->second->emit(c, node);
	if (!result)
		log::debug << L"Emitter " << type_name(node) << L" failed." << Endl;
	return result;
}

#pragma optimize ( "", on )

}
