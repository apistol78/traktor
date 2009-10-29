#include <iomanip>
#include "Render/OpenGL/GlslEmitter.h"
#include "Render/OpenGL/GlslContext.h"
#include "Render/VertexElement.h"
#include "Render/Nodes.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
std::wstring formatFloat(float v)
{
	std::wstringstream ss;
	ss << std::setw(1) << std::setprecision(8) << std::fixed << v;

	std::wstring s = ss.str();
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
	}
	return vs;
}

StringOutputStream& assign(StringOutputStream& f, GlslVariable* out)
{
#if defined(T_OPENGL_STD)
	f << glsl_type_name(out->getType()) << L" " << out->getName() << L" = ";
#elif defined(T_OPENGL_ES2)
	f << glsl_type_name(out->getType()) << L" " << out->getName() << L"; " << out->getName() << L" = ";
#endif
	return f;
}

void emitAbs(GlslContext& cx, Abs* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"abs(" << in->getName() << L");" << Endl;
}

void emitAdd(GlslContext& cx, Add* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" + " << in2->cast(type) << L";" << Endl;
}

void emitArcusCos(GlslContext& cx, ArcusCos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	T_ASSERT_M (theta->getType() == GtFloat, L"Incorrect type on Theta");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"acos(" << theta->getName() << L");" << Endl;
}

void emitArcusTan(GlslContext& cx, ArcusTan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* xy = cx.emitInput(node, L"XY");
	T_ASSERT_M (xy->getType() == GtFloat2, L"Incorrect type on XY");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"atan2(" << xy->getName() << L".x, " << xy->getName() << L".y);" << Endl;
}

void emitClamp(GlslContext& cx, Clamp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"clamp(" << in->getName() << L", " << formatFloat(node->getMin()) << L", " << formatFloat(node->getMax()) << L");" << Endl;
}

void emitColor(GlslContext& cx, Color* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);
	traktor::Color color = node->getColor();
	f << L"const vec4 " << out->getName() << L" = vec4(" << (color.r / 255.0f) << L", " << (color.g / 255.0f) << L", " << (color.b / 255.0f) << L", " << (color.a / 255.0f) << L");" << Endl;
}

void emitConditional(GlslContext& cx, Conditional* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	
	// Emit input and reference branches.
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* ref = cx.emitInput(node, L"Reference");
	if (!in || !ref)
		return;

	GlslVariable caseTrue, caseFalse;
	std::wstring caseTrueBranch, caseFalseBranch;

	// Emit true branch.
	{
		StringOutputStream fs;

		cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
		cx.getShader().pushScope();

		GlslVariable* ct = cx.emitInput(node, L"CaseTrue");
		T_ASSERT (ct);

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
		T_ASSERT (cf);

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
}

void emitCos(GlslContext& cx, Cos* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	T_ASSERT_M (theta->getType() == GtFloat, L"Incorrect type on Theta");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"cos(" << theta->getName() << L");" << Endl;
}

void emitCross(GlslContext& cx, Cross* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat3);
	assign(f, out) << L"cross(" << in1->cast(GtFloat3) << L", " << in2->cast(GtFloat3) << L");" << Endl;
}

void emitDerivative(GlslContext& cx, Derivative* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* input = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", input->getType());
#if defined(T_OPENGL_STD)
	switch (node->getAxis())
	{
	case Derivative::DaX:
		assign(f, out) << L"dFdx(" << input->getName() << L");" << Endl;
		break;
	case Derivative::DaY:
		assign(f, out) << L"dFdy(" << input->getName() << L");" << Endl;
		break;
	}
#elif defined(T_OPENGL_ES2)
	switch (input->getType())
	{
	case GtFloat:
		assign(f, out) << L"0.0;" << Endl;
		break;
	case GtFloat2:
		assign(f, out) << L"vec2(0.0, 0.0);" << Endl;
		break;
	case GtFloat3:
		assign(f, out) << L"vec3(0.0, 0.0, 0.0);" << Endl;
		break;
	case GtFloat4:
		assign(f, out) << L"vec4(0.0, 0.0, 0.0, 0.0);" << Endl;
		break;
	}
#endif
}

void emitDiv(GlslContext& cx, Div* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" / " << in2->cast(type) << L";" << Endl;
}

void emitDot(GlslContext& cx, Dot* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	assign(f, out) << L"dot(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
}

void emitExp(GlslContext& cx, Exp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"exp(" << in->getName() << L");" << Endl;
}

