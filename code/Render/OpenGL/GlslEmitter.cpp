#include <iomanip>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/GlslContext.h"
#include "Render/OpenGL/GlslEmitter.h"
#include "Render/OpenGL/Platform.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/Script.h"

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

StringOutputStream& assign(StringOutputStream& f, GlslVariable* out)
{
	f << glsl_type_name(out->getType()) << L" " << out->getName() << L" = ";
	return f;
}

bool emitAbs(GlslContext& cx, Abs* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"abs(" << in->getName() << L");" << Endl;
	return true;
}

bool emitAdd(GlslContext& cx, Add* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitArcusCos(GlslContext& cx, ArcusCos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"acos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitArcusTan(GlslContext& cx, ArcusTan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* xy = cx.emitInput(node, L"XY");
	if (!xy || xy->getType() != GtFloat2)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"atan(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;
	return true;
}

bool emitClamp(GlslContext& cx, Clamp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"clamp(" << in->getName() << L", " << formatFloat(node->getMin()) << L", " << formatFloat(node->getMax()) << L");" << Endl;
	return true;
}

bool emitColor(GlslContext& cx, Color* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);
	if (!out)
		return false;
	traktor::Color4ub color = node->getColor();
	f << L"const vec4 " << out->getName() << L" = vec4(" << (color.r / 255.0f) << L", " << (color.g / 255.0f) << L", " << (color.b / 255.0f) << L", " << (color.a / 255.0f) << L");" << Endl;
	return true;
}

bool emitConditional(GlslContext& cx, Conditional* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Emit input and reference branches.
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return false;

	GlslVariable caseTrue, caseFalse;
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

		cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		GlslVariable* ct = cx.emitInput(node, L"CaseTrue");
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

		GlslVariable* cf = cx.emitInput(node, L"CaseFalse");
		if (!cf)
			return false;

		caseFalse = *cf;
		caseFalseBranch = fs.str();

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	// Create output variable.
	GlslType outputType = std::max< GlslType >(caseTrue.getType(), caseFalse.getType());

	GlslVariable* out = cx.emitOutput(node, L"Output", outputType);
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

bool emitCos(GlslContext& cx, Cos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"cos(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitCross(GlslContext& cx, Cross* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat3);
	assign(f, out) << L"cross(" << in1->cast(GtFloat3) << L", " << in2->cast(GtFloat3) << L");" << Endl;
	return true;
}

bool emitDerivative(GlslContext& cx, Derivative* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* input = cx.emitInput(node, L"Input");
	if (!input)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", input->getType());
	switch (node->getAxis())
	{
	case Derivative::DaX:
		assign(f, out) << L"dFdx(" << input->getName() << L");" << Endl;
		break;
	case Derivative::DaY:
		assign(f, out) << L"dFdy(" << input->getName() << L");" << Endl;
		break;
	}
	cx.setRequireDerivatives();
	return true;
}

bool emitDiscard(GlslContext& cx, Discard* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Emit input and reference branches.
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* ref = cx.emitInput(node, L"Reference");
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
		T_ASSERT (0);
	}

	f << L"\tdiscard;" << Endl;

	GlslVariable* pass = cx.emitInput(node, L"Pass");
	if (!pass)
		return false;

	GlslVariable* out = cx.emitOutput(node, L"Output", pass->getType());
	assign(f, out) << pass->getName() << L";" << Endl;

	return true;
}

bool emitDiv(GlslContext& cx, Div* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitDot(GlslContext& cx, Dot* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	assign(f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitExp(GlslContext& cx, Exp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"exp(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFraction(GlslContext& cx, Fraction* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"fract(" << in->getName() << L");" << Endl;
	return true;
}

bool emitFragmentPosition(GlslContext& cx, FragmentPosition* node)
{
	if (!cx.inFragment())
		return false;

	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat2);
	assign(f, out) << L"gl_FragCoord.xy;" << Endl;

	return true;
}

bool emitFrontFace(GlslContext& cx, FrontFace* node)
{
	if (!cx.inFragment())
		return false;

	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"gl_FrontFacing ? 1.0 : 0.0;" << Endl;

	return true;
}

bool emitIndexedUniform(GlslContext& cx, IndexedUniform* node)
{
	const Node* indexNode = cx.getInputNode(node, L"Index");
	if (!indexNode)
		return false;

	GlslVariable* out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		glsl_from_parameter_type(node->getParameterType())
	);

	StringOutputStream& fb = cx.getShader().getOutputStream(GlslShader::BtBody);

	if (const Scalar* scalarIndexNode = dynamic_type_cast< const Scalar* >(indexNode))
		assign(fb, out) << node->getParameterName() << L"[" << int32_t(scalarIndexNode->get()) << L"];" << Endl;
	else
	{
		GlslVariable* index = cx.emitInput(node, L"Index");
		if (!index)
			return false;

		assign(fb, out) << node->getParameterName() << L"[int(" << index->getName() << L")];" << Endl;
	}

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
		fu << L"uniform " << glsl_type_name(out->getType()) << L" " << node->getParameterName() << L"[" << node->getLength() << L"];" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}

	switch (out->getType())
	{
	case GtFloat:
		if (!cx.defineUniform(node->getParameterName(), GL_FLOAT, node->getLength()))
			return false;
		break;

	case GtFloat2:
	case GtFloat3:
	case GtFloat4:
		if (!cx.defineUniform(node->getParameterName(), GL_FLOAT_VEC4, node->getLength()))
			return false;
		break;

	case GtFloat4x4:
		if (!cx.defineUniform(node->getParameterName(), GL_FLOAT_MAT4, node->getLength()))
			return false;
		break;

	default:
		return false;
	}

	return true;
}

