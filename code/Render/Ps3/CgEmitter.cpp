#include <cassert>
#include <cctype>
#include "Render/Ps3/CgEmitter.h"
#include "Render/Ps3/CgContext.h"
#include "Render/VertexElement.h"
#include "Render/Nodes.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

StringOutputStream& assign(StringOutputStream& f, CgVariable* out)
{
	f << cg_type_name(out->getType()) << " " << out->getName() << " = ";
	return f;
}

void emitAbs(CgContext& cx, Abs* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	assign(f, out) << "abs(" << in->getName() << ");" << Endl;
}

void emitAdd(CgContext& cx, Add* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, "Output", type);
	assign(f, out) << in1->cast(type) << " + " << in2->cast(type) << ";" << Endl;
}

void emitArcusCos(CgContext& cx, ArcusCos* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, "Theta");
	assert (theta->getType() == CtFloat);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	assign(f, out) << "acos(" << theta->getName() << ");" << Endl;
}

void emitArcusTan(CgContext& cx, ArcusTan* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* xy = cx.emitInput(node, "XY");
	assert (xy->getType() == CtFloat2);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	assign(f, out) << "atan2(" << xy->getName() << ".x, " << xy->getName() << ".y);" << Endl;
}

void emitClamp(CgContext& cx, Clamp* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	if (node->getMin() == 0.0f && node->getMax() == 1.0f)
		assign(f, out) << "saturate(" << in->getName() << ");" << Endl;
	else
		assign(f, out) << "clamp(" << in->getName() << ", " << node->getMin() << ", " << node->getMax() << ");" << Endl;
}

void emitColor(CgContext& cx, Color* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat4);
	Vector4 color = node->getColor();
	f << "float4 " << out->getName() << " = float4(" << color.x << ", " << color.y << ", " << color.z << ", " << color.w << ");" << Endl;
}

void emitConditional(CgContext& cx, Conditional* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* ref = cx.emitInput(node, "Reference");
	CgVariable* ct = cx.emitInput(node, "CaseTrue");
	CgVariable* cf = cx.emitInput(node, "CaseFalse");
	
	CgType type = std::max< CgType >(ct->getType(), cf->getType());
	CgVariable* out = cx.emitOutput(node, "Output", type);

	f << cg_type_name(out->getType()) << " " << out->getName() << ";" << Endl;

	switch (node->getOperator())
	{
	case Conditional::CoLess:
		f << "if (" << in->getName() << " < " << ref->getName() << ")" << Endl;
		break;
	case Conditional::CoLessEqual:
		f << "if (" << in->getName() << " <= " << ref->getName() << ")" << Endl;
		break;
	case Conditional::CoEqual:
		f << "if (" << in->getName() << " == " << ref->getName() << ")" << Endl;
		break;
	case Conditional::CoNotEqual:
		f << "if (" << in->getName() << " != " << ref->getName() << ")" << Endl;
		break;
	case Conditional::CoGreater:
		f << "if (" << in->getName() << " > " << ref->getName() << ")" << Endl;
		break;
	case Conditional::CoGreaterEqual:
		f << "if (" << in->getName() << " >= " << ref->getName() << ")" << Endl;
		break;
	default:
		assert (0);
	}

	f << IncreaseIndent;
	f << out->getName() << " = " << ct->cast(type) << ";" << Endl;
	f << DecreaseIndent;
	f << "else" << Endl;
	f << IncreaseIndent;
	f << out->getName() << " = " << cf->cast(type) << ";" << Endl;
	f << DecreaseIndent;
}

void emitCos(CgContext& cx, Cos* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, "Theta");
	assert (theta->getType() == CtFloat);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	assign(f, out) << "cos(" << theta->getName() << ");" << Endl;
}

void emitCross(CgContext& cx, Cross* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	assert (in1->getType() == in2->getType());
	CgVariable* out = cx.emitOutput(node, "Output", in1->getType());
	assign(f, out) << "cross(" << in1->getName() << ", " << in2->getName() << ");" << Endl;
}

void emitDiv(CgContext& cx, Div* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, "Output", type);
	assign(f, out) << in1->cast(type) << " / " << in2->cast(type) << ";" << Endl;
}

void emitDot(CgContext& cx, Dot* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	assign(f, out) << "dot(" << in1->cast(type) << ", " << in2->cast(type) << ");" << Endl;
}

void emitFraction(CgContext& cx, Fraction* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	assign(f, out) << "frac(" << in->getName() << ");" << Endl;
}