void emitFraction(GlslContext& cx, Fraction* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"fract(" << in->getName() << L");" << Endl;
}

void emitFragmentPosition(GlslContext& cx, FragmentPosition* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat2);
	assign(f, out) << L"gl_FragCoord.xy;" << Endl;
}

void emitIndexedUniform(GlslContext& cx, IndexedUniform* node)
{
	const GlslType c_parameterType[] = { GtFloat, GtFloat4, GtFloat4x4 };

	GlslVariable* index = cx.emitInput(node, L"Index");
	GlslVariable* out = cx.getShader().createTemporaryVariable(
		node->findOutputPin(L"Output"),
		c_parameterType[node->getParameterType()]
	);

	StringOutputStream& fb = cx.getShader().getOutputStream(GlslShader::BtBody);
	assign(fb, out) << node->getParameterName() << L"[int(" << index->getName() << L")];" << Endl;

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
		fu << L"uniform " << glsl_type_name(out->getType()) << L" " << node->getParameterName() << L"[" << node->getLength() << L"];" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}
}

void emitInterpolator(GlslContext& cx, Interpolator* node)
{
	if (!cx.inFragment())
	{
		// We're already in vertex state; skip interpolation.
		GlslVariable* in = cx.emitInput(node, L"Input");
		T_ASSERT (in);

		GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());

		StringOutputStream& fb = cx.getShader().getOutputStream(GlslShader::BtBody);
		assign(fb, out) << in->getName() << L";" << Endl;

		return;
	}

	cx.enterVertex();

	GlslVariable* in = cx.emitInput(node, L"Input");
	T_ASSERT (in);

	cx.enterFragment();

	int interpolatorId = cx.getShader().allocateInterpolator();
	std::wstring interpolator = L"Attr" + toString(interpolatorId);

	StringOutputStream& fo = cx.getVertexShader().getOutputStream(GlslShader::BtOutput);
	fo << L"varying " << glsl_type_name(in->getType()) << L" " << interpolator << L";" << Endl;

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
	fb << interpolator << L" = " << in->getName() << L";" << Endl;

	cx.getFragmentShader().createOuterVariable(
		node->findOutputPin(L"Output"),
		interpolator,
		in->getType()
	);

	StringOutputStream& fpi = cx.getFragmentShader().getOutputStream(GlslShader::BtInput);
	fpi << L"varying " << glsl_type_name(in->getType()) << L" " << interpolator << L";" << Endl;
}

void emitIterate(GlslContext& cx, Iterate* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Create iterator variable.
	GlslVariable* N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	GlslVariable* out = cx.emitOutput(node, L"Output", GtVoid);
	T_ASSERT (out);

	// Write input branch in a temporary output stream.
	StringOutputStream fs;
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	GlslVariable* input = cx.emitInput(node, L"Input");
	T_ASSERT (input);

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	out->setType(input->getType());

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
	f << out->getName() << L" = " << input->getName() << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;	

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);
}

void emitLength(GlslContext& cx, Length* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"length(" << in->getName() << L");" << Endl;
}

void emitLerp(GlslContext& cx, Lerp* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* blend = cx.emitInput(node, L"Blend");
	T_ASSERT_M (blend->getType() == GtFloat, L"Incorrect type on Blend");
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"mix(" << in1->cast(type) << L", " << in2->cast(type) << L", " << blend->getName() << L");" << Endl;
}

void emitLog(GlslContext& cx, Log* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
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
}

void emitMatrix(GlslContext& cx, Matrix* node)
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
}

void emitMax(GlslContext& cx, Max* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"max(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
}

void emitMin(GlslContext& cx, Min* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"min(" << in1->cast(type) << L", " << in2->cast(type) << L");" << Endl;
}

void emitMixIn(GlslContext& cx, MixIn* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* x = cx.emitInput(node, L"X");
	GlslVariable* y = cx.emitInput(node, L"Y");
	GlslVariable* z = cx.emitInput(node, L"Z");
	GlslVariable* w = cx.emitInput(node, L"W");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);
	assign(f, out) << L"vec4(" << (x ? x->getName() : L"0.0") << L", " << (y ? y->getName() : L"0.0") << L", " << (z ? z->getName() : L"0.0") << L", " << (w ? w->getName() : L"0.0") << L");" << Endl;
}