bool emitInstance(GlslContext& cx, Instance* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
#if !defined(T_OPENGL_ES2)
	assign(f, out) << L"float(gl_InstanceID);" << Endl;
#else
	const PropertyGroup* settings = cx.getSettings();
	if (settings && settings->getProperty< PropertyBoolean >(L"Glsl.ES2.SupportHwInstancing", false))
		assign(f, out) << L"float(gl_InstanceIDEXT);" << Endl;
	else
		assign(f, out) << L"_gl_instanceID;" << Endl;
#endif
	return true;
}

bool emitInterpolator(GlslContext& cx, Interpolator* node)
{
	if (!cx.inFragment())
	{
		// We're already in vertex state; skip interpolation.
		GlslVariable* in = cx.emitInput(node, L"Input");
		if (!in)
		{
			log::error << L"Unable to emit interpolator input (1)" << Endl;
			return false;
		}

		GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());

		StringOutputStream& fb = cx.getShader().getOutputStream(GlslShader::BtBody);
		assign(fb, out) << in->getName() << L";" << Endl;

		return true;
	}

	cx.enterVertex();

	GlslVariable* in = cx.emitInput(node, L"Input");
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

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
	fb << interpolatorMask << L" = " << in->getName() << L";" << Endl;

	cx.getFragmentShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		interpolatorMask,
		in->getType()
	);

	if (declare)
	{
		StringOutputStream& fvo = cx.getVertexShader().getOutputStream(GlslShader::BtOutput);
		StringOutputStream& fpi = cx.getFragmentShader().getOutputStream(GlslShader::BtInput);

#if !defined(T_OPENGL_ES2)
		fvo << L"out vec4 " << interpolatorName << L";" << Endl;
		fpi << L"in vec4 " << interpolatorName << L";" << Endl;
#else
		fvo << L"varying vec4 " << interpolatorName << L";" << Endl;
		fpi << L"varying vec4 " << interpolatorName << L";" << Endl;
#endif
	}

	return true;
}

bool emitIterate(GlslContext& cx, Iterate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	GlslVariable* N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	GlslVariable* out = cx.emitOutput(node, L"Output", GtVoid);
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
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		GlslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		// Emit post condition if connected; break iteration if condition is false.
		GlslVariable* condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(GtFloat) << L" == 0.0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	GlslVariable* initial = cx.emitInput(node, L"Initial");
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

bool emitIterate2d(GlslContext& cx, Iterate2d* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variables.
	GlslVariable* X = cx.emitOutput(node, L"X", GtFloat);
	T_ASSERT (X);

	GlslVariable* Y = cx.emitOutput(node, L"Y", GtFloat);
	T_ASSERT (Y);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	GlslVariable* out = cx.emitOutput(node, L"Output", GtVoid);
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
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	GlslVariable* input = cx.emitInput(node, L"Input");
	if (!input)
		return false;

	// Emit post condition if connected; break iteration if condition is false.
	GlslVariable* condition = cx.emitInput(node, L"Condition");
	if (condition)
	{
		fs << L"if (" << condition->cast(GtFloat) << L" == 0.0)" << Endl;
		fs << L"\tbreak;" << Endl;
	}

	inputName = input->getName();

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	out->setType(input->getType());

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	GlslVariable* initial = cx.emitInput(node, L"Initial");
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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"length(" << in->getName() << L");" << Endl;
	return true;
}

bool emitLerp(GlslContext& cx, Lerp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* blend = cx.emitInput(node, L"Blend");
	if (!blend || blend->getType() != GtFloat)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"mix(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;
	return true;
}

bool emitLog(GlslContext& cx, Log* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* xaxis = cx.emitInput(node, L"XAxis");
	GlslVariable* yaxis = cx.emitInput(node, L"YAxis");
	GlslVariable* zaxis = cx.emitInput(node, L"ZAxis");
	GlslVariable* translate = cx.emitInput(node, L"Translate");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4x4);
	f << L"mat4 " << out->getName() << L" = mat4(" << Endl;
	f << IncreaseIndent;
	f << (xaxis     ? xaxis->cast(GtFloat4)     : L"vec4(1.0, 0.0, 0.0, 0.0)") << L"," << Endl;
	f << (yaxis     ? yaxis->cast(GtFloat4)     : L"vec4(0.0, 1.0, 0.0, 0.0)") << L"," << Endl;
	f << (zaxis     ? zaxis->cast(GtFloat4)     : L"vec4(0.0, 0.0, 1.0, 0.0)") << L"," << Endl;
	f << (translate ? translate->cast(GtFloat4) : L"vec4(0.0, 0.0, 0.0, 1.0)") << Endl;
	f << DecreaseIndent;
	f << L");" << Endl;
	return true;
}

