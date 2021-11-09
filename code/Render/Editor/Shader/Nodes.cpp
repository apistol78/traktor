#include <algorithm>
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Format.h"
#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberBitMask.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/ITexture.h"
#include "Render/Editor/Shader/Nodes.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberRenderState : public MemberComplex
{
public:
	MemberRenderState(RenderState& ref, int32_t version)
	:	MemberComplex(L"", false)
	,	m_ref(ref)
	,	m_version(version)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< CullMode >::Key kCullModeOld[] =
		{
			{ L"CmNever", CullMode::Never },
			{ L"CmClockWise", CullMode::ClockWise },
			{ L"CmCounterClockWise", CullMode::CounterClockWise },
			{ 0 }
		};

		const MemberEnum< CullMode >::Key kCullMode[] =
		{
			{ L"Never", CullMode::Never },
			{ L"ClockWise", CullMode::ClockWise },
			{ L"CounterClockWise", CullMode::CounterClockWise },
			{ 0 }
		};

		const MemberEnum< BlendOperation >::Key kBlendOperations[] =
		{
			{ L"BoAdd", BoAdd },
			{ L"BoSubtract", BoSubtract },
			{ L"BoReverseSubtract", BoReverseSubtract },
			{ L"BoMin", BoMin },
			{ L"BoMax", BoMax },
			{ 0 }
		};

		const MemberEnum< BlendFactor >::Key kBlendFactors[] =
		{
			{ L"BfOne", BfOne },
			{ L"BfZero", BfZero },
			{ L"BfSourceColor", BfSourceColor },
			{ L"BfOneMinusSourceColor", BfOneMinusSourceColor },
			{ L"BfDestinationColor", BfDestinationColor },
			{ L"BfOneMinusDestinationColor", BfOneMinusDestinationColor },
			{ L"BfSourceAlpha", BfSourceAlpha },
			{ L"BfOneMinusSourceAlpha", BfOneMinusSourceAlpha },
			{ L"BfDestinationAlpha", BfDestinationAlpha },
			{ L"BfOneMinusDestinationAlpha", BfOneMinusDestinationAlpha },
			{ 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompareFunctions[] =
		{
			{ L"CfAlways", CfAlways },
			{ L"CfNever", CfNever },
			{ L"CfLess", CfLess },
			{ L"CfLessEqual", CfLessEqual },
			{ L"CfGreater", CfGreater },
			{ L"CfGreaterEqual", CfGreaterEqual },
			{ L"CfEqual", CfEqual },
			{ L"CfNotEqual", CfNotEqual },
			{ 0 }
		};

		const MemberBitMask::Bit kColorWriteBits[] =
		{
			{ L"red", CwRed },
			{ L"green", CwGreen },
			{ L"blue", CwBlue },
			{ L"alpha", CwAlpha },
			{ 0 }
		};

		const MemberEnum< StencilOperation >::Key kStencilOperations[] =
		{
			{ L"SoKeep", SoKeep },
			{ L"SoZero", SoZero },
			{ L"SoReplace", SoReplace },
			{ L"SoIncrementSaturate", SoIncrementSaturate },
			{ L"SoDecrementSaturate", SoDecrementSaturate },
			{ L"SoInvert", SoInvert },
			{ L"SoIncrement", SoIncrement },
			{ L"SoDecrement", SoDecrement },
			{ 0 }
		};

		if (m_version >= 9)
			s >> MemberEnum< CullMode >(L"cullMode", m_ref.cullMode, kCullMode);
		else
			s >> MemberEnum< CullMode >(L"cullMode", m_ref.cullMode, kCullModeOld);

		s >> Member< bool >(L"blendEnable", m_ref.blendEnable);

		if (m_version >= 7)
		{
			s >> MemberEnum< BlendOperation >(L"blendColorOperation", m_ref.blendColorOperation, kBlendOperations);
			s >> MemberEnum< BlendFactor >(L"blendColorSource", m_ref.blendColorSource, kBlendFactors);
			s >> MemberEnum< BlendFactor >(L"blendColorDestination", m_ref.blendColorDestination, kBlendFactors);
			s >> MemberEnum< BlendOperation >(L"blendAlphaOperation", m_ref.blendAlphaOperation, kBlendOperations);
			s >> MemberEnum< BlendFactor >(L"blendAlphaSource", m_ref.blendAlphaSource, kBlendFactors);
			s >> MemberEnum< BlendFactor >(L"blendAlphaDestination", m_ref.blendAlphaDestination, kBlendFactors);
		}
		else
		{
			s >> MemberEnum< BlendOperation >(L"blendOperation", m_ref.blendColorOperation, kBlendOperations);
			s >> MemberEnum< BlendFactor >(L"blendSource", m_ref.blendColorSource, kBlendFactors);
			s >> MemberEnum< BlendFactor >(L"blendDestination", m_ref.blendColorDestination, kBlendFactors);

			m_ref.blendAlphaOperation = BoAdd;
			m_ref.blendAlphaSource = BfOne;
			m_ref.blendAlphaDestination = BfOne;
		}

		s >> MemberBitMask(L"colorWriteMask", m_ref.colorWriteMask, kColorWriteBits);
		s >> Member< bool >(L"depthEnable", m_ref.depthEnable);
		s >> Member< bool >(L"depthWriteEnable", m_ref.depthWriteEnable);
		s >> MemberEnum< CompareFunction >(L"depthFunction", m_ref.depthFunction, kCompareFunctions);
		s >> Member< bool >(L"alphaTestEnable", m_ref.alphaTestEnable);
		s >> MemberEnum< CompareFunction >(L"alphaTestFunction", m_ref.alphaTestFunction, kCompareFunctions);
		s >> Member< int32_t >(L"alphaTestReference", m_ref.alphaTestReference);

		if (m_version >= 4)
			s >> Member< bool >(L"alphaToCoverageEnable", m_ref.alphaToCoverageEnable);

		if (m_version >= 1)
			s >> Member< bool >(L"wireframe", m_ref.wireframe);

		if (m_version >= 2)
		{
			s >> Member< bool >(L"stencilEnable", m_ref.stencilEnable);
			s >> MemberEnum< StencilOperation >(L"stencilFail", m_ref.stencilFail, kStencilOperations);
			s >> MemberEnum< StencilOperation >(L"stencilZFail", m_ref.stencilZFail, kStencilOperations);
			s >> MemberEnum< StencilOperation >(L"stencilPass", m_ref.stencilPass, kStencilOperations);
			s >> MemberEnum< CompareFunction >(L"stencilFunction", m_ref.stencilFunction, kCompareFunctions);
			s >> Member< uint32_t >(L"stencilReference", m_ref.stencilReference);
			s >> Member< uint32_t >(L"stencilMask", m_ref.stencilMask);
		}
	}

private:
	RenderState& m_ref;
	int32_t m_version;
};

class MemberSamplerState : public MemberComplex
{
public:
	MemberSamplerState(SamplerState& ref)
	:	MemberComplex(L"", false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< Filter >::Key kFilter[] =
		{
			{ L"FtPoint", FtPoint },
			{ L"FtLinear", FtLinear },
			{ 0 }
		};

		const MemberEnum< Address >::Key kAddress[] =
		{
			{ L"AdWrap", AdWrap },
			{ L"AdMirror", AdMirror },
			{ L"AdClamp", AdClamp },
			{ L"AdBorder", AdBorder },
			{ 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompareFunctions[] =
		{
			{ L"CfAlways", CfAlways },
			{ L"CfNever", CfNever },
			{ L"CfLess", CfLess },
			{ L"CfLessEqual", CfLessEqual },
			{ L"CfGreater", CfGreater },
			{ L"CfGreaterEqual", CfGreaterEqual },
			{ L"CfEqual", CfEqual },
			{ L"CfNotEqual", CfNotEqual },
			{ L"CfNone", CfNone },
			{ 0 }
		};

		s >> MemberEnum< Filter >(L"minFilter", m_ref.minFilter, kFilter);
		s >> MemberEnum< Filter >(L"mipFilter", m_ref.mipFilter, kFilter);
		s >> MemberEnum< Filter >(L"magFilter", m_ref.magFilter, kFilter);
		s >> MemberEnum< Address >(L"addressU", m_ref.addressU, kAddress);
		s >> MemberEnum< Address >(L"addressV", m_ref.addressV, kAddress);
		s >> MemberEnum< Address >(L"addressW", m_ref.addressW, kAddress);
		s >> MemberEnum< CompareFunction >(L"compare", m_ref.compare, kCompareFunctions);
		s >> Member< float >(L"mipBias", m_ref.mipBias);
		s >> Member< bool >(L"ignoreMips", m_ref.ignoreMips);
		s >> Member< bool >(L"useAnisotropic", m_ref.useAnisotropic);
	}

private:
	SamplerState& m_ref;
};

		}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Abs", 0, Abs, ImmutableNode)

const ImmutableNode::InputPinDesc c_Abs_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Abs_o[] = { { L"Output" }, { 0 } };

Abs::Abs()
:	ImmutableNode(c_Abs_i, c_Abs_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Add", 0, Add, ImmutableNode)

const ImmutableNode::InputPinDesc c_Add_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Add_o[] = { { L"Output" }, { 0 } };

Add::Add()
:	ImmutableNode(c_Add_i, c_Add_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ArcusCos", 0, ArcusCos, ImmutableNode)

const ImmutableNode::InputPinDesc c_ArcusCos_i[] = { { L"Theta", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_ArcusCos_o[] = { { L"Output" }, { 0 } };

ArcusCos::ArcusCos()
:	ImmutableNode(c_ArcusCos_i, c_ArcusCos_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ArcusTan", 0, ArcusTan, ImmutableNode)

const ImmutableNode::InputPinDesc c_ArcusTan_i[] = { { L"XY", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_ArcusTan_o[] = { { L"Output" }, { 0 } };

ArcusTan::ArcusTan()
:	ImmutableNode(c_ArcusTan_i, c_ArcusTan_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Clamp", 0, Clamp, ImmutableNode)

const ImmutableNode::InputPinDesc c_Clamp_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Clamp_o[] = { { L"Output" }, { 0 } };

Clamp::Clamp(float min, float max)
:	ImmutableNode(c_Clamp_i, c_Clamp_o)
,	m_min(min)
,	m_max(max)
{
}

void Clamp::setMin(float min)
{
	m_min = min;
}

float Clamp::getMin() const
{
	return m_min;
}

void Clamp::setMax(float max)
{
	m_max = max;
}

float Clamp::getMax() const
{
	return m_max;
}

std::wstring Clamp::getInformation() const
{
	StringOutputStream ss;
	ss << m_min << L" - " << m_max;
	return ss.str();
}

void Clamp::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< float >(L"min", m_min);
	s >> Member< float >(L"max", m_max);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Branch", 0, Branch, ImmutableNode)

const ImmutableNode::InputPinDesc c_Branch_i[] = { { L"True", false }, { L"False", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Branch_o[] = { { L"Output" }, { 0 } };

Branch::Branch(const std::wstring& parameterName)
:	ImmutableNode(c_Branch_i, c_Branch_o)
,	m_parameterName(parameterName)
{
}

void Branch::setParameterName(const std::wstring& parameterName)
{
	m_parameterName = parameterName;
}

const std::wstring& Branch::getParameterName() const
{
	return m_parameterName;
}

std::wstring Branch::getInformation() const
{
	return m_parameterName;
}

void Branch::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> Member< std::wstring >(L"parameterName", m_parameterName);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Color", 2, Color, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Color_o[] = { { L"Output" }, { 0 } };

Color::Color(const traktor::Color4f& color)
:	ImmutableNode(nullptr, c_Color_o)
,	m_color(color)
,	m_linear(false)
{
}

void Color::setColor(const traktor::Color4f& color)
{
	m_color = color;
}

const traktor::Color4f& Color::getColor() const
{
	return m_color;
}

void Color::setLinear(bool linear)
{
	m_linear = linear;
}

bool Color::getLinear() const
{
	return m_linear;
}

std::wstring Color::getInformation() const
{
	StringOutputStream ss;

	float ev = m_color.getEV();

	Color4f c0 = m_color;
	c0.setEV(traktor::Scalar(0.0f));

	float r = c0.getRed();
	float g = c0.getGreen();
	float b = c0.getBlue();
	float a = c0.getAlpha();

	int32_t ir = int32_t(r * 255.0f);
	int32_t ig = int32_t(g * 255.0f);
	int32_t ib = int32_t(b * 255.0f);
	int32_t ia = int32_t(a * 255.0f);

	ss << ir << L", " << ig << L", " << ib << L", " << ia << L" (EV " << (ev > 0.0f ? L"+" : L"") << ev << L")";
	return ss.str();
}

void Color::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	if (s.getVersion< Color >() >= 1)
		s >> Member< traktor::Color4f >(L"color", m_color, AttributeHdr());
	else
	{
		Color4ub ldr;
		s >> Member< traktor::Color4ub >(L"color", ldr);
		m_color = Color4f::fromColor4ub(ldr);
	}

	if (s.getVersion< Color >() >= 2)
		s >> Member< bool >(L"linear", m_linear);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Comment", 0, Comment, ImmutableNode)

const ImmutableNode::InputPinDesc c_Comment_i[] = { { 0 } };
const ImmutableNode::OutputPinDesc c_Comment_o[] = { { 0 } };

Comment::Comment()
:	ImmutableNode(c_Comment_i, c_Comment_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeOutput", 0, ComputeOutput, ImmutableNode)

const ImmutableNode::InputPinDesc c_ComputeOutput_i[] = { { L"Enable", true }, { L"Storage", false }, { L"Offset", false }, { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_ComputeOutput_o[] = { { 0 } };

ComputeOutput::ComputeOutput()
:	ImmutableNode(c_ComputeOutput_i, c_ComputeOutput_o)
,	m_technique(L"Default")
{
}

void ComputeOutput::setTechnique(const std::wstring& technique)
{
	m_technique = technique;
}

const std::wstring& ComputeOutput::getTechnique() const
{
	return m_technique;
}

std::wstring ComputeOutput::getInformation() const
{
	return m_technique;
}

void ComputeOutput::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> Member< std::wstring >(L"technique", m_technique);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Conditional", 1, Conditional, ImmutableNode)

const ImmutableNode::InputPinDesc c_Conditional_i[] = { { L"Input", false }, { L"Reference", false }, { L"CaseTrue", false }, { L"CaseFalse", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Conditional_o[] = { { L"Output" }, { 0 } };

Conditional::Conditional()
:	ImmutableNode(c_Conditional_i, c_Conditional_o)
,	m_branch(BrAuto)
,	m_operator(CoLess)
{
}

void Conditional::setBranch(Branch branch)
{
	m_branch = branch;
}

Conditional::Branch Conditional::getBranch() const
{
	return m_branch;
}

void Conditional::setOperator(Conditional::Operator op)
{
	m_operator = op;
}

Conditional::Operator Conditional::getOperator() const
{
	return m_operator;
}

std::wstring Conditional::getInformation() const
{
	const wchar_t* h[] = { L"<", L"<=", L"=", L"<>", L">", L">=" };
	return h[int(m_operator)];
}

void Conditional::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	const MemberEnum< Branch >::Key kBranch[] =
	{
		{ L"BrAuto", BrAuto },
		{ L"BrStatic", BrStatic },
		{ L"BrDynamic", BrDynamic },
		{ 0 }
	};

	const MemberEnum< Operator >::Key kOperator[] =
	{
		{ L"CoLess", CoLess },
		{ L"CoLessEqual", CoLessEqual },
		{ L"CoEqual", CoEqual },
		{ L"CoNotEqual", CoNotEqual },
		{ L"CoGreater", CoGreater },
		{ L"CoGreaterEqual", CoGreaterEqual },
		{ 0 }
	};

	if (s.getVersion() >= 1)
		s >> MemberEnum< Branch >(L"branch", m_branch, kBranch);

	s >> MemberEnum< Operator >(L"operator", m_operator, kOperator);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Connected", 0, Connected, ImmutableNode)

const ImmutableNode::InputPinDesc c_Connected_i[] = { { L"Input", true }, { L"True", true }, { L"False", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_Connected_o[] = { { L"Output" }, { 0 } };

Connected::Connected()
:	ImmutableNode(c_Connected_i, c_Connected_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Cos", 0, Cos, ImmutableNode)

const ImmutableNode::InputPinDesc c_Cos_i[] = { { L"Theta", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Cos_o[] = { { L"Output" }, { 0 } };

Cos::Cos()
:	ImmutableNode(c_Cos_i, c_Cos_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Cross", 0, Cross, ImmutableNode)

const ImmutableNode::InputPinDesc c_Cross_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Cross_o[] = { { L"Output" }, { 0 } };

Cross::Cross()
:	ImmutableNode(c_Cross_i, c_Cross_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Derivative", 0, Derivative, ImmutableNode)

const ImmutableNode::InputPinDesc c_Derivative_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Derivative_o[] = { { L"Output" }, { 0 } };

Derivative::Derivative()
:	ImmutableNode(c_Derivative_i, c_Derivative_o)
,	m_axis(DaX)
{
}

Derivative::Axis Derivative::getAxis() const
{
	return m_axis;
}

std::wstring Derivative::getInformation() const
{
	switch (m_axis)
	{
	case DaX:
		return L"f'(x)";
	case DaY:
		return L"f'(y)";
	}
	return L"";
}

void Derivative::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	const MemberEnum< Axis >::Key kAxis[] =
	{
		{ L"DaX", DaX },
		{ L"DaY", DaY },
		{ 0 }
	};

	s >> MemberEnum< Axis >(L"axis", m_axis, kAxis);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Discard", 0, Discard, ImmutableNode)

const ImmutableNode::InputPinDesc c_Discard_i[] = { { L"Input", false }, { L"Reference", false }, { L"Pass", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Discard_o[] = { { L"Output" }, { 0 } };

Discard::Discard()
:	ImmutableNode(c_Discard_i, c_Discard_o)
,	m_operator(CoLess)
{
}

void Discard::setOperator(Discard::Operator op)
{
	m_operator = op;
}

Discard::Operator Discard::getOperator() const
{
	return m_operator;
}

std::wstring Discard::getInformation() const
{
	const wchar_t* h[] = { L"<", L"<=", L"=", L"<>", L">", L">=" };
	return h[int(m_operator)];
}

void Discard::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	const MemberEnum< Operator >::Key kOperator[] =
	{
		{ L"CoLess", CoLess },
		{ L"CoLessEqual", CoLessEqual },
		{ L"CoEqual", CoEqual },
		{ L"CoNotEqual", CoNotEqual },
		{ L"CoGreater", CoGreater },
		{ L"CoGreaterEqual", CoGreaterEqual },
		{ 0 }
	};

	s >> MemberEnum< Operator >(L"operator", m_operator, kOperator);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.DispatchIndex", 0, DispatchIndex, ImmutableNode)

const ImmutableNode::InputPinDesc c_DispatchIndex_i[] = { { 0 } };
const ImmutableNode::OutputPinDesc c_DispatchIndex_o[] = { { L"Output" }, { 0 } };

DispatchIndex::DispatchIndex()
:	ImmutableNode(c_DispatchIndex_i, c_DispatchIndex_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Div", 0, Div, ImmutableNode)

const ImmutableNode::InputPinDesc c_Div_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Div_o[] = { { L"Output" }, { 0 } };

Div::Div()
:	ImmutableNode(c_Div_i, c_Div_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Dot", 0, Dot, ImmutableNode)

const ImmutableNode::InputPinDesc c_Dot_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Dot_o[] = { { L"Output" }, { 0 } };

Dot::Dot()
:	ImmutableNode(c_Dot_i, c_Dot_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Exp", 0, Exp, ImmutableNode)

const ImmutableNode::InputPinDesc c_Exp_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Exp_o[] = { { L"Output" }, { 0 } };

Exp::Exp()
:	ImmutableNode(c_Exp_i, c_Exp_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Fraction", 0, Fraction, ImmutableNode)

const ImmutableNode::InputPinDesc c_Fraction_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Fraction_o[] = { { L"Output" }, { 0 } };

Fraction::Fraction()
:	ImmutableNode(c_Fraction_i, c_Fraction_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.FragmentPosition", 0, FragmentPosition, ImmutableNode)

const ImmutableNode::OutputPinDesc c_FragmentPosition_o[] = { { L"Output" }, { 0 } };

FragmentPosition::FragmentPosition()
:	ImmutableNode(nullptr, c_FragmentPosition_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.FrontFace", 0, FrontFace, ImmutableNode)

const ImmutableNode::OutputPinDesc c_FrontFace_o[] = { { L"Output" }, { 0 } };

FrontFace::FrontFace()
:	ImmutableNode(nullptr, c_FrontFace_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IndexedUniform", 3, IndexedUniform, ImmutableNode)

const ImmutableNode::InputPinDesc c_IndexedUniform_i[] = { { L"Index", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_IndexedUniform_o[] = { { L"Output" }, { 0 } };

IndexedUniform::IndexedUniform(
	const std::wstring& parameterName,
	ParameterType type,
	UpdateFrequency frequency,
	int32_t length
)
:	ImmutableNode(c_IndexedUniform_i, c_IndexedUniform_o)
,	m_parameterName(parameterName)
,	m_type(type)
,	m_frequency(frequency)
,	m_length(length)
{
}

void IndexedUniform::setParameterName(const std::wstring& parameterName)
{
	m_parameterName = parameterName;
}

const std::wstring& IndexedUniform::getParameterName() const
{
	return m_parameterName;
}

void IndexedUniform::setParameterType(ParameterType type)
{
	m_type = type;
}

ParameterType IndexedUniform::getParameterType() const
{
	return m_type;
}

void IndexedUniform::setFrequency(UpdateFrequency frequency)
{
	m_frequency = frequency;
}

UpdateFrequency IndexedUniform::getFrequency() const
{
	return m_frequency;
}

void IndexedUniform::setLength(int32_t length)
{
	m_length = length;
}

int32_t IndexedUniform::getLength() const
{
	return m_length;
}

std::wstring IndexedUniform::getInformation() const
{
	StringOutputStream ss;
	ss << m_parameterName << L"[" << m_length << L"]";
	return ss.str();
}

void IndexedUniform::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"parameterName", m_parameterName);

	if (s.getVersion() >= 3)
	{
		const MemberEnum< ParameterType >::Key kParameterType_Keys[] =
		{
			{ L"Scalar", ParameterType::Scalar },
			{ L"Vector", ParameterType::Vector },
			{ L"Matrix", ParameterType::Matrix },
			{ 0 }
		};
		s >> MemberEnum< ParameterType >(L"type", m_type, kParameterType_Keys);
	}
	else
	{
		const MemberEnum< ParameterType >::Key kParameterType_Keys[] =
		{
			{ L"PtScalar", ParameterType::Scalar },
			{ L"PtVector", ParameterType::Vector },
			{ L"PtMatrix", ParameterType::Matrix },
			{ 0 }
		};
		s >> MemberEnum< ParameterType >(L"type", m_type, kParameterType_Keys);
	}

	if (s.getVersion() >= 1)
	{
		if (s.getVersion() >= 2)
		{
			const MemberEnum< UpdateFrequency >::Key kUpdateFrequency_Keys[] =
			{
				{ L"Once", UpdateFrequency::Once },
				{ L"Frame", UpdateFrequency::Frame },
				{ L"Draw", UpdateFrequency::Draw },
				{ 0 }
			};
			s >> MemberEnum< UpdateFrequency >(L"frequency", m_frequency, kUpdateFrequency_Keys);
		}
		else
		{
			const MemberEnum< UpdateFrequency >::Key kUpdateFrequency_Keys[] =
			{
				{ L"UfOnce", UpdateFrequency::Once },
				{ L"UfFrame", UpdateFrequency::Frame },
				{ L"UfDraw", UpdateFrequency::Draw },
				{ 0 }
			};
			s >> MemberEnum< UpdateFrequency >(L"frequency", m_frequency, kUpdateFrequency_Keys);
		}
	}

	s >> Member< int32_t >(L"length", m_length);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.InputPort", 2, InputPort, ImmutableNode)

const ImmutableNode::OutputPinDesc c_InputPort_o[] = { { L"Output" }, { 0 } };

InputPort::InputPort()
:	ImmutableNode(nullptr, c_InputPort_o)
,	m_name(L"")
,	m_connectable(true)
,	m_optional(false)
,	m_haveDefaultValue(false)
,	m_defaultValue(0.0f)
{
}

InputPort::InputPort(const std::wstring& name, bool connectable, bool optional, bool haveDefaultValue, float defaultValue)
:	ImmutableNode(nullptr, c_InputPort_o)
,	m_name(name)
,	m_connectable(connectable)
,	m_optional(optional)
,	m_haveDefaultValue(haveDefaultValue)
,	m_defaultValue(defaultValue)
{
}

void InputPort::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& InputPort::getName() const
{
	return m_name;
}

void InputPort::setConnectable(bool connectable)
{
	m_connectable = connectable;
}

bool InputPort::isConnectable() const
{
	return m_connectable;
}

void InputPort::setOptional(bool optional)
{
	m_optional = optional;
}

bool InputPort::isOptional() const
{
	return m_optional;
}

void InputPort::setHaveDefaultValue(bool haveDefaultValue)
{
	m_haveDefaultValue = haveDefaultValue;
}

bool InputPort::haveDefaultValue() const
{
	return m_haveDefaultValue;
}

void InputPort::setDefaultValue(float defaultValue)
{
	m_defaultValue = defaultValue;
}

float InputPort::getDefaultValue() const
{
	return m_defaultValue;
}

std::wstring InputPort::getInformation() const
{
	return m_name;
}

void InputPort::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"connectable", m_connectable);
		s >> Member< bool >(L"optional", m_optional);

		if (s.getVersion() >= 2)
			s >> Member< bool >(L"haveDefaultValue", m_haveDefaultValue);
		else
			m_haveDefaultValue = true;

		s >> Member< float >(L"defaultValue", m_defaultValue);
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Instance", 0, Instance, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Instance_o[] = { { L"Output" }, { 0 } };

Instance::Instance()
:	ImmutableNode(nullptr, c_Instance_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Interpolator", 0, Interpolator, ImmutableNode)

const ImmutableNode::InputPinDesc c_Interpolator_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Interpolator_o[] = { { L"Output" }, { 0 } };

Interpolator::Interpolator()
:	ImmutableNode(c_Interpolator_i, c_Interpolator_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Iterate", 0, Iterate, ImmutableNode)

const ImmutableNode::InputPinDesc c_Iterate_i[] = { { L"Input", false }, { L"Initial", true }, { L"Condition", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_Iterate_o[] = { { L"N" }, { L"Output" }, { 0 } };

Iterate::Iterate(int32_t from, int32_t to)
:	ImmutableNode(c_Iterate_i, c_Iterate_o)
,	m_from(from)
,	m_to(to)
{
}

void Iterate::setFrom(int32_t from)
{
	m_from = from;
}

int32_t Iterate::getFrom() const
{
	return m_from;
}

void Iterate::setTo(int32_t to)
{
	m_to = to;
}

int32_t Iterate::getTo() const
{
	return m_to;
}

std::wstring Iterate::getInformation() const
{
	StringOutputStream ss;
	ss << m_from << L" -> " << m_to;
	return ss.str();
}

void Iterate::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< int32_t >(L"from", m_from);
	s >> Member< int32_t >(L"to", m_to);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Iterate2", 0, Iterate2, ImmutableNode)

const ImmutableNode::InputPinDesc c_Iterate2_i[] 
{
	{ L"From", false },
	{ L"To", false },
	{ L"Input0", false },
	{ L"Initial0", true },
	{ L"Input1", true },
	{ L"Initial1", true },
	{ L"Input2", true },
	{ L"Initial2", true },
	{ L"Input3", true },
	{ L"Initial3", true },
	{ L"Condition", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Iterate2_o[] =
{
	{ L"N" },
	{ L"Output0" },
	{ L"Output1" },
	{ L"Output2" },
	{ L"Output3" },
	{ 0 }
};

Iterate2::Iterate2()
:	ImmutableNode(c_Iterate2_i, c_Iterate2_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Iterate2d", 0, Iterate2d, ImmutableNode)

const ImmutableNode::InputPinDesc c_Iterate2d_i[] = { { L"Input", false }, { L"Initial", true }, { L"Condition", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_Iterate2d_o[] = { { L"X" }, { L"Y" }, { L"Output" }, { 0 } };

Iterate2d::Iterate2d(int32_t fromX, int32_t toX, int32_t fromY, int32_t toY)
:	ImmutableNode(c_Iterate2d_i, c_Iterate2d_o)
,	m_fromX(fromX)
,	m_toX(toX)
,	m_fromY(fromY)
,	m_toY(toY)
{
}

void Iterate2d::setFromX(int32_t fromX)
{
	m_fromX = fromX;
}

int32_t Iterate2d::getFromX() const
{
	return m_fromX;
}

void Iterate2d::setToX(int32_t toX)
{
	m_toX = toX;
}

int32_t Iterate2d::getToX() const
{
	return m_toX;
}

void Iterate2d::setFromY(int32_t fromY)
{
	m_fromY = fromY;
}

int32_t Iterate2d::getFromY() const
{
	return m_fromY;
}

void Iterate2d::setToY(int32_t toY)
{
	m_toY = toY;
}

int32_t Iterate2d::getToY() const
{
	return m_toY;
}

std::wstring Iterate2d::getInformation() const
{
	StringOutputStream ss;
	ss << m_fromX << L" -> " << m_toX << L", " << m_fromY << L" -> " << m_toY;
	return ss.str();
}

void Iterate2d::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< int32_t >(L"fromX", m_fromX);
	s >> Member< int32_t >(L"toX", m_toX);
	s >> Member< int32_t >(L"fromY", m_fromY);
	s >> Member< int32_t >(L"toY", m_toY);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Length", 0, Length, ImmutableNode)

const ImmutableNode::InputPinDesc c_Length_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Length_o[] = { { L"Output" }, { 0 } };

Length::Length()
:	ImmutableNode(c_Length_i, c_Length_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Lerp", 0, Lerp, ImmutableNode)

const ImmutableNode::InputPinDesc c_Lerp_i[] = { { L"Input1", false }, { L"Input2", false }, { L"Blend", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Lerp_o[] = { { L"Output" }, { 0 } };

Lerp::Lerp()
:	ImmutableNode(c_Lerp_i, c_Lerp_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Log", 0, Log, ImmutableNode)

const ImmutableNode::InputPinDesc c_Log_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Log_o[] = { { L"Output" }, { 0 } };

Log::Log(Base base)
:	ImmutableNode(c_Log_i, c_Log_o)
,	m_base(base)
{
}

Log::Base Log::getBase() const
{
	return m_base;
}

std::wstring Log::getInformation() const
{
	switch (m_base)
	{
	case LbTwo:
		return L"2";

	case LbTen:
		return L"10";

	case LbNatural:
		return L"e";
	}
	return L"";
}

void Log::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	const MemberEnum< Base >::Key kBase[] =
	{
		{ L"LbTwo", LbTwo },
		{ L"LbTen", LbTen },
		{ L"LbNatural", LbNatural },
		{ 0 }
	};

	s >> MemberEnum< Base >(L"base", m_base, kBase);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MatrixIn", 0, MatrixIn, ImmutableNode)

const ImmutableNode::InputPinDesc c_MatrixIn_i[] = { { L"XAxis", true }, { L"YAxis", true }, { L"ZAxis", true }, { L"Translate", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_MatrixIn_o[] = { { L"Output" }, { 0 } };

MatrixIn::MatrixIn()
:	ImmutableNode(c_MatrixIn_i, c_MatrixIn_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MatrixOut", 0, MatrixOut, ImmutableNode)

const ImmutableNode::InputPinDesc c_MatrixOut_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_MatrixOut_o[] = { { L"XAxis" }, { L"YAxis" }, { L"ZAxis" }, { L"Translate" }, { 0 } };

MatrixOut::MatrixOut()
:	ImmutableNode(c_MatrixOut_i, c_MatrixOut_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Max", 0, Max, ImmutableNode)

const ImmutableNode::InputPinDesc c_Max_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Max_o[] = { { L"Output" }, { 0 } };

Max::Max()
:	ImmutableNode(c_Max_i, c_Max_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Min", 0, Min, ImmutableNode)

const ImmutableNode::InputPinDesc c_Min_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Min_o[] = { { L"Output" }, { 0 } };

Min::Min()
:	ImmutableNode(c_Min_i, c_Min_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MixIn", 0, MixIn, ImmutableNode)

const ImmutableNode::InputPinDesc c_MixIn_i[] = { { L"X", true }, { L"Y", true }, { L"Z", true }, { L"W", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_MixIn_o[] = { { L"Output" }, { 0 } };

MixIn::MixIn()
:	ImmutableNode(c_MixIn_i, c_MixIn_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MixOut", 0, MixOut, ImmutableNode)

const ImmutableNode::InputPinDesc c_MixOut_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_MixOut_o[] = { { L"X" }, { L"Y" }, { L"Z" }, { L"W" }, { 0 } };

MixOut::MixOut()
:	ImmutableNode(c_MixOut_i, c_MixOut_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Mul", 0, Mul, ImmutableNode)

const ImmutableNode::InputPinDesc c_Mul_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Mul_o[] = { { L"Output" }, { 0 } };

Mul::Mul()
:	ImmutableNode(c_Mul_i, c_Mul_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MulAdd", 0, MulAdd, ImmutableNode)

const ImmutableNode::InputPinDesc c_MulAdd_i[] = { { L"Input1", false }, { L"Input2", false }, { L"Input3", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_MulAdd_o[] = { { L"Output" }, { 0 } };

MulAdd::MulAdd()
:	ImmutableNode(c_MulAdd_i, c_MulAdd_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Neg", 0, Neg, ImmutableNode)

const ImmutableNode::InputPinDesc c_Neg_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Neg_o[] = { { L"Output" }, { 0 } };

Neg::Neg()
:	ImmutableNode(c_Neg_i, c_Neg_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Normalize", 0, Normalize, ImmutableNode)

const ImmutableNode::InputPinDesc c_Normalize_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Normalize_o[] = { { L"Output" }, { 0 } };

Normalize::Normalize()
:	ImmutableNode(c_Normalize_i, c_Normalize_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.OutputPort", 0, OutputPort, ImmutableNode)

const ImmutableNode::InputPinDesc c_OutputPort_i[] = { { L"Input", false }, { 0 } };

OutputPort::OutputPort(const std::wstring& name)
:	ImmutableNode(c_OutputPort_i, nullptr)
,	m_name(name)
{
}

void OutputPort::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& OutputPort::getName() const
{
	return m_name;
}

std::wstring OutputPort::getInformation() const
{
	return m_name;
}

void OutputPort::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> Member< std::wstring >(L"name", m_name);
}
/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Platform", 0, Platform, ImmutableNode)

const ImmutableNode::InputPinDesc c_Platform_i[] =
{
	{ L"Android", true },
	{ L"iOS", true },
	{ L"Linux", true },
	{ L"macOS", true },
	{ L"PS3", true },
	{ L"RaspberryPI", true },
	{ L"Windows", true },
	{ L"Other", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Platform_o[] = { { L"Output" }, { 0 } };

Platform::Platform()
:	ImmutableNode(c_Platform_i, c_Platform_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PixelOutput", 9, PixelOutput, ImmutableNode)

const ImmutableNode::InputPinDesc c_PixelOutput_i[] = { { L"Enable", true }, { L"Input", false }, { L"Input1", true }, { L"Input2", true }, { L"Input3", true }, { L"State", true }, { 0 } };

PixelOutput::PixelOutput()
:	ImmutableNode(c_PixelOutput_i, nullptr)
,	m_technique(L"Default")
,	m_priority(0)
,	m_registerCount(0)
,	m_precisionHint(PrecisionHint::Undefined)
{
}

void PixelOutput::setTechnique(const std::wstring& technique)
{
	m_technique = technique;
}

const std::wstring& PixelOutput::getTechnique() const
{
	return m_technique;
}

void PixelOutput::setPriority(uint32_t priority)
{
	m_priority = priority;
}

uint32_t PixelOutput::getPriority() const
{
	return m_priority;
}

void PixelOutput::setRenderState(const RenderState& renderState)
{
	m_renderState = renderState;
}

const RenderState& PixelOutput::getRenderState() const
{
	return m_renderState;
}

void PixelOutput::setRegisterCount(uint32_t registerCount)
{
	m_registerCount = registerCount;
}

uint32_t PixelOutput::getRegisterCount() const
{
	return m_registerCount;
}

void PixelOutput::setPrecisionHint(PrecisionHint precisionHint)
{
	m_precisionHint = precisionHint;
}

PrecisionHint PixelOutput::getPrecisionHint() const
{
	return m_precisionHint;
}

std::wstring PixelOutput::getInformation() const
{
	return m_technique;
}

void PixelOutput::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"technique", m_technique);

	if (s.getVersion() >= 5)
	{
		const MemberBitMask::Bit c_RenderPriorityBits[] =
		{
			{ L"setup", RpSetup },
			{ L"opaque", RpOpaque },
			{ L"postOpaque", RpPostOpaque },
			{ L"alphaBlend", RpAlphaBlend },
			{ L"postAlphaBlend", RpPostAlphaBlend },
			{ L"overlay", RpOverlay },
			{ 0 }
		};
		s >> MemberBitMask(L"priority", m_priority, c_RenderPriorityBits);
	}

	s >> MemberRenderState(m_renderState, s.getVersion());

	if (s.getVersion() >= 3)
		s >> Member< uint32_t >(L"registerCount", m_registerCount);

	if (s.getVersion() >= 6)
	{
		if (s.getVersion() >= 8)
		{
			const MemberEnum< PrecisionHint >::Key c_PrecisionHintKeys[] =
			{
				{ L"Undefined", PrecisionHint::Undefined },
				{ L"Low", PrecisionHint::Low },
				{ L"Medium", PrecisionHint::Medium },
				{ L"High", PrecisionHint::High },
				{ 0 }
			};
			s >> MemberEnum< PrecisionHint >(L"precisionHint", m_precisionHint, c_PrecisionHintKeys);
		}
		else
		{
			const MemberEnum< PrecisionHint >::Key c_PrecisionHintKeys[] =
			{
				{ L"PhUndefined", PrecisionHint::Undefined },
				{ L"PhLow", PrecisionHint::Low },
				{ L"PhMedium", PrecisionHint::Medium },
				{ L"PhHigh", PrecisionHint::High },
				{ 0 }
			};
			s >> MemberEnum< PrecisionHint >(L"precisionHint", m_precisionHint, c_PrecisionHintKeys);
		}
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PixelState", 9, PixelState, ImmutableNode)

const ImmutableNode::OutputPinDesc c_PixelState_o[] = { { L"Output" }, { 0 } };

PixelState::PixelState()
	: ImmutableNode(nullptr, c_PixelState_o)
	, m_priority(0)
	, m_precisionHint(PrecisionHint::Undefined)
{
}

void PixelState::setPriority(uint32_t priority)
{
	m_priority = priority;
}

uint32_t PixelState::getPriority() const
{
	return m_priority;
}

void PixelState::setRenderState(const RenderState& renderState)
{
	m_renderState = renderState;
}

const RenderState& PixelState::getRenderState() const
{
	return m_renderState;
}

void PixelState::setPrecisionHint(PrecisionHint precisionHint)
{
	m_precisionHint = precisionHint;
}

PrecisionHint PixelState::getPrecisionHint() const
{
	return m_precisionHint;
}

void PixelState::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	if (s.getVersion() >= 5)
	{
		const MemberBitMask::Bit c_RenderPriorityBits[] =
		{
			{ L"setup", RpSetup },
			{ L"opaque", RpOpaque },
			{ L"postOpaque", RpPostOpaque },
			{ L"alphaBlend", RpAlphaBlend },
			{ L"postAlphaBlend", RpPostAlphaBlend },
			{ L"overlay", RpOverlay },
			{ 0 }
		};
		s >> MemberBitMask(L"priority", m_priority, c_RenderPriorityBits);
	}

	s >> MemberRenderState(m_renderState, s.getVersion());

	if (s.getVersion() >= 8)
	{
		if (s.getVersion() >= 9)
		{
			const MemberEnum< PrecisionHint >::Key c_PrecisionHintKeys[] =
			{
				{ L"Undefined", PrecisionHint::Undefined },
				{ L"Low", PrecisionHint::Low },
				{ L"Medium", PrecisionHint::Medium },
				{ L"High", PrecisionHint::High },
				{ 0 }
			};
			s >> MemberEnum< PrecisionHint >(L"precisionHint", m_precisionHint, c_PrecisionHintKeys);
		}
		else
		{
			const MemberEnum< PrecisionHint >::Key c_PrecisionHintKeys[] =
			{
				{ L"PhUndefined", PrecisionHint::Undefined },
				{ L"PhLow", PrecisionHint::Low },
				{ L"PhMedium", PrecisionHint::Medium },
				{ L"PhHigh", PrecisionHint::High },
				{ 0 }
			};
			s >> MemberEnum< PrecisionHint >(L"precisionHint", m_precisionHint, c_PrecisionHintKeys);
		}
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Polynomial", 0, Polynomial, ImmutableNode)

const ImmutableNode::InputPinDesc c_Polynomial_i[] = { { L"X", false }, { L"Coefficients", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Polynomial_o[] = { { L"Output" }, { 0 } };

Polynomial::Polynomial()
:	ImmutableNode(c_Polynomial_i, c_Polynomial_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Pow", 0, Pow, ImmutableNode)

const ImmutableNode::InputPinDesc c_Pow_i[] = { { L"Exponent", false }, { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Pow_o[] = { { L"Output" }, { 0 } };

Pow::Pow()
:	ImmutableNode(c_Pow_i, c_Pow_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ReadStruct", 0, ReadStruct, ImmutableNode)

const ImmutableNode::InputPinDesc c_ReadStruct_i[] = { { L"Struct", false }, { L"Index", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_ReadStruct_o[] = { { L"Output" }, { 0 } };

ReadStruct::ReadStruct()
:	ImmutableNode(c_ReadStruct_i, c_ReadStruct_o)
{
}

void ReadStruct::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& ReadStruct::getName() const
{
	return m_name;
}

std::wstring ReadStruct::getInformation() const
{
	return m_name;
}

void ReadStruct::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RecipSqrt", 0, RecipSqrt, ImmutableNode)

const ImmutableNode::InputPinDesc c_RecipSqrt_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_RecipSqrt_o[] = { { L"Output" }, { 0 } };

RecipSqrt::RecipSqrt()
:	ImmutableNode(c_RecipSqrt_i, c_RecipSqrt_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Renderer", 0, Renderer, ImmutableNode)

const ImmutableNode::InputPinDesc c_Renderer_i[] =
{
	{ L"DX11", true },
	{ L"Vulkan", true },
	{ L"GCM", true },
	{ L"Other", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Renderer_o[] = { { L"Output" }, { 0 } };

Renderer::Renderer()
:	ImmutableNode(c_Renderer_i, c_Renderer_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Reflect", 0, Reflect, ImmutableNode)

const ImmutableNode::InputPinDesc c_Reflect_i[] = { { L"Normal", false }, { L"Direction", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Reflect_o[] = { { L"Output" }, { 0 } };

Reflect::Reflect()
:	ImmutableNode(c_Reflect_i, c_Reflect_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Repeat", 0, Repeat, ImmutableNode)

const ImmutableNode::InputPinDesc c_Repeat_i[] = { { L"Input", false }, { L"Initial", true }, { L"Condition", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Repeat_o[] = { { L"N" }, { L"Output" }, { 0 } };

Repeat::Repeat()
:	ImmutableNode(c_Repeat_i, c_Repeat_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Round", 0, Round, ImmutableNode)

const ImmutableNode::InputPinDesc c_Round_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Round_o[] = { { L"Output" }, { 0 } };

Round::Round()
:	ImmutableNode(c_Round_i, c_Round_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sampler", 5, Sampler, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sampler_i[] = { { L"Texture", false }, { L"TexCoord", false }, { L"Mip", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_Sampler_o[] = { { L"Output" }, { 0 } };

Sampler::Sampler()
:	ImmutableNode(c_Sampler_i, c_Sampler_o)
{
}

Sampler::Sampler(const SamplerState& state)
:	ImmutableNode(c_Sampler_i, c_Sampler_o)
,	m_state(state)
{
}

void Sampler::setSamplerState(const SamplerState& state)
{
	m_state = state;
}

const SamplerState& Sampler::getSamplerState() const
{
	return m_state;
}

void Sampler::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	if (s.getVersion() >= 5)
		s >> MemberSamplerState(m_state);
	else
	{
		const MemberEnum< Filter >::Key kFilter[] =
		{
			{ L"FtPoint", FtPoint },
			{ L"FtLinear", FtLinear },
			{ 0 }
		};

		const MemberEnum< Address >::Key kAddress[] =
		{
			{ L"AdWrap", AdWrap },
			{ L"AdMirror", AdMirror },
			{ L"AdClamp", AdClamp },
			{ L"AdBorder", AdBorder },
			{ 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompare[] =
		{
			{ L"CmNo", CfNone },
			{ L"CmAlways", CfAlways },
			{ L"CmNever", CfNever },
			{ L"CmLess", CfLess },
			{ L"CmLessEqual", CfLessEqual },
			{ L"CmGreater", CfGreater },
			{ L"CmGreaterEqual", CfGreaterEqual },
			{ L"CmEqual", CfEqual },
			{ L"CmNotEqual", CfNotEqual },
			{ 0 }
		};

		s >> MemberEnum< Filter >(L"minFilter", m_state.minFilter, kFilter);
		s >> MemberEnum< Filter >(L"mipFilter", m_state.mipFilter, kFilter);
		s >> MemberEnum< Filter >(L"magFilter", m_state.magFilter, kFilter);
		s >> MemberEnum< Address >(L"addressU", m_state.addressU, kAddress);
		s >> MemberEnum< Address >(L"addressV", m_state.addressV, kAddress);
		s >> MemberEnum< Address >(L"addressW", m_state.addressW, kAddress);

		if (s.getVersion() >= 4)
			s >> MemberEnum< CompareFunction >(L"compare", m_state.compare, kCompare);

		if (s.getVersion() >= 1)
			s >> Member< float >(L"mipBias", m_state.mipBias);

		if (s.getVersion() >= 3)
			s >> Member< bool >(L"ignoreMips", m_state.ignoreMips);

		if (s.getVersion() >= 2)
			s >> Member< bool >(L"useAnisotropic", m_state.useAnisotropic);
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Scalar", 0, Scalar, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Scalar_o[] = { { L"Output" }, { 0 } };

Scalar::Scalar(float value)
:	ImmutableNode(nullptr, c_Scalar_o)
,	m_value(value)
{
}

void Scalar::set(float value)
{
	m_value = value;
}

float Scalar::get() const
{
	return m_value;
}

std::wstring Scalar::getInformation() const
{
	StringOutputStream ss;
	ss << m_value;
	return ss.str();
}

void Scalar::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> Member< float >(L"value", m_value);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sign", 0, Sign, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sign_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Sign_o[] = { { L"Output" }, { 0 } };

Sign::Sign()
:	ImmutableNode(c_Sign_i, c_Sign_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sin", 0, Sin, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sin_i[] = { { L"Theta", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Sin_o[] = { { L"Output" }, { 0 } };

Sin::Sin()
:	ImmutableNode(c_Sin_i, c_Sin_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sqrt", 0, Sqrt, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sqrt_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Sqrt_o[] = { { L"Output" }, { 0 } };

Sqrt::Sqrt()
:	ImmutableNode(c_Sqrt_i, c_Sqrt_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Step", 0, Step, ImmutableNode)

const ImmutableNode::InputPinDesc c_Step_i[] = { { L"X", false }, { L"Y", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Step_o[] = { { L"Output" }, { 0 } };

Step::Step()
:	ImmutableNode(c_Step_i, c_Step_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Struct", 0, Struct, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Struct_o[] = { { L"Output" }, { 0 } };

Struct::Struct()
:	ImmutableNode(nullptr, c_Struct_o)
{
}

const std::wstring& Struct::getParameterName() const
{
	return m_parameterName;
}

const AlignedVector< Struct::NamedElement >& Struct::getElements() const
{
	return m_elements;
}

DataType Struct::getElementType(const std::wstring& name) const
{
	auto it = std::find_if(m_elements.begin(), m_elements.end(), [&](const NamedElement& elm) {
		return elm.name == name;
	});
	return it != m_elements.end() ? it->type : DtFloat1;
}

std::wstring Struct::getInformation() const
{
	return m_parameterName;
}

void Struct::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"parameterName", m_parameterName);
	s >> MemberAlignedVector< NamedElement, MemberComposite< NamedElement > >(L"elements", m_elements);
}

Struct::NamedElement::NamedElement()
:	type(DtFloat1)
{
}

void Struct::NamedElement::serialize(ISerializer& s)
{
	const MemberEnum< DataType >::Key kDataType[] =
	{
		{ L"DtFloat1", DtFloat1 },
		{ L"DtFloat2", DtFloat2 },
		{ L"DtFloat3", DtFloat3 },
		{ L"DtFloat4", DtFloat4 },
		{ L"DtByte4", DtByte4 },
		{ L"DtByte4N", DtByte4N },
		{ L"DtShort2", DtShort2 },
		{ L"DtShort4", DtShort4 },
		{ L"DtShort2N", DtShort2N },
		{ L"DtShort4N", DtShort4N },
		{ L"DtHalf2", DtHalf2 },
		{ L"DtHalf4", DtHalf4 },
		{ L"DtInteger1", DtInteger1 },
		{ L"DtInteger2", DtInteger2 },
		{ L"DtInteger3", DtInteger3 },
		{ L"DtInteger4", DtInteger4 },
		{ 0 }
	};

	s >> Member< std::wstring >(L"name", name);
	s >> MemberEnum< DataType >(L"type", type, kDataType);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sub", 0, Sub, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sub_i[] = { { L"Input1", false }, { L"Input2", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Sub_o[] = { { L"Output" }, { 0 } };

Sub::Sub()
:	ImmutableNode(c_Sub_i, c_Sub_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sum", 0, Sum, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sum_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Sum_o[] = { { L"N" }, { L"Output" }, { 0 } };

Sum::Sum(int32_t from, int32_t to)
:	ImmutableNode(c_Sum_i, c_Sum_o)
,	m_from(from)
,	m_to(to)
{
}

void Sum::setFrom(int32_t from)
{
	m_from = from;
}

int32_t Sum::getFrom() const
{
	return m_from;
}

void Sum::setTo(int32_t to)
{
	m_to = to;
}

int32_t Sum::getTo() const
{
	return m_to;
}

std::wstring Sum::getInformation() const
{
	StringOutputStream ss;
	ss << m_from << L" -> " << m_to;
	return ss.str();
}

void Sum::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< int32_t >(L"from", m_from);
	s >> Member< int32_t >(L"to", m_to);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Switch", 2, Switch, Node)

Switch::Switch()
:	m_branch(BrAuto)
,	m_width(1)
{
	updatePins();
}

Switch::~Switch()
{
	for (auto& inputPin : m_inputPins)
		delete inputPin;
	for (auto& outputPin : m_outputPins)
		delete outputPin;
}

void Switch::setBranch(Branch branch)
{
	m_branch = branch;
}

Switch::Branch Switch::getBranch() const
{
	return m_branch;
}

void Switch::setWidth(int32_t width)
{
	m_width = width;
}

int32_t Switch::getWidth() const
{
	return m_width;
}

const AlignedVector< int32_t >& Switch::getCases() const
{
	return m_cases;
}

int Switch::getInputPinCount() const
{
	return int(m_inputPins.size());
}

const InputPin* Switch::getInputPin(int index) const
{
	return m_inputPins[index];
}

int Switch::getOutputPinCount() const
{
	return m_width;
}

const OutputPin* Switch::getOutputPin(int index) const
{
	return m_outputPins[index];
}

void Switch::serialize(ISerializer& s)
{
	Node::serialize(s);

	const MemberEnum< Branch >::Key kBranch[] =
	{
		{ L"BrAuto", BrAuto },
		{ L"BrStatic", BrStatic },
		{ L"BrDynamic", BrDynamic },
		{ 0 }
	};

	if (s.getVersion() >= 1)
		s >> MemberEnum< Branch >(L"branch", m_branch, kBranch);

	if (s.getVersion< Switch >() >= 2)
		s >> Member< int32_t >(L"width", m_width, AttributeRange(1));

	s >> MemberAlignedVector< int32_t >(L"cases", m_cases);

	if (s.getDirection() == ISerializer::Direction::Read)
		updatePins();
}

void Switch::updatePins()
{
	const Guid c_null;

	for (auto& inputPin : m_inputPins)
		delete inputPin;
	for (auto& outputPin : m_outputPins)
		delete outputPin;

	m_inputPins.resize(1 + (1 + m_cases.size()) * m_width);
	m_inputPins[0] = new InputPin(this, c_null, L"Select", false);
	for (int32_t c = 0; c < m_width; ++c)
	{
		StringOutputStream ss;
		ss << L"Default";
		if (m_width > 1)
			ss << L" (" << c << L")";
		m_inputPins[1 + c] = new InputPin(this, c_null, ss.str(), false);
	}
	for (uint32_t i = 0; i < (uint32_t)m_cases.size(); ++i)
	{
		for (int32_t c = 0; c < m_width; ++c)
		{
			StringOutputStream ss;
			ss << L"Case " << m_cases[i];
			if (m_width > 1)
				ss << L" (" << c << L")";
			m_inputPins[1 + (1 + i) * m_width + c] = new InputPin(this, c_null, ss.str(), false);
		}
	}

	m_outputPins.resize(m_width);
	for (int32_t c = 0; c < m_width; ++c)
	{
		StringOutputStream ss;
		ss << L"Output";
		if (m_width > 1)
			ss << L" (" << c << L")";
		m_outputPins[c] = new OutputPin(this, c_null, ss.str());
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Swizzle", 0, Swizzle, ImmutableNode)

const ImmutableNode::InputPinDesc c_Swizzle_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Swizzle_o[] = { { L"Output" }, { 0 } };

Swizzle::Swizzle(const std::wstring& swizzle)
:	ImmutableNode(c_Swizzle_i, c_Swizzle_o)
,	m_swizzle(swizzle)
{
}

void Swizzle::set(const std::wstring& swizzle)
{
	m_swizzle = swizzle;
}

const std::wstring& Swizzle::get() const
{
	return m_swizzle;
}

std::wstring Swizzle::getInformation() const
{
	return m_swizzle;
}

void Swizzle::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> Member< std::wstring >(L"swizzle", m_swizzle);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Tan", 0, Tan, ImmutableNode)

const ImmutableNode::InputPinDesc c_Tan_i[] = { { L"Theta", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Tan_o[] = { { L"Output" }, { 0 } };

Tan::Tan()
:	ImmutableNode(c_Tan_i, c_Tan_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TargetSize", 0, TargetSize, ImmutableNode)

const ImmutableNode::InputPinDesc c_TargetSize_i[] = { { 0 } };
const ImmutableNode::OutputPinDesc c_TargetSize_o[] = { { L"Output" }, { 0 } };

TargetSize::TargetSize()
:	ImmutableNode(c_TargetSize_i, c_TargetSize_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Texture", 1, Texture, ImmutableNode)

const ImmutableNode::InputPinDesc c_Texture_i[] = { { 0 } };
const ImmutableNode::OutputPinDesc c_Texture_o[] = { { L"Output" }, { 0 } };

Texture::Texture(const Guid& external, ParameterType type)
:	ImmutableNode(c_Texture_i, c_Texture_o)
,	m_external(external)
,	m_type(type)
{
}

void Texture::setExternal(const Guid& external)
{
	m_external = external;
}

const Guid& Texture::getExternal() const
{
	return m_external;
}

void Texture::setParameterType(ParameterType type)
{
	m_type = type;
}

ParameterType Texture::getParameterType() const
{
	return m_type;
}

void Texture::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< Guid >(L"external", m_external, AttributeType(type_of< ITexture >()));

	if (s.getVersion() >= 1)
	{
		const MemberEnum< ParameterType >::Key c_ParameterType_Keys[] =
		{
			{ L"Texture2D", ParameterType::Texture2D },
			{ L"Texture3D", ParameterType::Texture3D },
			{ L"TextureCube", ParameterType::TextureCube },
			{ 0 }
		};		
		s >> MemberEnum< ParameterType >(L"type", m_type, c_ParameterType_Keys);
	}
	else
	{
		const MemberEnum< ParameterType >::Key c_ParameterType_Keys[] =
		{
			{ L"PtTexture2D", ParameterType::Texture2D },
			{ L"PtTexture3D", ParameterType::Texture3D },
			{ L"PtTextureCube", ParameterType::TextureCube },
			{ 0 }
		};
		s >> MemberEnum< ParameterType >(L"type", m_type, c_ParameterType_Keys);
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureSize", 0, TextureSize, ImmutableNode)

const ImmutableNode::InputPinDesc c_TextureSize_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_TextureSize_o[] = { { L"Output" }, { 0 } };

TextureSize::TextureSize()
:	ImmutableNode(c_TextureSize_i, c_TextureSize_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Transform", 0, Transform, ImmutableNode)

const ImmutableNode::InputPinDesc c_Transform_i[] = { { L"Input", false }, { L"Transform", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Transform_o[] = { { L"Output" }, { 0 } };

Transform::Transform()
:	ImmutableNode(c_Transform_i, c_Transform_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Transpose", 0, Transpose, ImmutableNode)

const ImmutableNode::InputPinDesc c_Transpose_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Transpose_o[] = { { L"Output" }, { 0 } };

Transpose::Transpose()
:	ImmutableNode(c_Transpose_i, c_Transpose_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Truncate", 0, Truncate, ImmutableNode)

const ImmutableNode::InputPinDesc c_Truncate_i[] = { { L"Input", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Truncate_o[] = { { L"Output" }, { 0 } };

Truncate::Truncate()
:	ImmutableNode(c_Truncate_i, c_Truncate_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Type", 0, Type, ImmutableNode)

const ImmutableNode::InputPinDesc c_Type_i[] = { { L"Type", true }, { L"Scalar", true }, { L"Vector", true }, { L"Matrix", true }, { L"Texture", true }, { L"State", true }, { L"Default", false }, { 0 } };
const ImmutableNode::OutputPinDesc c_Type_o[] = { { L"Output" }, { 0 } };

Type::Type()
:	ImmutableNode(c_Type_i, c_Type_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Uniform", 3, Uniform, ImmutableNode)

const ImmutableNode::InputPinDesc c_Uniform_i[] = { { L"Initial", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_Uniform_o[] = { { L"Output" }, { 0 } };

Uniform::Uniform(
	const std::wstring& parameterName,
	ParameterType type,
	UpdateFrequency frequency
)
:	ImmutableNode(c_Uniform_i, c_Uniform_o)
,	m_parameterName(parameterName)
,	m_type(type)
,	m_frequency(frequency)
{
}

void Uniform::setParameterName(const std::wstring& parameterName)
{
	m_parameterName = parameterName;
}

const std::wstring& Uniform::getParameterName() const
{
	return m_parameterName;
}

void Uniform::setParameterType(ParameterType type)
{
	m_type = type;
}

ParameterType Uniform::getParameterType() const
{
	return m_type;
}

void Uniform::setFrequency(UpdateFrequency frequency)
{
	m_frequency = frequency;
}

UpdateFrequency Uniform::getFrequency() const
{
	return m_frequency;
}

std::wstring Uniform::getInformation() const
{
	return m_parameterName;
}

void Uniform::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"parameterName", m_parameterName);

	if (s.getVersion() >= 3)
	{
		const MemberEnum< ParameterType >::Key kParameterType_Keys[] =
		{
			{ L"Scalar", ParameterType::Scalar },
			{ L"Vector", ParameterType::Vector },
			{ L"Matrix", ParameterType::Matrix },
			{ L"Texture2D", ParameterType::Texture2D },
			{ L"Texture3D", ParameterType::Texture3D },
			{ L"TextureCube", ParameterType::TextureCube },
			{ L"Image2D", ParameterType::Image2D },
			{ L"Image3D", ParameterType::Image3D },
			{ L"ImageCube", ParameterType::ImageCube },
			{ 0 }
		};
		s >> MemberEnum< ParameterType >(L"type", m_type, kParameterType_Keys);
	}
	else
	{
		const MemberEnum< ParameterType >::Key kParameterType_Keys[] =
		{
			{ L"PtScalar", ParameterType::Scalar },
			{ L"PtVector", ParameterType::Vector },
			{ L"PtMatrix", ParameterType::Matrix },
			{ L"PtTexture2D", ParameterType::Texture2D },
			{ L"PtTexture3D", ParameterType::Texture3D },
			{ L"PtTextureCube", ParameterType::TextureCube },
			{ L"PtImage2D", ParameterType::Image2D },
			{ L"PtImage3D", ParameterType::Image3D },
			{ L"PtImageCube", ParameterType::ImageCube },
			{ 0 }
		};
		s >> MemberEnum< ParameterType >(L"type", m_type, kParameterType_Keys);
	}

	if (s.getVersion() >= 1)
	{
		if (s.getVersion() >= 2)
		{
			const MemberEnum< UpdateFrequency >::Key kUpdateFrequency_Keys[] =
			{
				{ L"Once", UpdateFrequency::Once },
				{ L"Frame", UpdateFrequency::Frame },
				{ L"Draw", UpdateFrequency::Draw },
				{ 0 }
			};
			s >> MemberEnum< UpdateFrequency >(L"frequency", m_frequency, kUpdateFrequency_Keys);
		}
		else
		{
			const MemberEnum< UpdateFrequency >::Key kUpdateFrequency_Keys[] =
			{
				{ L"UfOnce", UpdateFrequency::Once },
				{ L"UfFrame", UpdateFrequency::Frame },
				{ L"UfDraw", UpdateFrequency::Draw },
				{ 0 }
			};
			s >> MemberEnum< UpdateFrequency >(L"frequency", m_frequency, kUpdateFrequency_Keys);
		}
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Variable", 1, Variable, ImmutableNode)

const ImmutableNode::InputPinDesc c_Variable_i[] = { { L"Input", true }, { 0 } };
const ImmutableNode::OutputPinDesc c_Variable_o[] = { { L"Output" }, { 0 } };

Variable::Variable()
:	ImmutableNode(c_Variable_i, c_Variable_o)
,	m_global(false)
{
}

void Variable::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Variable::getName() const
{
	return m_name;
}

bool Variable::isGlobal() const
{
	return m_global;
}

std::wstring Variable::getInformation() const
{
	return m_name;
}

void Variable::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion< Variable >() >= 1)
		s >> Member< bool >(L"global", m_global);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Vector", 0, Vector, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Vector_o[] = { { L"Output" }, { 0 } };

Vector::Vector(const Vector4& value)
:	ImmutableNode(nullptr, c_Vector_o)
,	m_value(value)
{
}

void Vector::set(const Vector4& value)
{
	m_value = value;
}

const Vector4& Vector::get() const
{
	return m_value;
}

std::wstring Vector::getInformation() const
{
	StringOutputStream ss;
	ss << m_value;
	return ss.str();
}

void Vector::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> Member< Vector4 >(L"value", m_value);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.VertexInput", 0, VertexInput, ImmutableNode)

const ImmutableNode::OutputPinDesc c_VertexInput_o[] = { { L"Output" }, { 0 } };

VertexInput::VertexInput(const std::wstring& name, DataUsage usage, DataType type, int index)
:	ImmutableNode(nullptr, c_VertexInput_o)
,	m_name(name)
,	m_usage(usage)
,	m_type(type)
,	m_index(index)
{
}

void VertexInput::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& VertexInput::getName() const
{
	return m_name;
}

void VertexInput::setDataUsage(DataUsage usage)
{
	m_usage = usage;
}

DataUsage VertexInput::getDataUsage() const
{
	return m_usage;
}

void VertexInput::setDataType(DataType type)
{
	m_type = type;
}

DataType VertexInput::getDataType() const
{
	return m_type;
}

void VertexInput::setIndex(int32_t index)
{
	m_index = index;
}

int32_t VertexInput::getIndex() const
{
	return m_index;
}

std::wstring VertexInput::getInformation() const
{
	return m_name;
}

void VertexInput::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	const MemberEnum< DataUsage >::Key kDataUsage[] =
	{
		{ L"DuPosition", DuPosition },
		{ L"DuNormal", DuNormal },
		{ L"DuTangent", DuTangent },
		{ L"DuBinormal", DuBinormal },
		{ L"DuColor", DuColor },
		{ L"DuCustom", DuCustom },
		{ 0 }
	};

	const MemberEnum< DataType >::Key kDataType[] =
	{
		{ L"DtFloat1", DtFloat1 },
		{ L"DtFloat2", DtFloat2 },
		{ L"DtFloat3", DtFloat3 },
		{ L"DtFloat4", DtFloat4 },
		{ L"DtByte4", DtByte4 },
		{ L"DtByte4N", DtByte4N },
		{ L"DtShort2", DtShort2 },
		{ L"DtShort4", DtShort4 },
		{ L"DtShort2N", DtShort2N },
		{ L"DtShort4N", DtShort4N },
		{ L"DtHalf2", DtHalf2 },
		{ L"DtHalf4", DtHalf4 },
		{ L"DtInteger1", DtInteger1 },
		{ L"DtInteger2", DtInteger2 },
		{ L"DtInteger3", DtInteger3 },
		{ L"DtInteger4", DtInteger4 },
		{ 0 }
	};

	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberEnum< DataUsage >(L"usage", m_usage, kDataUsage);
	s >> MemberEnum< DataType >(L"type", m_type, kDataType);
	s >> Member< int32_t >(L"index", m_index);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.VertexOutput", 3, VertexOutput, ImmutableNode)

const ImmutableNode::InputPinDesc c_VertexOutput_i[] = { { L"Input", false }, { 0 } };

VertexOutput::VertexOutput()
:	ImmutableNode(c_VertexOutput_i, 0)
,	m_technique(L"Default")
,	m_precisionHint(PrecisionHint::Undefined)
{
}

void VertexOutput::setTechnique(const std::wstring& technique)
{
	m_technique = technique;
}

const std::wstring& VertexOutput::getTechnique() const
{
	return m_technique;
}

void VertexOutput::setPrecisionHint(PrecisionHint precisionHint)
{
	m_precisionHint = precisionHint;
}

PrecisionHint VertexOutput::getPrecisionHint() const
{
	return m_precisionHint;
}

std::wstring VertexOutput::getInformation() const
{
	return m_technique;
}

void VertexOutput::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"technique", m_technique);

	if (s.getVersion() >= 2)
	{
		if (s.getVersion() >= 3)
		{
			const MemberEnum< PrecisionHint >::Key c_PrecisionHintKeys[] =
			{
				{ L"Undefined", PrecisionHint::Undefined },
				{ L"Low", PrecisionHint::Low },
				{ L"Medium", PrecisionHint::Medium },
				{ L"High", PrecisionHint::High },
				{ 0 }
			};
			s >> MemberEnum< PrecisionHint >(L"precisionHint", m_precisionHint, c_PrecisionHintKeys);
		}
		else
		{
			const MemberEnum< PrecisionHint >::Key c_PrecisionHintKeys[] =
			{
				{ L"PhUndefined", PrecisionHint::Undefined },
				{ L"PhLow", PrecisionHint::Low },
				{ L"PhMedium", PrecisionHint::Medium },
				{ L"PhHigh", PrecisionHint::High },
				{ 0 }
			};
			s >> MemberEnum< PrecisionHint >(L"precisionHint", m_precisionHint, c_PrecisionHintKeys);
		}
	}
}

/*---------------------------------------------------------------------------*/

	}
}