void emitIndexedUniform(CgContext& cx, IndexedUniform* node)
{
	const CgType c_parameterType[] = { CtFloat, CtFloat4, CtFloat4x4 };

	CgVariable* index = cx.emitInput(node, "Index");
	CgVariable* out = cx.getShader().createTemporaryVariable(
		node->findOutputPin("Output"),
		c_parameterType[node->getParameterType()]
	);

	StringOutputStream& fb = cx.getShader().getFormatter(CgShader::BtBody);
	assign(fb, out) << node->getParameterName() << "[" << index->getName() << "];" << Endl;

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getFormatter(CgShader::BtUniform);
		fu << "uniform " << cg_type_name(out->getType()) << " " << node->getParameterName() << "[" << node->getLength() << "];" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}
}

void emitInterpolator(CgContext& cx, Interpolator* node)
{
	assert (cx.inPixel());
	cx.enterVertex();

	CgVariable* in = cx.emitInput(node, "Input");

	int interpolatorId = cx.getShader().allocateInterpolator();
	std::wstring interpolator = "Attr" + toString(interpolatorId);

	StringOutputStream& fo = cx.getVertexShader().getFormatter(CgShader::BtOutput);
	fo << cg_type_name(in->getType()) << " " << interpolator << " : TEXCOORD" << interpolatorId << ";" << Endl;

	StringOutputStream& fb = cx.getVertexShader().getFormatter(CgShader::BtBody);
	fb << "o." << interpolator << " = " << in->getName() << ";" << Endl;

	cx.enterPixel();

	cx.getPixelShader().createVariable(
		node->findOutputPin("Output"),
		"i." + interpolator,
		in->getType()
	);

	StringOutputStream& fpi = cx.getPixelShader().getFormatter(CgShader::BtInput);
	fpi << cg_type_name(in->getType()) << " " << interpolator << " : TEXCOORD" << interpolatorId << ";" << Endl;
}

void emitLength(CgContext& cx, Length* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	assign(f, out) << "length(" << in->getName() << ");" << Endl;
}

void emitLerp(CgContext& cx, Lerp* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* blend = cx.emitInput(node, "Blend");
	assert (blend->getType() == CtFloat);
	CgVariable* out = cx.emitOutput(node, "Output", in1->getType());
	assign(f, out) << "lerp(" << in1->cast(type) << ", " << in2->cast(type) << ", " << blend->getName() << ");" << Endl;
}

void emitLog(CgContext& cx, Log* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	switch (node->getBase())
	{
	case Log::LbTwo:
		assign(f, out) << "log2(" << in->getName() << ");" << Endl;
		break;

	case Log::LbTen:
		assign(f, out) << "log10(" << in->getName() << ");" << Endl;
		break;

	case Log::LbNatural:
		assign(f, out) << "log(" << in->getName() << ");" << Endl;
		break;
	}
}

void emitMatrix(CgContext& cx, Matrix* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* xaxis = cx.emitInput(node, "XAxis");
	CgVariable* yaxis = cx.emitInput(node, "YAxis");
	CgVariable* zaxis = cx.emitInput(node, "ZAxis");
	CgVariable* translate = cx.emitInput(node, "Translate");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat4x4);
	assign(f, out) << Endl;
	f << "{" << Endl;
	f << IncreaseIndent;
	f << xaxis->cast(CtFloat4) << "," << Endl;
	f << yaxis->cast(CtFloat4) << "," << Endl;
	f << zaxis->cast(CtFloat4) << "," << Endl;
	f << translate->cast(CtFloat4) << Endl;
	f << DecreaseIndent;
	f << "};" << Endl;
}

void emitMixIn(CgContext& cx, MixIn* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* x = cx.emitInput(node, "X");
	CgVariable* y = cx.emitInput(node, "Y");
	CgVariable* z = cx.emitInput(node, "Z");
	CgVariable* w = cx.emitInput(node, "W");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat4);
	assign(f, out) << "float4(" << (x ? x->getName() : "0.0f") << ", " << (y ? y->getName() : "0.0f") << ", " << (z ? z->getName() : "0.0f") << ", " << (w ? w->getName() : "0.0f") << ");" << Endl;
}