bool emitMatrixOut(GlslContext& cx, MatrixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	GlslVariable* xaxis = cx.emitOutput(node, L"XAxis", GtFloat4);
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

	GlslVariable* yaxis = cx.emitOutput(node, L"YAxis", GtFloat4);
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

	GlslVariable* zaxis = cx.emitOutput(node, L"ZAxis", GtFloat4);
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

	GlslVariable* translate = cx.emitOutput(node, L"Translate", GtFloat4);
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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"max(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMin(GlslContext& cx, Min* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitMixIn(GlslContext& cx, MixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* x = cx.emitInput(node, L"X");
	GlslVariable* y = cx.emitInput(node, L"Y");
	GlslVariable* z = cx.emitInput(node, L"Z");
	GlslVariable* w = cx.emitInput(node, L"W");

	if (!y && !z && !w)
	{
		GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
		assign(f, out) << L"(" << (x ? x->getName() : L"0.0") << L");" << Endl;
	}
	else if (!z && !w)
	{
		GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat2);
		assign(f, out) << L"vec2(" << (x ? x->getName() : L"0.0") << L", " << (y ? y->getName() : L"0.0") << L");" << Endl;
	}
	else if (!w)
	{
		GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat3);
		assign(f, out) << L"vec3(" << (x ? x->getName() : L"0.0") << L", " << (y ? y->getName() : L"0.0") << L", " << (z ? z->getName() : L"0.0") << L");" << Endl;
	}
	else
	{
		GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);
		assign(f, out) << L"vec4(" << (x ? x->getName() : L"0.0") << L", " << (y ? y->getName() : L"0.0") << L", " << (z ? z->getName() : L"0.0") << L", " << (w ? w->getName() : L"0.0") << L");" << Endl;
	}

	return true;
}