void emitMixOut(GlslContext& cx, MixOut* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	switch (in->getType())
	{
	case GtFloat:
		{
			GlslVariable* x = cx.emitOutput(node, L"X", GtFloat);
			assign(f, x) << in->getName() << L".x;" << Endl;
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
	}
}

void emitMul(GlslContext& cx, Mul* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L";" << Endl;
}

void emitMulAdd(GlslContext& cx, MulAdd* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslVariable* in3 = cx.emitInput(node, L"Input3");
	GlslType type = std::max< GlslType >(std::max< GlslType >(in1->getType(), in2->getType()), in3->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" * " << in2->cast(type) << L" + " << in3->cast(type) << L";" << Endl;
}

void emitNeg(GlslContext& cx, Neg* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"-" << in->getName() << L";" << Endl;
}

void emitNormalize(GlslContext& cx, Normalize* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"normalize(" << in->getName() << L");" << Endl;
}

void emitPixelOutput(GlslContext& cx, PixelOutput* node)
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

	cx.enterFragment();

	GlslVariable* in = cx.emitInput(node, L"Input");

	StringOutputStream& fpb = cx.getFragmentShader().getOutputStream(GlslShader::BtBody);
	fpb << L"gl_FragColor = " << in->cast(GtFloat4) << L";" << Endl;

	uint32_t colorMask =
		((node->getColorWriteMask() & PixelOutput::CwRed) ? RenderState::CmRed : 0) |
		((node->getColorWriteMask() & PixelOutput::CwGreen) ? RenderState::CmGreen : 0) |
		((node->getColorWriteMask() & PixelOutput::CwBlue) ? RenderState::CmBlue : 0) |
		((node->getColorWriteMask() & PixelOutput::CwAlpha) ? RenderState::CmAlpha : 0);

	RenderState& rs = cx.getRenderState();
	rs.cullFaceEnable = node->getCullMode() == PixelOutput::CmNever ? GL_FALSE : GL_TRUE;
	rs.cullFace = c_oglCullFace[node->getCullMode()];
	rs.blendEnable = node->getBlendEnable() ? GL_TRUE : GL_FALSE;
	rs.blendEquation = c_oglBlendEquation[node->getBlendOperation()];
	rs.blendFuncSrc = c_oglBlendFunction[node->getBlendSource()];
	rs.blendFuncDest = c_oglBlendFunction[node->getBlendDestination()];
	rs.depthTestEnable = node->getDepthEnable() ? GL_TRUE : GL_FALSE;
	rs.colorMask = colorMask;
	rs.depthMask = node->getDepthWriteEnable() ? GL_TRUE : GL_FALSE;
	rs.depthFunc = c_oglFunction[node->getDepthFunction()];
	rs.alphaTestEnable = node->getAlphaTestEnable() ? GL_TRUE : GL_FALSE;
	rs.alphaFunc = c_oglFunction[node->getAlphaTestFunction()];
	rs.alphaRef = GLclampf(node->getAlphaTestReference() / 255.0f);
}

void emitPlatform(GlslContext& cx, Platform* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

#if defined(T_OPENGL_STD)
	GlslVariable* input = cx.emitInput(node, L"OpenGL");
#elif defined(T_OPENGL_ES2)
	GlslVariable* input = cx.emitInput(node, L"OpenGL ES2");
#endif
	if (!input)
		input = cx.emitInput(node, L"Other");

	T_ASSERT (input);

	GlslVariable* out = cx.emitOutput(node, L"Output", input->getType());
	assign(f, out) << input->getName() << L";" << Endl;
}

void emitPolynomial(GlslContext& cx, Polynomial* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	GlslVariable* x = cx.emitInput(node, L"X");
	GlslVariable* coeffs = cx.emitInput(node, L"Coefficients");
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
	}
	f << L";" << Endl;
}