void emitMixOut(CgContext& cx, MixOut* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* x = cx.emitOutput(node, "X", CtFloat);
	CgVariable* y = cx.emitOutput(node, "Y", CtFloat);
	CgVariable* z = cx.emitOutput(node, "Z", CtFloat);
	CgVariable* w = cx.emitOutput(node, "W", CtFloat);
	assign(f, x) << in->getName() << ".x;" << Endl;
	assign(f, y) << in->getName() << ".y;" << Endl;
	assign(f, z) << in->getName() << ".z;" << Endl;
	assign(f, w) << in->getName() << ".w;" << Endl;
}

void emitMul(CgContext& cx, Mul* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, "Output", type);
	assign(f, out) << in1->cast(type) << " * " << in2->cast(type) << ";" << Endl;
}

void emitMulAdd(CgContext& cx, MulAdd* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	CgVariable* in3 = cx.emitInput(node, "Input3");
	CgType type = std::max< CgType >(std::max< CgType >(in1->getType(), in2->getType()), in3->getType());
	CgVariable* out = cx.emitOutput(node, "Output", type);
	assign(f, out) << in1->cast(type) << " * " << in2->cast(type) << " + " << in3->cast(type) << ";" << Endl;
}

void emitNeg(CgContext& cx, Neg* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	assign(f, out) << "-" << in->getName() << ";" << Endl;
}

void emitNormalize(CgContext& cx, Normalize* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	assign(f, out) << "normalize(" << in->getName() << ");" << Endl;
}

void emitPixelOutput(CgContext& cx, PixelOutput* node)
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
		CELL_GCM_GEQUAL
	};

	cx.enterPixel();

	CgVariable* in = cx.emitInput(node, "Input");

	StringOutputStream& fpo = cx.getPixelShader().getFormatter(CgShader::BtOutput);
	fpo << "float4 Color0 : COLOR0;" << Endl;

	StringOutputStream& fpb = cx.getPixelShader().getFormatter(CgShader::BtBody);
	fpb << "o.Color0 = " << in->cast(CtFloat4) << ";" << Endl;

	RenderState& rs = cx.getRenderState();
	rs.cullFaceEnable = node->getCullMode() == PixelOutput::CmNever ? CELL_GCM_FALSE : CELL_GCM_TRUE;
	rs.cullFace = gcmCullFace[node->getCullMode()];
	rs.blendEnable = node->getBlendEnable() ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rs.blendEquation = gcmBlendEquation[node->getBlendOperation()];
	rs.blendFuncSrc = gcmBlendFunction[node->getBlendSource()];
	rs.blendFuncDest = gcmBlendFunction[node->getBlendDestination()];
	rs.depthTestEnable = node->getDepthEnable() ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rs.colorMask = node->getColorWriteEnable() ? CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_A : CELL_GCM_ZERO;
	rs.depthMask = node->getDepthWriteEnable() ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rs.depthFunc = gcmFunction[node->getDepthFunction()];
	rs.alphaTestEnable = node->getAlphaTestEnable() ? CELL_GCM_TRUE : CELL_GCM_FALSE;
	rs.alphaFunc = gcmFunction[node->getAlphaTestFunction()];
	rs.alphaRef = node->getAlphaTestReference();
}

void emitPolynomial(CgContext& cx, Polynomial* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);

	CgVariable* x = cx.emitInput(node, "X");
	CgVariable* coeffs = cx.emitInput(node, "Coefficients");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);

	assign(f, out);
	switch (coeffs->getType())
	{
	case CtFloat:
		f << coeffs->getName() << ".x * " << x->getName();
		break;
	case CtFloat2:
		f << coeffs->getName() << ".x * pow(" << x->getName() << ", 2) + " << coeffs->getName() << ".y * " << x->getName();
		break;
	case CtFloat3:
		f << coeffs->getName() << ".x * pow(" << x->getName() << ", 3) + " << coeffs->getName() << ".y * pow(" << x->getName() << ", 2) + " << coeffs->getName() << ".z * " << x->getName();
		break;
	case CtFloat4:
		f << coeffs->getName() << ".x * pow(" << x->getName() << ", 4) + " << coeffs->getName() << ".y * pow(" << x->getName() << ", 3) + " << coeffs->getName() << ".z * pow(" << x->getName() << ", 2) + " << coeffs->getName() << ".w * " << x->getName();
		break;
	}
	f << ";" << Endl;
}

void emitPow(CgContext& cx, Pow* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* exponent = cx.emitInput(node, "Exponent");
	CgVariable* in = cx.emitInput(node, "Input");
	CgType type = std::max< CgType >(exponent->getType(), in->getType());
	CgVariable* out = cx.emitOutput(node, "Output", type);
	assign(f, out) << "pow(" << in->cast(type) << ", " << exponent->cast(type) << ");" << Endl;
}