bool emitMixOut(GlslContext& cx, MixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	switch (in->getType())
	{
	case GtFloat:
		{
			GlslVariable* x = cx.emitOutput(node, L"X", GtFloat);
			assign(f, x) << in->getName() << L";" << Endl;
		}
		break;

	case GtFloat2:
		{
			GlslVariable* x = cx.emitOutput(node, L"X", GtFloat);
			GlslVariable* y = cx.emitOutput(node, L"Y", GtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
		}
		break;

	case GtFloat3:
		{
			GlslVariable* x = cx.emitOutput(node, L"X", GtFloat);
			GlslVariable* y = cx.emitOutput(node, L"Y", GtFloat);
			GlslVariable* z = cx.emitOutput(node, L"Z", GtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
			assign(f, y) << in->getName() << L".y;" << Endl;
			assign(f, z) << in->getName() << L".z;" << Endl;
		}
		break;

	case GtFloat4:
		{
			GlslVariable* x = cx.emitOutput(node, L"X", GtFloat);
			GlslVariable* y = cx.emitOutput(node, L"Y", GtFloat);
			GlslVariable* z = cx.emitOutput(node, L"Z", GtFloat);
			GlslVariable* w = cx.emitOutput(node, L"W", GtFloat);
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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitMulAdd(GlslContext& cx, MulAdd* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslVariable* in3 = cx.emitInput(node, L"Input3");
	if (!in1 || !in2 || !in3)
		return false;
	GlslType type = std::max< GlslType >(std::max< GlslType >(in1->getType(), in2->getType()), in3->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L" + " << in3->cast(type) << L";" << Endl;
	return true;
}

bool emitNeg(GlslContext& cx, Neg* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"-" << in->getName() << L";" << Endl;
	return true;
}

bool emitNormalize(GlslContext& cx, Normalize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"normalize(" << in->getName() << L");" << Endl;
	return true;
}

bool emitPixelOutput(GlslContext& cx, PixelOutput* node)
{
	const GLenum c_oglCullFace[] =
	{
		GL_FRONT,
		GL_BACK,
		GL_FRONT
	};

	const GLenum c_oglBlendEquation[] =
	{
		GL_FUNC_ADD,
		GL_FUNC_SUBTRACT,
		GL_FUNC_REVERSE_SUBTRACT,
#if defined(T_OPENGL_STD)
		GL_MIN,
		GL_MAX
#elif defined(T_OPENGL_ES2)
		GL_FUNC_ADD,
		GL_FUNC_ADD
#endif
	};

	const GLenum c_oglBlendFunction[] =
	{
		GL_ONE,
		GL_ZERO,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA
	};

	const GLenum c_oglFunction[] =
	{
		GL_ALWAYS,
		GL_NEVER,
		GL_LESS,
		GL_LEQUAL,
		GL_GREATER,
		GL_GEQUAL,
		GL_EQUAL,
		GL_NOTEQUAL
	};

	const GLenum c_oglStencilOperation[] =
	{
		GL_KEEP,
		GL_ZERO,
		GL_REPLACE,
		GL_INCR,
		GL_DECR,
		GL_INVERT,
		GL_INCR_WRAP,
		GL_DECR_WRAP
	};

	RenderState rs = node->getRenderState();

	cx.enterFragment();

	const wchar_t* inputs[] = { L"Input", L"Input1", L"Input2", L"Input3" };
	GlslVariable* in[4];

	for (int32_t i = 0; i < sizeof_array(in); ++i)
		in[i] = cx.emitInput(node, inputs[i]);

	if (!in[0])
		return false;

#if !defined(T_OPENGL_ES2)
	for (int32_t i = 0; i < sizeof_array(in); ++i)
	{
		if (!in[i])
			continue;

		StringOutputStream& fpb = cx.getFragmentShader().getOutputStream(GlslShader::BtBody);
		fpb << L"_gl_FragData_" << i << L" = " << in[i]->cast(GtFloat4) << L";" << Endl;
	}
#else
	StringOutputStream& fpb = cx.getFragmentShader().getOutputStream(GlslShader::BtBody);
	fpb << L"gl_FragColor" << L" = " << in[0]->cast(GtFloat4) << L";" << Endl;
#endif

	uint32_t colorMask =
		((rs.colorWriteMask & CwRed) ? RenderStateOpenGL::CmRed : 0) |
		((rs.colorWriteMask & CwGreen) ? RenderStateOpenGL::CmGreen : 0) |
		((rs.colorWriteMask & CwBlue) ? RenderStateOpenGL::CmBlue : 0) |
		((rs.colorWriteMask & CwAlpha) ? RenderStateOpenGL::CmAlpha : 0);

	RenderStateOpenGL& rsogl = cx.getRenderState();
	rsogl.cullFaceEnable = (rs.cullMode == CmNever) ? GL_FALSE : GL_TRUE;
	rsogl.cullFace = c_oglCullFace[rs.cullMode];
	rsogl.blendEnable = rs.blendEnable ? GL_TRUE : GL_FALSE;
	rsogl.blendEquation = c_oglBlendEquation[rs.blendOperation];
	rsogl.blendFuncSrc = c_oglBlendFunction[rs.blendSource];
	rsogl.blendFuncDest = c_oglBlendFunction[rs.blendDestination];
	rsogl.depthTestEnable = rs.depthEnable ? GL_TRUE : GL_FALSE;
	rsogl.colorMask = colorMask;
	rsogl.depthMask = rs.depthWriteEnable ? GL_TRUE : GL_FALSE;
	rsogl.depthFunc = c_oglFunction[rs.depthFunction];
	rsogl.alphaTestEnable = rs.alphaTestEnable ? GL_TRUE : GL_FALSE;
	rsogl.alphaFunc = c_oglFunction[rs.alphaTestFunction];
	rsogl.alphaRef = GLclampf(rs.alphaTestReference / 255.0f);
	rsogl.stencilTestEnable = rs.stencilEnable;
	rsogl.stencilFunc = c_oglFunction[rs.stencilFunction];
	rsogl.stencilRef = rs.stencilReference;
	rsogl.stencilOpFail = c_oglStencilOperation[rs.stencilFail];
	rsogl.stencilOpZFail = c_oglStencilOperation[rs.stencilZFail];
	rsogl.stencilOpZPass = c_oglStencilOperation[rs.stencilPass];

	return true;
}

bool emitPolynomial(GlslContext& cx, Polynomial* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	GlslVariable* x = cx.emitInput(node, L"X");
	GlslVariable* coeffs = cx.emitInput(node, L"Coefficients");
	if (!x || !coeffs)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);

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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* exponent = cx.emitInput(node, L"Exponent");
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!exponent || !in)
		return false;
	GlslType type = std::max< GlslType >(exponent->getType(), in->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"pow(max(" << in->cast(type) << L", 0.0), " << exponent->cast(type) << L");" << Endl;
	return true;
}

bool emitReflect(GlslContext& cx, Reflect* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* normal = cx.emitInput(node, L"Normal");
	GlslVariable* direction = cx.emitInput(node, L"Direction");
	if (!normal || !direction)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", direction->getType());
	assign(f, out) << L"reflect(" << direction->getName() << L", " << normal->cast(direction->getType()) << L");" << Endl;
	return true;
}

bool emitRecipSqrt(GlslContext& cx, RecipSqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"inversesqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitRepeat(GlslContext& cx, Repeat* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	GlslVariable* N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	GlslVariable* out = cx.emitOutput(node, L"Output", GtVoid);
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
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		// Emit pre-condition, break iteration if condition is false.
		GlslVariable* condition = cx.emitInput(node, L"Condition");
		if (condition)
		{
			fs << L"if (" << condition->cast(GtFloat) << L" == 0.0)" << Endl;
			fs << L"\tbreak;" << Endl;
		}

		GlslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
	}

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);

	// As we now know the type of output variable we can safely
	// initialize it.
	GlslVariable* initial = cx.emitInput(node, L"Initial");
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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
#if !defined(T_OPENGL_ES2)
	assign(f, out) << L"round(" << in->getName() << L");" << Endl;
#else
	assign(f, out) << L"trunc(" << in->getName() << L" + " << expandScalar(0.5f, in->getType()) << L");" << Endl;
#endif
	return true;
}

bool emitSampler(GlslContext& cx, Sampler* node)
{
	const GLenum c_glFilter[] =
	{
		GL_NEAREST,
		GL_LINEAR
	};

	const GLenum c_glWrap[] =
	{
		GL_REPEAT,
		GL_REPEAT,
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE
	};

	const GLenum c_glCompare[] =
	{
		GL_ALWAYS,
		GL_NEVER,
		GL_LESS,
		GL_LEQUAL,
		GL_GREATER,
		GL_GEQUAL,
		GL_EQUAL,
		GL_NOTEQUAL,
		GL_INVALID_ENUM
	};

	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	GlslVariable* texture = cx.emitInput(node, L"Texture");
	if (!texture || texture->getType() < GtTexture2D)
		return false;

	GlslVariable* texCoord = cx.emitInput(node, L"TexCoord");
	if (!texCoord)
		return false;

	const SamplerState& samplerState = node->getSamplerState();

	GlslVariable* out = cx.emitOutput(node, L"Output", (samplerState.compare == CfNone) ? GtFloat4 : GtFloat);

	bool needAddressW = bool(texture->getType() > GtTexture2D);
	GLenum target = GL_INVALID_ENUM;

	switch (texture->getType())
	{
	case GtTexture2D:
		target = GL_TEXTURE_2D;
		break;

	case GtTexture3D:
		target = 0x806F; // GL_TEXTURE_3D;
		break;

	case GtTextureCube:
		target = GL_TEXTURE_CUBE_MAP;
		break;

	default:
		return false;
	}

	// Calculate sampler hash.
	Adler32 samplerHash;
	samplerHash.feed(texture->getName());
	samplerHash.feed(samplerState.minFilter);
	samplerHash.feed(samplerState.mipFilter);
	samplerHash.feed(samplerState.magFilter);
	samplerHash.feed(samplerState.addressU);
	samplerHash.feed(samplerState.addressV);
	if (needAddressW)
		samplerHash.feed(samplerState.addressW);
	samplerHash.feed(samplerState.compare);

    // Use same stage index for both vertex and fragment shader.
    // Sampler name is defined by which stage it's associated with.
	int32_t stage;

	// Define sampler.
	std::wstring samplerName = L"_gl_sampler_" + texture->getName();
    bool defineStates = cx.defineSampler(samplerName, samplerHash.get(), target, texture->getName(), stage);

	if (defineStates)
	{
		RenderStateOpenGL& rs = cx.getRenderState();

		bool minLinear = samplerState.minFilter != FtPoint;
		bool mipLinear = samplerState.mipFilter != FtPoint;

		if (!minLinear && !mipLinear)
			rs.samplerStates[stage].minFilter = GL_NEAREST;
		else if (!minLinear && mipLinear)
			rs.samplerStates[stage].minFilter = GL_NEAREST_MIPMAP_LINEAR;
		else if (minLinear && !mipLinear)
			rs.samplerStates[stage].minFilter = GL_LINEAR_MIPMAP_NEAREST;
		else
			rs.samplerStates[stage].minFilter = GL_LINEAR_MIPMAP_LINEAR;

		rs.samplerStates[stage].magFilter = c_glFilter[samplerState.magFilter];
		rs.samplerStates[stage].wrapS = c_glWrap[samplerState.addressU];
		rs.samplerStates[stage].wrapT = c_glWrap[samplerState.addressV];

		if (needAddressW)
			rs.samplerStates[stage].wrapR = c_glWrap[samplerState.addressW];

		rs.samplerStates[stage].compare = c_glCompare[samplerState.compare];
	}

	if (cx.getShader().getUniforms().find(samplerName) == cx.getShader().getUniforms().end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
		if (samplerState.compare == CfNone)
		{
			switch (texture->getType())
			{
			case GtTexture2D:
#if defined(T_OPENGL_STD)
				fu << L"uniform sampler2D " << samplerName << L";" << Endl;
#elif defined(T_OPENGL_ES2)
				fu << L"uniform lowp sampler2D " << samplerName << L";" << Endl;
#endif
				break;

			case GtTexture3D:
				fu << L"uniform sampler3D " << samplerName << L";" << Endl;
				break;

			case GtTextureCube:
				fu << L"uniform samplerCube " << samplerName << L";" << Endl;
				break;

			default:
				return false;
			}
		}
		else
		{
			if (!cx.inFragment())
				return false;

			switch (texture->getType())
			{
			case GtTexture2D:
#if defined(T_OPENGL_STD)
				fu << L"uniform sampler2DShadow " << samplerName << L";" << Endl;
#elif defined(T_OPENGL_ES2)
				fu << L"uniform lowp sampler2DShadow " << samplerName << L";" << Endl;
#endif
				break;

			case GtTexture3D:
				fu << L"uniform sampler3DShadow " << samplerName << L";" << Endl;
				break;

			case GtTextureCube:
				fu << L"uniform samplerCubeShadow " << samplerName << L";" << Endl;
				break;

			default:
				return false;
			}
		}
		cx.getShader().addUniform(samplerName);
	}

	if (cx.inFragment())
	{
		if (samplerState.compare == CfNone)
		{
#if defined(T_OPENGL_STD)
			if (!samplerState.ignoreMips)
			{
				float bias = samplerState.mipBias;
				switch (texture->getType())
				{
				case GtTexture2D:
					if (std::abs(bias) < FUZZY_EPSILON)
						assign(f, out) << L"texture(" << samplerName << L", " << texCoord->cast(GtFloat2) << L");" << Endl;
					else
						assign(f, out) << L"texture(" << samplerName << L", " << texCoord->cast(GtFloat2) << L", " << formatFloat(bias) << L");" << Endl;
					break;

				case GtTexture3D:
				case GtTextureCube:
					assign(f, out) << L"texture(" << samplerName << L", " << texCoord->cast(GtFloat3) << L");" << Endl;
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
					assign(f, out) << L"textureLod(" << samplerName << L", " << texCoord->cast(GtFloat2) << L", 0.0);" << Endl;
					break;

				case GtTexture3D:
				case GtTextureCube:
					assign(f, out) << L"textureLod(" << samplerName << L", " << texCoord->cast(GtFloat3) << L", 0.0);" << Endl;
					break;

				default:
					return false;
				}
			}
#else
			switch (texture->getType())
			{
			case GtTexture2D:
				assign(f, out) << L"texture2D(" << samplerName << L", " << texCoord->cast(GtFloat2) << L");" << Endl;
				break;

			case GtTexture3D:
				assign(f, out) << L"texture3D(" << samplerName << L", " << texCoord->cast(GtFloat3) << L");" << Endl;
				break;

			case GtTextureCube:
				assign(f, out) << L"textureCube(" << samplerName << L", " << texCoord->cast(GtFloat3) << L");" << Endl;
				break;

			default:
				return false;
			}
#endif
		}
		else
		{
#if defined(T_OPENGL_STD)
			if (!samplerState.ignoreMips)
			{
				switch (texture->getType())
				{
				case GtTexture2D:
					assign(f, out) << L"texture(" << samplerName << L", " << texCoord->cast(GtFloat3) << L" * vec3(1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.5));" << Endl;
					break;

				case GtTexture3D:
				case GtTextureCube:
					assign(f, out) << L"texture(" << samplerName << L", " << texCoord->cast(GtFloat4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5)));" << Endl;
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
					assign(f, out) << L"textureLod(" << samplerName << L", " << texCoord->cast(GtFloat3) << L" * vec3(1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.5), 0.0);" << Endl;
					break;

				case GtTexture3D:
				case GtTextureCube:
					assign(f, out) << L"textureLod(" << samplerName << L", " << texCoord->cast(GtFloat4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5), 0.0);" << Endl;
					break;

				default:
					return false;
				}
			}
	#else
			switch (texture->getType())
			{
			case GtTexture2D:
				assign(f, out) << L"texture2D(" << samplerName << L", " << texCoord->cast(GtFloat3) << L" * vec3(1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.5));" << Endl;
				break;

			case GtTexture3D:
				assign(f, out) << L"texture3D(" << samplerName << L", " << texCoord->cast(GtFloat4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5));" << Endl;
				break;

			case GtTextureCube:
				assign(f, out) << L"textureCube(" << samplerName << L", " << texCoord->cast(GtFloat4) << L" * vec3(1.0, 1.0, 1.0, 0.5) + vec3(0.0, 0.0, 0.0, 0.5));" << Endl;
				break;

			default:
				return false;
			}
	#endif
		}
	}

	if (cx.inVertex())
	{
#if defined(T_OPENGL_STD)
		switch (texture->getType())
		{
		case GtTexture2D:
			assign(f, out) << L"texture2DBilinear(" << samplerName << L", " << texCoord->cast(GtFloat2) << L");" << Endl;
			break;

		case GtTexture3D:
		case GtTextureCube:
			assign(f, out) << L"texture(" << samplerName << L", " << texCoord->cast(GtFloat3) << L", 0.0);" << Endl;
			break;

        default:
            return false;
		}
#else
		switch (texture->getType())
		{
		case GtTexture2D:
			assign(f, out) << L"texture2D(" << samplerName << L", " << texCoord->cast(GtFloat2) << L");" << Endl;
			break;

		case GtTexture3D:
			assign(f, out) << L"texture3D(" << samplerName << L", " << texCoord->cast(GtFloat3) << L");" << Endl;
			break;

		case GtTextureCube:
			assign(f, out) << L"textureCube(" << samplerName << L", " << texCoord->cast(GtFloat3) << L");" << Endl;
			break;

		default:
			return false;
		}
#endif
	}

	return true;
}