void emitPow(GlslContext& cx, Pow* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* exponent = cx.emitInput(node, L"Exponent");
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslType type = std::max< GlslType >(exponent->getType(), in->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << L"pow(max(" << in->cast(type) << L", 0.0), " << exponent->cast(type) << L");" << Endl;
}

void emitReflect(GlslContext& cx, Reflect* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* normal = cx.emitInput(node, L"Normal");
	GlslVariable* direction = cx.emitInput(node, L"Direction");
	GlslVariable* out = cx.emitOutput(node, L"Output", direction->getType());
	assign(f, out) << L"reflect(" << direction->getName() << L", " << normal->cast(direction->getType()) << L");" << Endl;
}

void emitSampler(GlslContext& cx, Sampler* node)
{
	const GLenum c_glFilter[] =
	{
		GL_NEAREST,
		GL_LINEAR,
		GL_LINEAR
	};

	const GLenum c_glWrap[] =
	{
		GL_REPEAT,
		GL_REPEAT,	// AdMirror
#if defined(T_OPENGL_STD)
		GL_CLAMP,
		GL_CLAMP	// AdBorder
#elif defined(T_OPENGL_ES2)
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_EDGE
#endif
	};

	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* texCoord = cx.emitInput(node, L"TexCoord");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);

	// Transform texture coordinate by "origin & scale" uniform.
	std::wstring originScale = L"t_internal_" + node->getParameterName() + L"_OriginScale";
	std::wstring samplerTexCoord = out->getName() + L"_texCoord";
	f << L"vec2 " << samplerTexCoord << L" = " << texCoord->cast(GtFloat2) << L" * " << originScale << L".zw + " << originScale << L".xy;" << Endl;

	if (cx.inFragment())
	{
		switch (node->getLookup())
		{
		case Sampler::LuSimple:
			assign(f, out) << L"texture2D(" << node->getParameterName() << L", " << samplerTexCoord << L");" << Endl;
			break;

		case Sampler::LuCube:
			assign(f, out) << L"textureCube(" << node->getParameterName() << L", vec3(" << samplerTexCoord << L".xy, 0.0));" << Endl;
			break;

		case Sampler::LuVolume:
			assign(f, out) << L"texture3D(" << node->getParameterName() << L", vec3(" << samplerTexCoord << L".xy, 0.0));" << Endl;
			break;
		}
	}
	if (cx.inVertex())
	{
		switch (node->getLookup())
		{
		case Sampler::LuSimple:
			assign(f, out) << L"texture2DLod(" << node->getParameterName() << L", " << samplerTexCoord << L", 0.0);" << Endl;
			break;

		case Sampler::LuCube:
			assign(f, out) << L"textureCubeLod(" << node->getParameterName() << L", vec3(" << samplerTexCoord << L".xy, 0.0));" << Endl;
			break;

		case Sampler::LuVolume:
			assign(f, out) << L"texture3DLod(" << node->getParameterName() << L", vec3(" << samplerTexCoord << L".xy, 0.0));" << Endl;
			break;
		}
	}

	const std::set< std::wstring >& samplers = cx.getShader().getSamplers();
	if (samplers.find(node->getParameterName()) == samplers.end())
	{
		int sampler = int(samplers.size());

		StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
		switch (node->getLookup())
		{
		case Sampler::LuSimple:
			fu << L"uniform sampler2D " << node->getParameterName() << L";" << Endl;
			break;

		case Sampler::LuCube:
			fu << L"uniform samplerCube " << node->getParameterName() << L";" << Endl;
			break;

		case Sampler::LuVolume:
			fu << L"uniform sampler3D " << node->getParameterName() << L";" << Endl;
			break;
		}
		fu << L"uniform vec4 " << originScale << L";" << Endl;

		RenderState& rs = cx.getRenderState();

		if (cx.inFragment())
		{
			bool minLinear = node->getMinFilter() != Sampler::FtPoint;
			bool mipLinear = node->getMipFilter() != Sampler::FtPoint;

			if (!minLinear && !mipLinear)
				rs.samplerStates[sampler].minFilter = GL_NEAREST;
			else if (!minLinear && mipLinear)
				rs.samplerStates[sampler].minFilter = GL_NEAREST_MIPMAP_LINEAR;
			else if (minLinear && !mipLinear)
				rs.samplerStates[sampler].minFilter = GL_LINEAR_MIPMAP_NEAREST;
			else
				rs.samplerStates[sampler].minFilter = GL_LINEAR_MIPMAP_LINEAR;

			rs.samplerStates[sampler].magFilter = c_glFilter[node->getMagFilter()];
			rs.samplerStates[sampler].wrapS = c_glWrap[node->getAddressU()];
			rs.samplerStates[sampler].wrapT = c_glWrap[node->getAddressV()];
		}
		else
		{
			rs.samplerStates[sampler].minFilter = GL_NEAREST;
			rs.samplerStates[sampler].magFilter = GL_NEAREST;
			rs.samplerStates[sampler].wrapS = GL_REPEAT;
			rs.samplerStates[sampler].wrapT = GL_REPEAT;
		}

		cx.getShader().addSampler(node->getParameterName());
	}
}