void emitReflect(CgContext& cx, Reflect* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* normal = cx.emitInput(node, "Normal");
	CgVariable* direction = cx.emitInput(node, "Direction");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat3);
	assign(f, out) << "reflect(" << normal->cast(CtFloat3) << ", " << direction->cast(CtFloat3) << ");" << Endl;
}

void emitSampler(CgContext& cx, Sampler* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* texCoord = cx.emitInput(node, "TexCoord");
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat4);

	std::wstring parameterName = "SamplerTexture_" + node->getParameterName();

	if (cx.inPixel())
	{
		switch (node->getLookup())
		{
		case Sampler::LuSimple:
			assign(f, out) << "tex2D(" << parameterName << ", " << texCoord->getName() << ");" << Endl;
			break;

		case Sampler::LuCube:
			assign(f, out) << "texCUBE(" << parameterName << ", " << texCoord->getName() << ");" << Endl;
			break;

		case Sampler::LuVolume:
			assign(f, out) << "tex3D(" << parameterName << ", " << texCoord->getName() << ");" << Endl;
			break;
		}
	}
	if (cx.inVertex())
	{
		switch (node->getLookup())
		{
		case Sampler::LuSimple:
			assign(f, out) << "tex2Dlod(" << parameterName << ", " << texCoord->cast(CtFloat4) << ");" << Endl;
			break;

		case Sampler::LuCube:
			assign(f, out) << "texCUBElod(" << parameterName << ", " << texCoord->cast(CtFloat4) << ");" << Endl;
			break;

		case Sampler::LuVolume:
			assign(f, out) << "tex3Dlod(" << parameterName << ", " << texCoord->cast(CtFloat4) << ");" << Endl;
			break;
		}
	}
	const std::set< std::wstring >& samplers = cx.getShader().getSamplers();
	if (samplers.find(parameterName) == samplers.end())
	{
		int sampler = int(samplers.size());

		StringOutputStream& fu = cx.getShader().getFormatter(CgShader::BtUniform);

		switch (node->getLookup())
		{
		case Sampler::LuSimple:
			fu << "uniform sampler2D " << parameterName << ";" << Endl;
			break;

		case Sampler::LuCube:
			fu << "uniform samplerCUBE " << parameterName << ";" << Endl;
			break;

		case Sampler::LuVolume:
			fu << "uniform sampler3D " << parameterName << ";" << Endl;
			break;
		}
		
		cx.getShader().addSampler(parameterName);
	}
}

void emitScalar(CgContext& cx, Scalar* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	f << cg_type_name(out->getType()) << " " << out->getName() << " = " << node->get() << ";" << Endl;
}

void emitSin(CgContext& cx, Sin* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, "Theta");
	assert (theta->getType() == CtFloat);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	assign(f, out) << "sin(" << theta->getName() << ");" << Endl;
}

void emitSqrt(CgContext& cx, Sqrt* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	assign(f, out) << "sqrt(" << in->getName() << ");" << Endl;
}

void emitSub(CgContext& cx, Sub* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in1 = cx.emitInput(node, "Input1");
	CgVariable* in2 = cx.emitInput(node, "Input2");
	CgType type = std::max< CgType >(in1->getType(), in2->getType());
	CgVariable* out = cx.emitOutput(node, "Output", type);
	assign(f, out) << in1->cast(type) << " - " << in2->cast(type) << ";" << Endl;
}

void emitSum(CgContext& cx, Sum* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);

	CgVariable* N = cx.emitOutput(node, "N", CtFloat);
	CgVariable* out = cx.emitOutput(node, "Output", /*input->getType()*/CtFloat4);

	assign(f, out) << "0;" << Endl;
	f << "for (float " << N->getName() << " = " << node->getFrom() << "; " << N->getName() << " <= " << node->getTo() << "; ++" << N->getName() << ")" << Endl;
	f << "{" << Endl;
	f << IncreaseIndent;

	CgVariable* input = cx.emitInput(node, "Input");
	f << out->getName() << " += " << input->getName() << ";" << Endl;

	f << DecreaseIndent;
	f << "}" << Endl;
}