bool emitScalar(GlslContext& cx, Scalar* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	f << L"const float " << out->getName() << L" = " << formatFloat(node->get()) << L";" << Endl;
	return true;
}

bool emitSign(GlslContext& cx, Sign* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sign(" << in->getName() << L");" << Endl;
	return true;
}

bool emitScript(GlslContext& cx, Script* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Get platform specific script from node.
	std::wstring script = node->getScript();
	if (script.empty())
		return false;

	// Emit input and outputs.
	int32_t inputPinCount = node->getInputPinCount();
	int32_t outputPinCount = node->getOutputPinCount();

	const std::map< std::wstring, SamplerState >& samplers = node->getSamplers();

	RefArray< GlslVariable > in(inputPinCount);
	RefArray< GlslVariable > out(outputPinCount);

	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const TypedOutputPin* outputPin = static_cast< const TypedOutputPin* >(node->getOutputPin(i));
		T_ASSERT (outputPin);

		out[i] = cx.emitOutput(
			node,
			outputPin->getName(),
			glsl_from_parameter_type(outputPin->getType())
		);
	}

	for (int32_t i = 0; i < inputPinCount; ++i)
	{
		in[i] = cx.emitInput(node->getInputPin(i));
		if (!in[i])
			return false;

		if (node->getInputPinType(i) >= PtTexture2D)
		{
			std::wstring samplerId = node->getInputPinSamplerId(i);
			if (samplerId.empty())
				return false;

			std::map< std::wstring, SamplerState >::const_iterator it = samplers.find(samplerId);
			if (it == samplers.end())
				return false;

			const SamplerState& samplerState = it->second;

			const GLenum c_glFilter[] =
			{
				GL_NEAREST,
				GL_LINEAR
			};

			const GLenum c_glWrap[] =
			{
				GL_REPEAT,
				GL_REPEAT,
				GL_CLAMP_TO_EDGE,
				GL_CLAMP_TO_EDGE
			};

			const GLenum c_glCompare[] =
			{
				GL_INVALID_ENUM,
				GL_ALWAYS,
				GL_NEVER,
				GL_LESS,
				GL_LEQUAL,
				GL_GREATER,
				GL_GEQUAL,
				GL_EQUAL,
				GL_NOTEQUAL
			};

			Adler32 samplerHash;
			samplerHash.feed(node->getInputPin(i)->getName());
			samplerHash.feed(samplerState.minFilter);
			samplerHash.feed(samplerState.mipFilter);
			samplerHash.feed(samplerState.magFilter);
			samplerHash.feed(samplerState.addressU);
			samplerHash.feed(samplerState.addressV);
			samplerHash.feed(samplerState.addressW);
			samplerHash.feed(samplerState.compare);

			// Use same stage index for both vertex and fragment shader.
			// Sampler name is defined by which stage it's associated with.
			int32_t stage;

			// Define sampler.
			bool defineStates = cx.defineSampler(samplerId, samplerHash.get(), GL_TEXTURE_2D, node->getInputPin(i)->getName(), stage);
			if (defineStates)
			{
				RenderStateOpenGL& rs = cx.getRenderState();

				bool minLinear = samplerState.minFilter != FtPoint;
				bool mipLinear = samplerState.mipFilter != FtPoint;

				if (!minLinear && !mipLinear)
					rs.samplerStates[stage].minFilter = GL_NEAREST;
				else if (!minLinear && mipLinear)
					rs.samplerStates[stage].minFilter = GL_NEAREST_MIPMAP_LINEAR;
				else if (minLinear && !mipLinear)
					rs.samplerStates[stage].minFilter = GL_LINEAR_MIPMAP_NEAREST;
				else
					rs.samplerStates[stage].minFilter = GL_LINEAR_MIPMAP_LINEAR;

				rs.samplerStates[stage].magFilter = c_glFilter[samplerState.magFilter];
				rs.samplerStates[stage].wrapS = c_glWrap[samplerState.addressU];
				rs.samplerStates[stage].wrapT = c_glWrap[samplerState.addressV];
				rs.samplerStates[stage].wrapR = c_glWrap[samplerState.addressW];
				rs.samplerStates[stage].compare = c_glCompare[samplerState.compare];
			}

			if (cx.getShader().getUniforms().find(samplerId) == cx.getShader().getUniforms().end())
			{
				StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
				if (samplerState.compare == CfNone)
				{
#if defined(T_OPENGL_STD)
					fu << L"uniform sampler2D " << samplerId << L";" << Endl;
#elif defined(T_OPENGL_ES2)
					fu << L"uniform lowp sampler2D " << samplerId << L";" << Endl;
#endif
				}
				else
				{
					if (!cx.inFragment())
						return false;

	#if defined(T_OPENGL_STD)
					fu << L"uniform sampler2DShadow " << samplerId << L";" << Endl;
	#elif defined(T_OPENGL_ES2)
					fu << L"uniform lowp sampler2DShadow " << samplerId << L";" << Endl;
	#endif
				}
				cx.getShader().addUniform(samplerId);
			}
		}
	}

	// Define script instance.
	if (cx.getShader().defineScript(node->getName()))
	{
		StringOutputStream& fs = cx.getShader().getOutputStream(GlslShader::BtScript);

		fs << L"void " << node->getName() << L"(";

		int32_t ii = 0;
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			if (in[i]->getType() >= GtTexture2D)
				continue;

			if (ii++ > 0)
				fs << L", ";

			fs << glsl_type_name(in[i]->getType()) << L" " << node->getInputPin(i)->getName();
		}

		if (!in.empty())
			fs << L", ";

		for (int32_t i = 0; i < outputPinCount; ++i)
		{
			if (i > 0)
				fs << L", ";
			fs << L"out " << glsl_type_name(out[i]->getType()) << L" " << node->getOutputPin(i)->getName();
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
		f << glsl_type_name(out[i]->getType()) << L" " << out[i]->getName() << L";" << Endl;

	f << node->getName() << L"(";

	int32_t ii = 0;
	for (RefArray< GlslVariable >::const_iterator i = in.begin(); i != in.end(); ++i)
	{
		if ((*i)->getType() >= GtTexture2D)
			continue;

		if (ii++ > 0)
			f << L", ";

		f << (*i)->getName();
	}

	if (ii > 0)
		f << L", ";

	for (RefArray< GlslVariable >::const_iterator i = out.begin(); i != out.end(); ++i)
	{
		if (i != out.begin())
			f << L", ";
		f << (*i)->getName();
	}

	f << L");" << Endl;
	return true;
}