void emitScalar(GlslContext& cx, Scalar* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	f << L"const float " << out->getName() << L" = " << formatFloat(node->get()) << L";" << Endl;
}

void emitSin(GlslContext& cx, Sin* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	T_ASSERT_M (theta->getType() == GtFloat, L"Incorrect type on Theta");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"sin(" << theta->getName() << L");" << Endl;
}

void emitSqrt(GlslContext& cx, Sqrt* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << L"sqrt(" << in->getName() << L");" << Endl;
}

void emitSub(GlslContext& cx, Sub* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in1 = cx.emitInput(node, L"Input1");
	GlslVariable* in2 = cx.emitInput(node, L"Input2");
	GlslType type = std::max< GlslType >(in1->getType(), in2->getType());
	GlslVariable* out = cx.emitOutput(node, L"Output", type);
	assign(f, out) << in1->cast(type) << L" - " << in2->cast(type) << L";" << Endl;
}

void emitSum(GlslContext& cx, Sum* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	// Create iterator variable.
	GlslVariable* N = cx.emitOutput(node, L"N", GtFloat);
	T_ASSERT (N);

	// Create void output variable; change type later when we know
	// the type of the input branch.
	GlslVariable* out = cx.emitOutput(node, L"Output", GtVoid);
	T_ASSERT (out);

	// Write input branch in a temporary output stream.
	StringOutputStream fs;
	cx.getShader().pushOutputStream(GlslShader::BtBody, &fs);
	cx.getShader().pushScope();

	GlslVariable* input = cx.emitInput(node, L"Input");
	T_ASSERT (input);

	// Modify output variable; need to have input variable ready as it
	// will determine output type.
	out->setType(input->getType());

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
	f << out->getName() << L" += " << input->getName() << L";" << Endl;

	f << DecreaseIndent;
	f << L"}" << Endl;	

	cx.getShader().popScope();
	cx.getShader().popOutputStream(GlslShader::BtBody);
}

void emitSwizzle(GlslContext& cx, Swizzle* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	std::wstring map = node->get();
	T_ASSERT (map.length() > 0);

	const GlslType types[] = { GtFloat, GtFloat2, GtFloat3, GtFloat4 };
	GlslType type = types[map.length() - 1];

	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", type);

	std::wstringstream ss;
	ss << glsl_type_name(type) << L"(";
	for (size_t i = 0; i < map.length(); ++i)
	{
		if (i > 0)
			ss << L", ";
		switch (tolower(map[i]))
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
			ss << in->getName() << L'.' << char(tolower(map[i]));
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

	assign(f, out) << ss.str() << L";" << Endl;
}

void emitSwitch(GlslContext& cx, Switch* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);

	GlslVariable* in = cx.emitInput(node, L"Select");
	T_ASSERT (in);

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
}

void emitTan(GlslContext& cx, Tan* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* theta = cx.emitInput(node, L"Theta");
	T_ASSERT_M (theta->getType() == GtFloat, L"Incorrect type on Theta");
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat);
	assign(f, out) << L"tan(" << theta->getName() << L");" << Endl;
}

void emitTransform(GlslContext& cx, Transform* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* transform = cx.emitInput(node, L"Transform");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
	assign(f, out) << transform->getName() << L" * " << in->cast(GtFloat4) << L";" << Endl;
}

void emitTranspose(GlslContext& cx, Transpose* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* in = cx.emitInput(node, L"Input");
	GlslVariable* out = cx.emitOutput(node, L"Output", in->getType());
//#if defined(T_OPENGL_STD)
//	assign(f, out) << L"transpose(" << in->getName() << L");" << Endl;
//#elif defined(T_OPENGL_ES2)
	assign(f, out) << in->getName() << L";" << Endl;
//#endif
}

void emitUniform(GlslContext& cx, Uniform* node)
{
	const GlslType c_parameterType[] = { GtFloat, GtFloat4, GtFloat4x4 };
	GlslVariable* out = cx.getShader().createVariable(
		node->findOutputPin(L"Output"),
		node->getParameterName(),
		c_parameterType[node->getParameterType()]
	);

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getOutputStream(GlslShader::BtUniform);
		fu << L"uniform " << glsl_type_name(out->getType()) << L" " << node->getParameterName() << L";" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}
}

void emitVector(GlslContext& cx, Vector* node)
{
	StringOutputStream& f = cx.getShader().getOutputStream(GlslShader::BtBody);
	GlslVariable* out = cx.emitOutput(node, L"Output", GtFloat4);
	assign(f, out) << L"vec4(" << formatFloat(node->get().x()) << L", " << formatFloat(node->get().y()) << L", " << formatFloat(node->get().z()) << L", " << formatFloat(node->get().w()) << L");" << Endl;
}