void emitSwizzle(CgContext& cx, Swizzle* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);

	std::wstring map = node->get();
	assert (map.length() > 0);

	const CgType types[] = { CtFloat, CtFloat2, CtFloat3, CtFloat4 };
	CgType type = types[map.length() - 1];

	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", type);

	std::wstringstream ss;
	ss << cg_type_name(type) << "(";
	for (size_t i = 0; i < map.length(); ++i)
	{
		if (i > 0)
			ss << ", ";
		switch (std::tolower(map[i]))
		{
		case 'x':
		case 'y':
		case 'z':
		case 'w':
			ss << in->getName() << '.' << char(std::tolower(map[i]));
			break;
		case '0':
			ss << "0.0f";
			break;
		case '1':
			ss << "1.0f";
			break;
		}
	}
	ss << ")";

	assign(f, out) << ss.str() << ";" << Endl;
}

void emitTan(CgContext& cx, Tan* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* theta = cx.emitInput(node, "Theta");
	assert (theta->getType() == CtFloat);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat);
	assign(f, out) << "tan(" << theta->getName() << ");" << Endl;
}

void emitTransform(CgContext& cx, Transform* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* transform = cx.emitInput(node, "Transform");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	assign(f, out) << "columnMajorMul(" << transform->getName() << ", " << in->getName() << ");" << Endl;
}

void emitTranspose(CgContext& cx, Transpose* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* in = cx.emitInput(node, "Input");
	CgVariable* out = cx.emitOutput(node, "Output", in->getType());
	assign(f, out) << "transpose(" << in->getName() << ");" << Endl;
}

void emitUniform(CgContext& cx, Uniform* node)
{
	const CgType c_parameterType[] = { CtFloat, CtFloat4, CtFloat4x4 };
	CgVariable* out = cx.getShader().createVariable(
		node->findOutputPin("Output"),
		node->getParameterName(),
		c_parameterType[node->getParameterType()]
	);

	const std::set< std::wstring >& uniforms = cx.getShader().getUniforms();
	if (uniforms.find(node->getParameterName()) == uniforms.end())
	{
		StringOutputStream& fu = cx.getShader().getFormatter(CgShader::BtUniform);
		fu << "uniform " << cg_type_name(out->getType()) << " " << node->getParameterName() << ";" << Endl;
		cx.getShader().addUniform(node->getParameterName());
	}
}

void emitVector(CgContext& cx, Vector* node)
{
	StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
	CgVariable* out = cx.emitOutput(node, "Output", CtFloat4);
	f << "float4 " << out->getName() << " = float4(" << node->get().x << ", " << node->get().y << ", " << node->get().z << ", " << node->get().w << ");" << Endl;
}

void emitVertexInput(CgContext& cx, VertexInput* node)
{
	assert (cx.inVertex());
	CgVariable* out = cx.getShader().getInputVariable(node->getName());
	if (!out)
	{
		CgType type = cg_from_data_type(node->getDataType());

		std::wstringstream semantic;
		semantic << "ATTR" << int(node->getDataUsage());

		StringOutputStream& fi = cx.getVertexShader().getFormatter(CgShader::BtInput);
		fi << cg_type_name(type) << " " << node->getName() << " : " << semantic.str() << ";" << Endl;

		if (node->getDataUsage() == DuPosition && type != CtFloat4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin("Output"),
				CtFloat4
			);
			StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
			switch (type)
			{
			case CtFloat:
				assign(f, out) << "float4(i." << node->getName() << ".x, 0, 0, 1);" << Endl;
				break;

			case CtFloat2:
				assign(f, out) << "float4(i." << node->getName() << ".xy, 0, 1);" << Endl;
				break;

			case CtFloat3:
				assign(f, out) << "float4(i." << node->getName() << ".xyz, 1);" << Endl;
				break;
			}
		}
		else if (node->getDataUsage() == DuNormal && type != CtFloat4)
		{
			out = cx.getShader().createTemporaryVariable(
				node->findOutputPin("Output"),
				CtFloat4
			);
			StringOutputStream& f = cx.getShader().getFormatter(CgShader::BtBody);
			switch (type)
			{
			case CtFloat:
				assign(f, out) << "float4(i." << node->getName() << ".x, 0, 0, 0);" << Endl;
				break;

			case CtFloat2:
				assign(f, out) << "float4(i." << node->getName() << ".xy, 0, 0);" << Endl;
				break;

			case CtFloat3:
				assign(f, out) << "float4(i." << node->getName() << ".xyz, 0);" << Endl;
				break;
			}
		}
		else
		{
			out = cx.getShader().createVariable(
				node->findOutputPin("Output"),
				"i." + node->getName(),
				type
			);
		}

		cx.getShader().addInputVariable(node->getName(), out);
	}
	else
	{
		out = cx.getShader().createVariable(
			node->findOutputPin("Output"),
			out->getName(),
			out->getType()
		);
	}
}