bool emitSin(GlslContext& cx, Sin* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"sin(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitSqrt(GlslContext& cx, Sqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sqrt(" << in->getName() << L");" << Endl;
	return true;
}

bool emitStep(GlslContext& cx, Step* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"X");
	GlslVariable* in2 = cx.emitInput(node, L"Y");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"step(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
	return true;
}

bool emitSub(GlslContext& cx, Sub* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	if (!in1 || !in2)
		return false;
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;
	return true;
}

bool emitSum(GlslContext& cx, Sum* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	std::wstring inputName;

	// Create iterator variable.
	GlslVariable* N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	GlslVariable* out = cx.emitOutput(node, L"Output", GtVoid);
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
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	{
		GlslVariable* input = cx.emitInput(node, L"Input");
		if (!input)
			return false;

		inputName = input->getName();

		// Modify output variable; need to have input variable ready as it
		// will determine output type.
		out->setType(input->getType());
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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	std::wstring map = toLower(node->get());
	if (map.length() == 0)
		return false;

	const GlslType types[] = { GtFloat, GtFloat2, GtFloat3, GtFloat4 };
	GlslType type = types[map.length() - 1];

	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	if (
		(map == L"xyzw" && in->getType() == GtFloat4) ||
		(map == L"xyz" && in->getType() == GtFloat3) ||
		(map == L"xy" && in->getType() == GtFloat2) ||
		(map == L"x" && in->getType() == GtFloat)
	)
	{
		// No need to swizzle; pass variable further.
		cx.emitOutput(node, L"Output", in);
	}
	else
	{
		GlslVariable* out = cx.emitOutput(node, L"Output", type);

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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	GlslVariable* in = cx.emitInput(node, L"Select");
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
		T_ASSERT (caseInput);

		GlslVariable* caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

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
		T_ASSERT (caseInput);

		GlslVariable* caseInputVariable = cx.emitInput(caseInput);
		T_ASSERT (caseInputVariable);

		caseBranches.push_back(fs.str());
		caseInputs.push_back(*caseInputVariable);
		outputType = std::max(outputType, caseInputVariable->getType());

		cx.getShader().popScope();
		cx.getShader().popOutputStream(GlslShader::BtBody);
	}

	GlslVariable* out = cx.emitOutput(node, L"Output", outputType);
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
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	if (!theta || theta->getType() != GtFloat)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"tan(" << theta->getName() << L");" << Endl;
	return true;
}

bool emitTargetSize(GlslContext& cx, TargetSize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat2);
#if defined(T_OPENGL_ES2)
	assign(f, out) << L"_gl_targetSize.xy;" << Endl;
#else
	assign(f, out) << L"_gl_targetSize;" << Endl;
#endif
	return true;
}