void emitVertexInput(GlslContext& cx, VertexInput* node)
{
	T_ASSERT (cx.inVertex());
	GlslVariable* out = cx.getShader().getInputVariable(node->getName());
	if (!out)
	{
		GlslType type = glsl_from_data_type(node->getDataType());
		std::wstring attributeName = glsl_vertex_attr_name(node->getDataUsage(), node->getIndex());

		StringOutputStream& fi = cx.getVertexShader().getOutputStream(GlslShader::BtInput);
		fi << L"attribute " << glsl_type_name(type) << L" " << attributeName << L";" << Endl;

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
}

void emitVertexOutput(GlslContext& cx, VertexOutput* node)
{
	cx.enterVertex();
	GlslVariable* in = cx.emitInput(node, L"Input");

	StringOutputStream& fb = cx.getVertexShader().getOutputStream(GlslShader::BtBody);
#if defined(T_OPENGL_ES2)
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
	}
#else
	switch (in->getType())
	{
	case GtFloat:
		fb << L"gl_Position = vec4(" << in->getName() << L", 0.0, 0.0, 1.0);" << Endl;
		break;

	case GtFloat2:
		fb << L"gl_Position = vec4(" << in->getName() << L".xy, 0.0, 1.0);" << Endl;
		break;

	case GtFloat3:
		fb << L"gl_Position = vec4(" << in->getName() << L".xyz, 1.0);" << Endl;
		break;

	case GtFloat4:
		fb << L"gl_Position = " << in->getName() << L";" << Endl;
		break;
	}
#endif
}

		}

struct Emitter
{
	virtual void emit(GlslContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public Emitter
{
	typedef void (*function_t)(GlslContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function) :
		m_function(function)
	{
	}

	virtual void emit(GlslContext& c, Node* node)
	{
		T_ASSERT (is_a< NodeType >(node));
		(*m_function)(c, static_cast< NodeType* >(node));
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
	m_emitters[&type_of< Matrix >()] = new EmitterCast< Matrix >(emitMatrix);
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
	m_emitters[&type_of< Platform >()] = new EmitterCast< Platform >(emitPlatform);
	m_emitters[&type_of< Reflect >()] = new EmitterCast< Reflect >(emitReflect);
	m_emitters[&type_of< Sampler >()] = new EmitterCast< Sampler >(emitSampler);
	m_emitters[&type_of< Scalar >()] = new EmitterCast< Scalar >(emitScalar);
	m_emitters[&type_of< Sin >()] = new EmitterCast< Sin >(emitSin);
	m_emitters[&type_of< Sqrt >()] = new EmitterCast< Sqrt >(emitSqrt);
	m_emitters[&type_of< Sub >()] = new EmitterCast< Sub >(emitSub);
	m_emitters[&type_of< Sum >()] = new EmitterCast< Sum >(emitSum);
	m_emitters[&type_of< Switch >()] = new EmitterCast< Switch >(emitSwitch);
	m_emitters[&type_of< Swizzle >()] = new EmitterCast< Swizzle >(emitSwizzle);
	m_emitters[&type_of< Tan >()] = new EmitterCast< Tan >(emitTan);
	m_emitters[&type_of< Transform >()] = new EmitterCast< Transform >(emitTransform);
	m_emitters[&type_of< Transpose >()] = new EmitterCast< Transpose >(emitTranspose);
	m_emitters[&type_of< Uniform >()] = new EmitterCast< Uniform >(emitUniform);
	m_emitters[&type_of< Vector >()] = new EmitterCast< Vector >(emitVector);
	m_emitters[&type_of< VertexInput >()] = new EmitterCast< VertexInput >(emitVertexInput);
	m_emitters[&type_of< VertexOutput >()] = new EmitterCast< VertexOutput >(emitVertexOutput);
}

GlslEmitter::~GlslEmitter()
{
	for (std::map< const Type*, Emitter* >::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
		delete i->second;
}

void GlslEmitter::emit(GlslContext& c, Node* node)
{
	std::map< const Type*, Emitter* >::iterator i = m_emitters.find(&node->getType());
	T_ASSERT_M (i != m_emitters.end(), L"No emitter for node");
	T_ASSERT (i->second);
	i->second->emit(c, node);
}

	}
}