void emitVertexOutput(CgContext& cx, VertexOutput* node)
{
	cx.enterVertex();
	CgVariable* in = cx.emitInput(node, "Input");

	StringOutputStream& fo = cx.getVertexShader().getFormatter(CgShader::BtOutput);
	fo << "float4 Position : POSITION0;" << Endl;

	StringOutputStream& fb = cx.getVertexShader().getFormatter(CgShader::BtBody);
	switch (in->getType())
	{
	case CtFloat:
		fb << "_Position_ = o.Position = float4(" << in->getName() << ".x, 0, 0, 1);" << Endl;
		break;

	case CtFloat2:
		fb << "_Position_ = o.Position = float4(" << in->getName() << ".xy, 0, 1);" << Endl;
		break;

	case CtFloat3:
		fb << "_Position_ = o.Position = float4(" << in->getName() << ".xyz, 1);" << Endl;
		break;

	case CtFloat4:
		fb << "_Position_ = o.Position = " << in->getName() << ";" << Endl;
		break;
	}
}

struct Emitter
{
	virtual void emit(CgContext& c, Node* node) = 0;
};

template < typename NodeType >
struct EmitterCast : public Emitter
{
	typedef void (*function_t)(CgContext& c, NodeType* node);

	function_t m_function;

	EmitterCast(function_t function) :
		m_function(function)
	{
	}

	virtual void emit(CgContext& c, Node* node)
	{
		assert (is_a< NodeType >(node));
		(*m_function)(c, static_cast< NodeType* >(node));
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
	m_emitters[&type_of< Div >()] = new EmitterCast< Div >(emitDiv);
	m_emitters[&type_of< Dot >()] = new EmitterCast< Dot >(emitDot);
	m_emitters[&type_of< Fraction >()] = new EmitterCast< Fraction >(emitFraction);
	m_emitters[&type_of< IndexedUniform >()] = new EmitterCast< IndexedUniform >(emitIndexedUniform);
	m_emitters[&type_of< Interpolator >()] = new EmitterCast< Interpolator >(emitInterpolator);
	m_emitters[&type_of< Length >()] = new EmitterCast< Length >(emitLength);
	m_emitters[&type_of< Lerp >()] = new EmitterCast< Lerp >(emitLerp);
	m_emitters[&type_of< Log >()] = new EmitterCast< Log >(emitLog);
	m_emitters[&type_of< Matrix >()] = new EmitterCast< Matrix >(emitMatrix);
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
	m_emitters[&type_of< Sampler >()] = new EmitterCast< Sampler >(emitSampler);
	m_emitters[&type_of< Scalar >()] = new EmitterCast< Scalar >(emitScalar);
	m_emitters[&type_of< Sin >()] = new EmitterCast< Sin >(emitSin);
	m_emitters[&type_of< Sqrt >()] = new EmitterCast< Sqrt >(emitSqrt);
	m_emitters[&type_of< Sub >()] = new EmitterCast< Sub >(emitSub);
	m_emitters[&type_of< Sum >()] = new EmitterCast< Sum >(emitSum);
	m_emitters[&type_of< Swizzle >()] = new EmitterCast< Swizzle >(emitSwizzle);
	m_emitters[&type_of< Tan >()] = new EmitterCast< Tan >(emitTan);
	m_emitters[&type_of< Transform >()] = new EmitterCast< Transform >(emitTransform);
	m_emitters[&type_of< Transpose >()] = new EmitterCast< Transpose >(emitTranspose);
	m_emitters[&type_of< Uniform >()] = new EmitterCast< Uniform >(emitUniform);
	m_emitters[&type_of< Vector >()] = new EmitterCast< Vector >(emitVector);
	m_emitters[&type_of< VertexInput >()] = new EmitterCast< VertexInput >(emitVertexInput);
	m_emitters[&type_of< VertexOutput >()] = new EmitterCast< VertexOutput >(emitVertexOutput);
}

CgEmitter::~CgEmitter()
{
	for (std::map< const Type*, Emitter* >::iterator i = m_emitters.begin(); i != m_emitters.end(); ++i)
		delete i->second;
}

void CgEmitter::emit(CgContext& c, Node* node)
{
	std::map< const Type*, Emitter* >::iterator i = m_emitters.find(&node->getType());
	assert (i != m_emitters.end());
	assert (i->second);
	i->second->emit(c, node);
}

	}
}