bool emitTextureSize(GlslContext& cx, TextureSize* node)
{
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in || in->getType() < GtTexture2D)
		return false;

	std::wstring uniformName = L"_gl_textureSize_" + in->getName();

	GlslVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		uniformName,
		GtFloat4
	);
    if (!out)
        return false;

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(uniformName) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
		fu << L"uniform vec4 " << uniformName << L";" << Endl;
		cx.getShader().addUniform(uniformName);
	}

	return true;
}

bool emitTransform(GlslContext& cx, Transform* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* transform = cx.emitInput(node, L"Transform");
	if (!in || !transform)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);
	assign(f, out) << transform->getName() << L" * " << in->cast(GtFloat4) << L";" << Endl;
	return true;
}

bool emitTranspose(GlslContext& cx, Transpose* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"transpose(" << in->getName() << L");" << Endl;
	cx.setRequireTranspose();
	return true;
}

bool emitTruncate(GlslContext& cx, Truncate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << in->getName() << L" - fract(" << in->getName() << L");" << Endl;
	return true;
}

bool emitUniform(GlslContext& cx, Uniform* node)
{
	GlslVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		glsl_from_parameter_type(node->getParameterType())
	);

	if (out->getType() < GtTexture2D)
	{
		const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
		if (uniforms.find(node->getParameterName()) == uniforms.end())
		{
			StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
			fu << L"uniform " << glsl_type_name(out->getType()) << L" " << node->getParameterName() << L";" << Endl;
			cx.getShader().addUniform(node->getParameterName());
		}

		switch (out->getType())
		{
		case GtFloat:
			if (!cx.defineUniform(node->getParameterName(), GL_FLOAT, 1))
				return false;
			break;

		case GtFloat2:
		case GtFloat3:
		case GtFloat4:
			if (!cx.defineUniform(node->getParameterName(), GL_FLOAT_VEC4, 1))
				return false;
			break;

		case GtFloat4x4:
			if (!cx.defineUniform(node->getParameterName(), GL_FLOAT_MAT4, 1))
				return false;
			break;

		default:
			return false;
		}
	}
	else
		cx.defineTexture(node->getParameterName());

	return true;
}

bool emitVector(GlslContext& cx, Vector* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);
	assign(f, out) << L"vec4(" << formatFloat(node->get().x()) << L", " << formatFloat(node->get().y()) << L", " << formatFloat(node->get().z()) << L", " << formatFloat(node->get().w()) << L");" << Endl;
	return true;
}

bool emitVertexInput(GlslContext& cx, VertexInput* node)
{
	if (!cx.inVertex())
		return false;

	GlslVariable* out = cx.getShader().getInputVariable(node->getName());
	if (!out)
	{
		GlslType type = glsl_from_data_type(node->getDataType());
		std::wstring attributeName = glsl_vertex_attr_name(node->getDataUsage(), node->getIndex());

		StringOutputStream& fi = cx.getVertexShader().getOutputStream(GlslShader::BtInput);
#if !defined(T_OPENGL_ES2)
		fi << L"in " << glsl_type_name(type) << L" " << attributeName << L";" << Endl;
#else
		fi << L"attribute " << glsl_type_name(type) << L" " << attributeName << L";" << Endl;
#endif

		if (node->getDataUsage() == DuPosition && type != GtFloat4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin(L"Output"),
				GtFloat4
			);
			StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
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
			StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
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
			StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
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

	GlslVariable* in = cx.emitInput(node, L"Input");
	if (!in)
		return false;

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
	switch (in->getType())
	{
	case GtFloat:
		fb << L"gl_Position = PV(vec4(" << in->getName() << L", 0.0, 0.0, 1.0));" << Endl;
		break;

	case GtFloat2:
		fb << L"gl_Position = PV(vec4(" << in->getName() << L".xy, 0.0, 1.0));" << Endl;
		break;

	case GtFloat3:
		fb << L"gl_Position = PV(vec4(" << in->getName() << L".xyz, 1.0));" << Endl;
		break;

	case GtFloat4:
		fb << L"gl_Position = PV(" << in->getName() << L");" << Endl;
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

	virtual bool emit(GlslContext& c, Node* node)
	{
		T_ASSERT (is_a< NodeType >(node));
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

GlslEmitter::~GlslEmitter()
{
	for (std::map< const TypeInfo*, Emitter* >::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
		delete i->second;
}

bool GlslEmitter::emit(GlslContext& c, Node* node)
{
	// Find emitter for node.
	std::map< const TypeInfo*, Emitter* >::iterator i = m_emitters.find(&type_of(node));
	if (i == m_emitters.end())
	{
		log::error << L"No emitter for node " << type_name(node) << Endl;
		return false;
	}

	// Emit GLSL code.
	T_ASSERT (i->second);
	return i->second->emit(c, node);
}

	}
}
