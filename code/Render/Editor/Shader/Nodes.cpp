/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/ITexture.h"
#include "Render/Editor/Shader/Nodes.h"

namespace traktor::render
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
			{ L"BoAdd", BlendOperation::Add },
			{ L"BoSubtract", BlendOperation::Subtract },
			{ L"BoReverseSubtract", BlendOperation::ReverseSubtract },
			{ L"BoMin", BlendOperation::Min },
			{ L"BoMax", BlendOperation::Max },
			{ 0 }
		};

		const MemberEnum< BlendFactor >::Key kBlendFactors[] =
		{
			{ L"BfOne", BlendFactor::One },
			{ L"BfZero", BlendFactor::Zero },
			{ L"BfSourceColor", BlendFactor::SourceColor },
			{ L"BfOneMinusSourceColor", BlendFactor::OneMinusSourceColor },
			{ L"BfDestinationColor", BlendFactor::DestinationColor },
			{ L"BfOneMinusDestinationColor", BlendFactor::OneMinusDestinationColor },
			{ L"BfSourceAlpha", BlendFactor::SourceAlpha },
			{ L"BfOneMinusSourceAlpha", BlendFactor::OneMinusSourceAlpha },
			{ L"BfDestinationAlpha", BlendFactor::DestinationAlpha },
			{ L"BfOneMinusDestinationAlpha", BlendFactor::OneMinusDestinationAlpha },
			{ 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompareFunctions[] =
		{
			{ L"CfAlways", CompareFunction::Always },
			{ L"CfNever", CompareFunction::Never },
			{ L"CfLess", CompareFunction::Less },
			{ L"CfLessEqual", CompareFunction::LessEqual },
			{ L"CfGreater", CompareFunction::Greater },
			{ L"CfGreaterEqual", CompareFunction::GreaterEqual },
			{ L"CfEqual", CompareFunction::Equal },
			{ L"CfNotEqual", CompareFunction::NotEqual },
			{ 0 }
		};

		const MemberBitMask::Bit kColorWriteBits[] =
		{
			{ L"red", ColorWrite::Red },
			{ L"green", ColorWrite::Green },
			{ L"blue", ColorWrite::Blue },
			{ L"alpha", ColorWrite::Alpha },
			{ 0 }
		};

		const MemberEnum< StencilOperation >::Key kStencilOperations[] =
		{
			{ L"SoKeep", StencilOperation::Keep },
			{ L"SoZero", StencilOperation::Zero },
			{ L"SoReplace", StencilOperation::Replace },
			{ L"SoIncrementSaturate", StencilOperation::IncrementSaturate },
			{ L"SoDecrementSaturate", StencilOperation::DecrementSaturate },
			{ L"SoInvert", StencilOperation::Invert },
			{ L"SoIncrement", StencilOperation::Increment },
			{ L"SoDecrement", StencilOperation::Decrement },
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

			m_ref.blendAlphaOperation = BlendOperation::Add;
			m_ref.blendAlphaSource = BlendFactor::One;
			m_ref.blendAlphaDestination = BlendFactor::One;
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
			{ L"FtPoint", Filter::Point },
			{ L"FtLinear", Filter::Linear },
			{ 0 }
		};

		const MemberEnum< Address >::Key kAddress[] =
		{
			{ L"AdWrap", Address::Wrap },
			{ L"AdMirror", Address::Mirror },
			{ L"AdClamp", Address::Clamp },
			{ L"AdBorder", Address::Border },
			{ 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompareFunctions[] =
		{
			{ L"CfAlways", CompareFunction::Always },
			{ L"CfNever", CompareFunction::Never },
			{ L"CfLess", CompareFunction::Less },
			{ L"CfLessEqual", CompareFunction::LessEqual },
			{ L"CfGreater", CompareFunction::Greater },
			{ L"CfGreaterEqual", CompareFunction::GreaterEqual },
			{ L"CfEqual", CompareFunction::Equal },
			{ L"CfNotEqual", CompareFunction::NotEqual },
			{ L"CfNone", CompareFunction::None },
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

const ImmutableNode::InputPinDesc c_Abs_i[] =
{
	{ L"Input", L"{7FC3EE08-50EF-41A7-A6EA-94FA670F7607}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Abs_o[] =
{
	{ L"Output", L"{F467883B-A337-4DFB-92CC-FE41A84F5C46}" },
	{ 0 }
};

Abs::Abs()
:	ImmutableNode(c_Abs_i, c_Abs_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Add", 0, Add, ImmutableNode)

const ImmutableNode::InputPinDesc c_Add_i[] =
{
	{ L"Input1", L"{3DE04294-4DEA-4A13-A460-2274647357EA}", false },
	{ L"Input2", L"{9F45B2C3-B513-4646-B0C1-663748FD169C}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Add_o[] =
{
	{ L"Output", L"{32FD3DAA-16C1-44C8-8A1E-E9ECF97F31D2}" },
	{ 0 }
};

Add::Add()
:	ImmutableNode(c_Add_i, c_Add_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ArcusCos", 0, ArcusCos, ImmutableNode)

const ImmutableNode::InputPinDesc c_ArcusCos_i[] =
{
	{ L"Theta", L"{98592661-CD08-47A6-89FC-250FC6060922}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_ArcusCos_o[] =
{
	{ L"Output", L"{BEF8F577-31CA-4C43-9B37-964EC60F06C3}" },
	{ 0 }
};

ArcusCos::ArcusCos()
:	ImmutableNode(c_ArcusCos_i, c_ArcusCos_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ArcusTan", 0, ArcusTan, ImmutableNode)

const ImmutableNode::InputPinDesc c_ArcusTan_i[] =
{
	{ L"XY", L"{C7CA3BF9-EB86-4968-AA77-6F5607DD055C}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_ArcusTan_o[] =
{
	{ L"Output", L"{F5BB1AD0-2D03-495A-83D5-BC17A3E21C55}" },
	{ 0 }
};

ArcusTan::ArcusTan()
:	ImmutableNode(c_ArcusTan_i, c_ArcusTan_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Branch", 0, Branch, ImmutableNode)

const ImmutableNode::InputPinDesc c_Branch_i[] =
{
	{ L"True", L"{A1DDB166-9422-45A3-AE93-6702275DAD1C}", false },
	{ L"False", L"{92AA3735-BB4C-4541-81DA-AC500930B2E6}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Branch_o[] =
{
	{ L"Output", L"{9D9FA2FC-9298-4A6F-88A9-A055F8A91F52}" },
	{ 0 }
};

Branch::Branch(const std::wstring& parameterName)
	: ImmutableNode(c_Branch_i, c_Branch_o)
	, m_parameterName(parameterName)
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.BundleUnite", 0, BundleUnite, Node)

BundleUnite::BundleUnite()
{
	updatePins();
}

int BundleUnite::getInputPinCount() const
{
	return (int)m_inputPins.size();
}

const InputPin* BundleUnite::getInputPin(int index) const
{
	return &m_inputPins[index];
}

int BundleUnite::getOutputPinCount() const
{
	return 1;
}

const OutputPin* BundleUnite::getOutputPin(int index) const
{
	return &m_outputPin;
}

void BundleUnite::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> MemberAlignedVector< std::wstring >(L"names", m_names);

	if (s.getDirection() == ISerializer::Direction::Read)
		updatePins();
}

void BundleUnite::updatePins()
{
	m_inputPins.resize(1 + m_names.size());
	m_inputPins[0] = InputPin(this, Guid(L"{FFD72AB8-3ECF-4EBE-AF54-041F18540D4D}"), L"Input", true);

	Guid id(L"{EEB495BD-DE7F-4DE8-943A-7E9B220B927F}");
	for (int32_t i = 0; i < (int32_t)m_names.size(); ++i)
	{
		m_inputPins[1 + i] = InputPin(this, id, m_names[i], true);
		id.permutate();
	}

	m_outputPin = OutputPin(this, id, L"Output"); id.permutate();
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.BundleSplit", 0, BundleSplit, Node)

BundleSplit::BundleSplit()
{
	updatePins();
}

int BundleSplit::getInputPinCount() const
{
	return 1;
}

const InputPin* BundleSplit::getInputPin(int index) const
{
	return &m_inputPin;
}

int BundleSplit::getOutputPinCount() const
{
	return (int)m_outputPins.size();
}

const OutputPin* BundleSplit::getOutputPin(int index) const
{
	return &m_outputPins[index];
}

void BundleSplit::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> MemberAlignedVector< std::wstring >(L"names", m_names);

	if (s.getDirection() == ISerializer::Direction::Read)
		updatePins();
}

void BundleSplit::updatePins()
{
	Guid id(L"{D596E4E4-1A78-4D80-BC6B-6DEEF934EDEC}");

	m_inputPin = InputPin(this, id, L"Input", false); id.permutate();

	m_outputPins.resize(m_names.size());
	for (int32_t i = 0; i < (int32_t)m_names.size(); ++i)
	{
		m_outputPins[i] = OutputPin(this, id, m_names[i]);
		id.permutate();
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Clamp", 0, Clamp, ImmutableNode)

const ImmutableNode::InputPinDesc c_Clamp_i[] =
{
	{ L"Input", L"{9BA81525-6E53-498C-AA97-B31FB48F3A50}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Clamp_o[] =
{
	{ L"Output", L"{8F2140B5-F4E2-4E8F-8121-13C980250D3C}" },
	{ 0 }
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Color", 2, Color, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Color_o[] =
{
	{ L"Output", L"{4B2822D0-4D9A-4BBB-9956-A996493EE672}" },
	{ 0 }
};

Color::Color(const traktor::Color4f& color)
:	ImmutableNode(nullptr, c_Color_o)
,	m_color(color)
,	m_linear(true)
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

	const float ev = m_color.getEV();

	Color4f c0 = m_color;
	c0.setEV(traktor::Scalar(0.0f));

	const float r = c0.getRed();
	const float g = c0.getGreen();
	const float b = c0.getBlue();
	const float a = c0.getAlpha();

	const int32_t ir = int32_t(r * 255.0f);
	const int32_t ig = int32_t(g * 255.0f);
	const int32_t ib = int32_t(b * 255.0f);
	const int32_t ia = int32_t(a * 255.0f);

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeOutput", 2, ComputeOutput, ImmutableNode)

const ImmutableNode::InputPinDesc c_ComputeOutput_i[] =
{
	{ L"Enable", L"{7DAABFF8-8E54-44AF-B94F-4EE0DAC2C189}", true },
	{ L"Storage", L"{60D25829-A948-4883-B26F-F2BE12B49891}", false },
	{ L"Offset", L"{0907D535-A1B6-409A-A70A-C250D3CDCD58}", false },
	{ L"Input", L"{4356E929-19D3-45DD-8143-0168015DB701}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_ComputeOutput_o[] = { { 0 } };

ComputeOutput::ComputeOutput()
:	ImmutableNode(c_ComputeOutput_i, c_ComputeOutput_o)
,	m_technique(L"Default")
{
	m_localSize[0] = 1;
	m_localSize[1] = 1;
	m_localSize[2] = 1;
}

void ComputeOutput::setTechnique(const std::wstring& technique)
{
	m_technique = technique;
}

const std::wstring& ComputeOutput::getTechnique() const
{
	return m_technique;
}

const int32_t* ComputeOutput::getLocalSize() const
{
	return m_localSize;
}

std::wstring ComputeOutput::getInformation() const
{
	return m_technique;
}

void ComputeOutput::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> Member< std::wstring >(L"technique", m_technique);

	if (s.getVersion< ComputeOutput >() >= 2)
	{
		const wchar_t* c_localSizeElements[] = { L"X", L"Y", L"Z" };
		s >> MemberStaticArray< int32_t, 3 >(L"localSize", m_localSize, AttributeRange(1), c_localSizeElements);
	}
	else if (s.getVersion< ComputeOutput >() == 1)
		s >> MemberStaticArray< int32_t, 3 >(L"localSize", m_localSize, AttributeRange(1));
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Conditional", 1, Conditional, ImmutableNode)

const ImmutableNode::InputPinDesc c_Conditional_i[] =
{
	{ L"Input", L"{70F5921B-7C4D-41E7-88F1-5A71903D2B34}", false },
	{ L"Reference", L"{E66A382E-EDDD-4790-B93E-5CA128757BCC}", false },
	{ L"CaseTrue", L"{6BD21B41-32B3-4296-BE06-B5D7734CA0FB}", false },
	{ L"CaseFalse", L"{6DB78B3B-C272-4243-BCDA-E31CA159D644}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Conditional_o[] =
{
	{ L"Output", L"{E6DBE5B6-98AA-419A-92EA-B7D3E76D207C}" },
	{ 0 }
};

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

const ImmutableNode::InputPinDesc c_Connected_i[] =
{
	{ L"Input", L"{197CC347-3580-4502-8AA1-976E476EA843}", true },
	{ L"True", L"{3F36532A-8A98-4EE7-85ED-2DA35230BC3C}", true },
	{ L"False", L"{5539AA7E-F780-482E-82B8-1E817340D7C0}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Connected_o[] =
{
	{ L"Output", L"{DBDA59DC-83A0-4619-BB51-792A017346DF}" },
	{ 0 }
};

Connected::Connected()
:	ImmutableNode(c_Connected_i, c_Connected_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Cos", 0, Cos, ImmutableNode)

const ImmutableNode::InputPinDesc c_Cos_i[] =
{
	{ L"Theta", L"{47AF9AC9-B428-4DFB-8622-6455FD5B79AC}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Cos_o[] =
{
	{ L"Output", L"{676B22B4-EF0F-461F-9BB7-3ACBB50B7A08}" },
	{ 0 }
};

Cos::Cos()
:	ImmutableNode(c_Cos_i, c_Cos_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Cross", 0, Cross, ImmutableNode)

const ImmutableNode::InputPinDesc c_Cross_i[] =
{
	{ L"Input1", L"{FBA4F44A-9C8C-4AC5-9A2C-1E2D9D410187}", false },
	{ L"Input2", L"{38F0F19A-E863-4D0D-9AA1-72392DF1A076}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Cross_o[] =
{
	{ L"Output", L"{04ABEE84-0049-4827-A742-D44EB2D15560}" },
	{ 0 }
};

Cross::Cross()
:	ImmutableNode(c_Cross_i, c_Cross_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Derivative", 1, Derivative, ImmutableNode)

const ImmutableNode::InputPinDesc c_Derivative_i[] =
{
	{ L"Input", L"{E8138FAF-A686-4404-9A2C-C518185097AD}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Derivative_o[] =
{
	{ L"Output", L"{DBDA9965-A7DF-484F-967C-017DEBFBF42B}" },
	{ 0 }
};

Derivative::Derivative()
:	ImmutableNode(c_Derivative_i, c_Derivative_o)
,	m_axis(Axis::X)
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
	case Axis::X:
		return L"f'(x)";
	case Axis::Y:
		return L"f'(y)";
	}
	return L"";
}

void Derivative::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	if (s.getVersion< Derivative >() >= 1)
	{
		const MemberEnum< Axis >::Key kAxis[] =
		{
			{ L"X", Axis::X },
			{ L"Y", Axis::Y },
			{ 0 }
		};
		s >> MemberEnum< Axis >(L"axis", m_axis, kAxis);
	}
	else
	{
		const MemberEnum< Axis >::Key kAxis[] =
		{
			{ L"DaX", Axis::X },
			{ L"DaY", Axis::Y },
			{ 0 }
		};
		s >> MemberEnum< Axis >(L"axis", m_axis, kAxis);
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Discard", 0, Discard, ImmutableNode)

const ImmutableNode::InputPinDesc c_Discard_i[] =
{
	{ L"Input", L"{5438A442-AF6D-45EB-8163-DF351C6DBF4B}", false },
	{ L"Reference", L"{52792682-A02B-4627-81A2-C83D24BE7143}", false },
	{ L"Pass", L"{2A4105A4-A10B-4669-AE99-DE69647E09D7}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Discard_o[] =
{
	{ L"Output", L"{291502CF-D8D7-4CEC-8457-E867D70CD04E}" },
	{ 0 }
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.DispatchIndex", 1, DispatchIndex, ImmutableNode)

const ImmutableNode::InputPinDesc c_DispatchIndex_i[] = { { 0 } };
const ImmutableNode::OutputPinDesc c_DispatchIndex_o[] =
{
	{ L"Output", L"{AAE82FD3-522F-43C6-A594-2E13D126E5DB}" },
	{ 0 }
};

DispatchIndex::DispatchIndex()
:	ImmutableNode(c_DispatchIndex_i, c_DispatchIndex_o)
{
}

DispatchIndex::Scope DispatchIndex::getScope() const
{
	return m_scope;
}

void DispatchIndex::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	if (s.getVersion< DispatchIndex >() >= 1)
	{
		const MemberEnum< Scope >::Key kScope_Keys[] =
		{
			{ L"Global", Scope::Global },
			{ L"Local", Scope::Local },
			{ L"Group", Scope::Group },
			{ 0 }
		};
		s >> MemberEnum< Scope >(L"scope", m_scope, kScope_Keys);
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Div", 0, Div, ImmutableNode)

const ImmutableNode::InputPinDesc c_Div_i[] =
{
	{ L"Input1", L"{3E538F69-C85C-44E2-9320-73F876288BAE}", false },
	{ L"Input2", L"{C9CC095B-00C6-4A84-88B5-2455115CAF9C}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Div_o[] =
{
	{ L"Output", L"{3B01355D-9153-4864-9B56-7D55F801BFF3}" },
	{ 0 }
};

Div::Div()
:	ImmutableNode(c_Div_i, c_Div_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Dot", 0, Dot, ImmutableNode)

const ImmutableNode::InputPinDesc c_Dot_i[] =
{
	{ L"Input1", L"{403E07EE-E6D1-4863-B70E-CF14F8CD623A}", false },
	{ L"Input2", L"{91730BBC-BBE9-45F0-9C41-31141255F359}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Dot_o[] =
{
	{ L"Output", L"{73A3DE7A-6D35-4844-BF53-55E239F553F1}" },
	{ 0 }
};

Dot::Dot()
:	ImmutableNode(c_Dot_i, c_Dot_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Exp", 0, Exp, ImmutableNode)

const ImmutableNode::InputPinDesc c_Exp_i[] =
{
	{ L"Input", L"{016440FF-B769-41EF-9AD1-C23805A34E5A}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Exp_o[] =
{
	{ L"Output", L"{B5039420-505F-450F-BD34-EB95792428A4}" },
	{ 0 }
};

Exp::Exp()
:	ImmutableNode(c_Exp_i, c_Exp_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Fraction", 0, Fraction, ImmutableNode)

const ImmutableNode::InputPinDesc c_Fraction_i[] =
{
	{ L"Input", L"{D82995D3-DEC5-4361-9C27-E4CE24F6F96F}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Fraction_o[] =
{
	{ L"Output", L"{911D26D6-E535-4D73-983D-A812B4ECEFE9}" },
	{ 0 }
};

Fraction::Fraction()
:	ImmutableNode(c_Fraction_i, c_Fraction_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.FragmentPosition", 0, FragmentPosition, ImmutableNode)

const ImmutableNode::OutputPinDesc c_FragmentPosition_o[] =
{
	{ L"Output", L"{C9223159-9DF2-46A9-B0F2-D0D7D5BEE6F7}" },
	{ 0 }
};

FragmentPosition::FragmentPosition()
:	ImmutableNode(nullptr, c_FragmentPosition_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.FrontFace", 0, FrontFace, ImmutableNode)

const ImmutableNode::OutputPinDesc c_FrontFace_o[] =
{
	{ L"Output", L"{0C479505-83BD-4054-9091-B6A95467B0AC}" },
	{ 0 }
};

FrontFace::FrontFace()
:	ImmutableNode(nullptr, c_FrontFace_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.IndexedUniform", 3, IndexedUniform, ImmutableNode)

const ImmutableNode::InputPinDesc c_IndexedUniform_i[] =
{
	{ L"Index", L"{E457DE92-8BE5-4385-9AD3-3903238A8FD9}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_IndexedUniform_o[] =
{
	{ L"Output", L"{410A240E-17E1-40F0-82FE-BB8ECD086DCA}" },
	{ 0 }
};

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

const ImmutableNode::OutputPinDesc c_InputPort_o[] =
{
	{ L"Output", L"{9AE16BB1-EB68-45A0-A878-8D01C210082B}" },
	{ 0 }
};

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

const ImmutableNode::OutputPinDesc c_Instance_o[] =
{
	{ L"Output", L"{61C214E2-B4D2-4905-A50C-7C38F98E8F91}" },
	{ 0 }
};

Instance::Instance()
:	ImmutableNode(nullptr, c_Instance_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Interpolator", 0, Interpolator, ImmutableNode)

const ImmutableNode::InputPinDesc c_Interpolator_i[] =
{
	{ L"Input", L"{93DEEDC9-D4C7-47F8-8D6A-A79DABD6BA6A}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Interpolator_o[] =
{
	{ L"Output", L"{8FFB3BDB-A00E-4406-994C-0D52FAF04871}" },
	{ 0 }
};

Interpolator::Interpolator()
:	ImmutableNode(c_Interpolator_i, c_Interpolator_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Iterate", 0, Iterate, ImmutableNode)

const ImmutableNode::InputPinDesc c_Iterate_i[] =
{
	{ L"Input", L"{32B75C55-9ABF-43D3-BD90-2759D8BC47E9}", false },
	{ L"Initial", L"{4918677A-523D-4D9C-BA19-798731AC046E}", true },
	{ L"Condition", L"{E37F8315-037A-4BE4-8E5B-5C7647E2117F}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Iterate_o[] =
{
	{ L"N", L"{E6115BB9-BD93-43FB-84A4-9A6A51DE8C27}" },
	{ L"Output", L"{5230F675-56E5-41DD-83FC-7F14F3D661AC}" },
	{ 0 }
};

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

const ImmutableNode::InputPinDesc c_Iterate2_i[] =
{
	{ L"From", L"{185EFF9F-C1A0-4090-B5A5-D1ECB2207C1D}", false },
	{ L"To", L"{1FEC3EA7-3529-4990-B0F5-A13751DF2DBA}", false },
	{ L"Input0", L"{E878AB3C-E58E-48AD-92D5-75176A2DF7F7}", false },
	{ L"Initial0", L"{AFBF2539-CF97-47C0-A9CC-7D5C806E819B}", true },
	{ L"Input1", L"{9DF821A1-CB26-483E-8CC8-C259352111E9}", true },
	{ L"Initial1", L"{90389353-C168-420E-B18A-07FBD721A05D}", true },
	{ L"Input2", L"{140976D7-E8E8-489B-A5E2-E830730A1115}", true },
	{ L"Initial2", L"{68EB5D65-A95C-43A4-8171-D164E03BCA6B}", true },
	{ L"Input3", L"{380153CA-2717-409A-9435-0E1367A62C75}", true },
	{ L"Initial3", L"{D488EE3A-3389-4EAE-A200-DC5AAA049743}", true },
	{ L"Condition", L"{5E04170E-C005-4415-AD07-AEB1A3CEFDE1}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Iterate2_o[] =
{
	{ L"N", L"{36E29D75-02C6-405F-832D-B0A91528A58A}" },
	{ L"Output0", L"{48BD0EF3-A583-439B-84D6-00023498BD11}" },
	{ L"Output1", L"{D1209320-B416-47D5-91BC-128FBB52BF93}" },
	{ L"Output2", L"{C65A8663-8A3F-4F52-9267-022834C88DF7}" },
	{ L"Output3", L"{9C4C1D83-4E9B-4C53-87FF-D4C1A343926C}" },
	{ 0 }
};

Iterate2::Iterate2()
:	ImmutableNode(c_Iterate2_i, c_Iterate2_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Iterate2d", 0, Iterate2d, ImmutableNode)

const ImmutableNode::InputPinDesc c_Iterate2d_i[] =
{
	{ L"Input", L"{416E14C7-223F-4249-8B99-DF2D33E1925F}", false },
	{ L"Initial", L"{F4874E81-9AB1-4AC9-B804-74656FC9B1B5}", true },
	{ L"Condition", L"{C523918D-E9EE-4CF7-AB8E-AF49525FA332}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Iterate2d_o[] =
{
	{ L"X", L"{01C16D15-9227-4FC8-BD1E-E6B55220A259}" },
	{ L"Y", L"{53AF6F56-60DD-453C-A082-85C28D9D7520}" },
	{ L"Output", L"{EB350BD3-62DC-46F1-8339-24858AA9F964}" },
	{ 0 }
};

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

const ImmutableNode::InputPinDesc c_Length_i[] =
{
	{ L"Input", L"{AEFD686C-1E92-44B0-81E9-EA26CA3070F7}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Length_o[] =
{
	{ L"Output", L"{C55FBCBA-5E27-4B38-ACFE-831F8DB3CCA4}" },
	{ 0 }
};

Length::Length()
:	ImmutableNode(c_Length_i, c_Length_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Lerp", 0, Lerp, ImmutableNode)

const ImmutableNode::InputPinDesc c_Lerp_i[] =
{
	{ L"Input1", L"{8760B263-C70C-46BA-9E97-A3D6B08941E9}", false },
	{ L"Input2", L"{7ACEE4C2-A92D-4E50-BFAE-C1D744BEE5E0}", false },
	{ L"Blend", L"{5A25771B-894F-4D92-8D1E-FBF8449850E8}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Lerp_o[] =
{
	{ L"Output", L"{0D78442F-AA88-4B95-A497-E86A86E07FB8}" },
	{ 0 }
};

Lerp::Lerp()
:	ImmutableNode(c_Lerp_i, c_Lerp_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Log", 0, Log, ImmutableNode)

const ImmutableNode::InputPinDesc c_Log_i[] =
{
	{ L"Input", L"{CADFA48F-B72D-4BC8-8CFA-30D367EA2D09}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Log_o[] =
{
	{ L"Output", L"{483BBCAE-725F-44AD-8EBD-96993FB92BA6}" },
	{ 0 }
};

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

const ImmutableNode::InputPinDesc c_MatrixIn_i[] =
{
	{ L"XAxis", L"{06867C72-4921-4EB8-8806-EFCF09BD0C34}", true },
	{ L"YAxis", L"{6A074E66-11A2-4867-A53C-8165AE05CB3B}", true },
	{ L"ZAxis", L"{CB2AD645-8548-45AA-8E8F-FB5226460883}", true },
	{ L"Translate", L"{172A43BF-BADE-41A5-AF3B-3A88FBE9FA80}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_MatrixIn_o[] =
{
	{ L"Output", L"{585541AA-7E7B-44C5-B872-328289FB4854}" },
	{ 0 }
};

MatrixIn::MatrixIn()
:	ImmutableNode(c_MatrixIn_i, c_MatrixIn_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MatrixOut", 0, MatrixOut, ImmutableNode)

const ImmutableNode::InputPinDesc c_MatrixOut_i[] =
{
	{ L"Input", L"{69B552F1-EBCF-4393-8967-DD2A7D9A9C3A}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_MatrixOut_o[] =
{
	{ L"XAxis", L"{9464D788-37CB-49C1-97FA-D5DC48937BE5}" },
	{ L"YAxis", L"{D7BE8909-43BC-49D3-A843-767771142DDD}" },
	{ L"ZAxis", L"{79FBABA2-975A-4D62-81D7-72552A87BF75}" },
	{ L"Translate", L"{516A331D-8EF6-4354-A29B-214078F183FC}" },
	{ 0 }
};

MatrixOut::MatrixOut()
:	ImmutableNode(c_MatrixOut_i, c_MatrixOut_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Max", 0, Max, ImmutableNode)

const ImmutableNode::InputPinDesc c_Max_i[] =
{
	{ L"Input1", L"{1CD3B29D-693D-4635-B49F-EF53300CAA8A}", false },
	{ L"Input2", L"{EF189922-9B95-4572-B8C9-E5BE23606F9C}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Max_o[] =
{
	{ L"Output", L"{D833DB7F-13CB-42D8-8135-48F35EB1288B}" },
	{ 0 }
};

Max::Max()
:	ImmutableNode(c_Max_i, c_Max_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Min", 0, Min, ImmutableNode)

const ImmutableNode::InputPinDesc c_Min_i[] =
{
	{ L"Input1", L"{A638D9DC-801B-4225-BD42-C113E75BFCB7}", false },
	{ L"Input2", L"{18238878-3A1B-43B7-967A-B4BE70B8091E}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Min_o[] =
{
	{ L"Output", L"{03ABBB82-6E2D-4FD8-B171-F14ECC9720DA}" },
	{ 0 }
};

Min::Min()
:	ImmutableNode(c_Min_i, c_Min_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MixIn", 0, MixIn, ImmutableNode)

const ImmutableNode::InputPinDesc c_MixIn_i[] =
{
	{ L"X", L"{AE1EB082-4A05-45B9-BA6A-7F85D78DCA70}", true },
	{ L"Y", L"{64D2F72D-2607-4893-9F25-B1EA45BB8E7B}", true },
	{ L"Z", L"{1FAB08BF-4315-4DDA-99DA-91F76FCA4F12}", true },
	{ L"W", L"{3A4F77F2-38C5-4B62-93EB-267A5D20D28E}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_MixIn_o[] =
{
	{ L"Output", L"{28E6DBB6-876B-4DB2-9A84-4801CAE6A2C2}" },
	{ 0 }
};

MixIn::MixIn()
:	ImmutableNode(c_MixIn_i, c_MixIn_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MixOut", 0, MixOut, ImmutableNode)

const ImmutableNode::InputPinDesc c_MixOut_i[] =
{
	{ L"Input", L"{EBC2324F-DA7F-4CF7-8F4C-55E7FBB5A66E}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_MixOut_o[] =
{
	{ L"X", L"{1C507FD0-1C24-4CD6-AAD5-B10043E8E8F6}" },
	{ L"Y", L"{796E45D7-8C17-4E31-9F8E-ABE972465631}" },
	{ L"Z", L"{A266EE7E-90AE-4423-890F-C1F43D173C78}" },
	{ L"W", L"{42FBDF1A-5EDE-485D-BCD8-BF288A5EE88A}" },
	{ 0 }
};

MixOut::MixOut()
:	ImmutableNode(c_MixOut_i, c_MixOut_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Mul", 0, Mul, ImmutableNode)

const ImmutableNode::InputPinDesc c_Mul_i[] =
{
	{ L"Input1", L"{69997292-C813-490C-910C-620B9AD3A2BB}", false },
	{ L"Input2", L"{D2D716D6-C4A1-471F-894A-D718515F6281}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Mul_o[] =
{
	{ L"Output", L"{9E839249-E9B9-4736-8BDD-A95A1C892B42}" },
	{ 0 }
};

Mul::Mul()
:	ImmutableNode(c_Mul_i, c_Mul_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MulAdd", 0, MulAdd, ImmutableNode)

const ImmutableNode::InputPinDesc c_MulAdd_i[] =
{
	{ L"Input1", L"{4067F6C8-9404-45CA-9359-D9E2456F7431}", false },
	{ L"Input2", L"{F95D9BA4-88CC-4001-9948-B8173FDDE6F0}", false },
	{ L"Input3", L"{567A2DC4-F113-4DE2-917A-791917DA5DA5}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_MulAdd_o[] =
{
	{ L"Output", L"{29BA4386-B838-4550-93DF-F53D8B812C88}" },
	{ 0 }
};

MulAdd::MulAdd()
:	ImmutableNode(c_MulAdd_i, c_MulAdd_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Neg", 0, Neg, ImmutableNode)

const ImmutableNode::InputPinDesc c_Neg_i[] =
{
	{ L"Input", L"{98BF7CC4-A80C-43DA-B7FD-21BA4C039E28}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Neg_o[] =
{
	{ L"Output", L"{D74E3627-9E70-4BAD-A6CE-2087D41D6ED7}" },
	{ 0 }
};

Neg::Neg()
:	ImmutableNode(c_Neg_i, c_Neg_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Normalize", 0, Normalize, ImmutableNode)

const ImmutableNode::InputPinDesc c_Normalize_i[] =
{
	{ L"Input", L"{FE413452-19EB-4DC9-B724-2984FA17CC20}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Normalize_o[] =
{
	{ L"Output", L"{8591EE56-8A3E-41CF-8E98-8F8DB26AABDC}" },
	{ 0 }
};

Normalize::Normalize()
:	ImmutableNode(c_Normalize_i, c_Normalize_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.OutputPort", 0, OutputPort, ImmutableNode)

const ImmutableNode::InputPinDesc c_OutputPort_i[] =
{
	{ L"Input", L"{731844D4-AFDC-4EAA-8B41-C4BA2455898F}", false },
	{ 0 }
};

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
	{ L"Android", L"{F9FF9BED-2CEE-4116-A6D3-3E03D2C3B91C}", true },
	{ L"iOS", L"{352EC3DB-B30A-4686-ADF4-4812E6550789}", true },
	{ L"Linux", L"{84B13963-5727-4B25-BACD-515545C763AA}", true },
	{ L"macOS", L"{857F1D67-2B8E-4607-92DF-326E9682C7DF}", true },
	{ L"RaspberryPI", L"{E4CD016C-B1C7-424C-8428-55AF0FAA4A27}", true },
	{ L"Windows", L"{3B3811D6-891D-4D1F-82A7-D5A23B406B88}", true },
	{ L"Other", L"{AD09EB97-E7F6-45B5-8FCC-4FA391F0738A}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Platform_o[] =
{
	{ L"Output", L"{DF6B63C8-1558-4EDE-8619-C3C0D1BB38BA}" },
	{ 0 }
};

Platform::Platform()
:	ImmutableNode(c_Platform_i, c_Platform_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PixelOutput", 10, PixelOutput, ImmutableNode)

const ImmutableNode::InputPinDesc c_PixelOutput_i[] =
{
	{ L"Enable", L"{185C5A67-11F2-4641-B007-2EA8FCB020D7}", true },
	{ L"Input", L"{7174FE5A-D079-4452-AFEF-9FC2BCC2900F}", false },
	{ L"Input1", L"{1BE35A74-64EA-4C68-926E-B66760B718DB}", true },
	{ L"Input2", L"{6642BD4D-39D8-4AF4-A31B-5D7A491E0C48}", true },
	{ L"Input3", L"{1EB6B476-B7FE-48E3-A12E-EE40F79FEBE0}", true },
	{ L"State", L"{BEF751FF-A4A9-4FCA-B177-1FB2122E50F6}", true },
	{ 0 }
};

PixelOutput::PixelOutput()
:	ImmutableNode(c_PixelOutput_i, nullptr)
,	m_technique(L"Default")
,	m_priority(0)
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
			{ L"setup", RenderPriority::Setup },
			{ L"opaque", RenderPriority::Opaque },
			{ L"postOpaque", RenderPriority::PostOpaque },
			{ L"alphaBlend", RenderPriority::AlphaBlend },
			{ L"postAlphaBlend", RenderPriority::PostAlphaBlend },
			{ L"overlay", RenderPriority::Overlay },
			{ 0 }
		};
		s >> MemberBitMask(L"priority", m_priority, c_RenderPriorityBits);
	}

	s >> MemberRenderState(m_renderState, s.getVersion());

	if (s.getVersion() >= 3 && s.getVersion() < 10)
		s >> ObsoleteMember< uint32_t >(L"registerCount");

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

const ImmutableNode::OutputPinDesc c_PixelState_o[] =
{
	{ L"Output", L"{D6BD4EC1-DD7F-4CA0-83F5-231EED3C487B}" },
	{ 0 }
};

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
			{ L"setup", RenderPriority::Setup },
			{ L"opaque", RenderPriority::Opaque },
			{ L"postOpaque", RenderPriority::PostOpaque },
			{ L"alphaBlend", RenderPriority::AlphaBlend },
			{ L"postAlphaBlend", RenderPriority::PostAlphaBlend },
			{ L"overlay", RenderPriority::Overlay },
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.PreviewOutput", 0, PreviewOutput, ImmutableNode)

const ImmutableNode::InputPinDesc c_PreviewOutput_i[] =
{
	{ L"Input", L"{BDB32F7B-599C-42C8-A2CE-2B1BD0738E8B}", false },
	{ 0 }
};

PreviewOutput::PreviewOutput()
:	ImmutableNode(c_PreviewOutput_i, 0)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Polynomial", 0, Polynomial, ImmutableNode)

const ImmutableNode::InputPinDesc c_Polynomial_i[] =
{
	{ L"X", L"{186E5F99-35AD-4FEA-8D90-36BE6F62E149}", false },
	{ L"Coefficients", L"{E3096274-67F2-4DB8-A985-0F2449175EE6}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Polynomial_o[] =
{
	{ L"Output", L"{45163CD3-DB70-46F4-B195-695CC94BE2BE}" },
	{ 0 }
};

Polynomial::Polynomial()
:	ImmutableNode(c_Polynomial_i, c_Polynomial_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Pow", 0, Pow, ImmutableNode)

const ImmutableNode::InputPinDesc c_Pow_i[] =
{
	{ L"Exponent", L"{1A674CC4-1D87-4859-AB19-4DDD06A12987}", false },
	{ L"Input", L"{E96D386C-3A96-4807-BFC9-50180678A096}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Pow_o[] =
{
	{ L"Output", L"{B4FEA355-7A63-498C-966D-42F58570CB20}" },
	{ 0 }
};

Pow::Pow()
:	ImmutableNode(c_Pow_i, c_Pow_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ReadStruct", 0, ReadStruct, ImmutableNode)

const ImmutableNode::InputPinDesc c_ReadStruct_i[] =
{
	{ L"Struct", L"{3B445686-9AD9-4A7E-8C09-C92EDB98EFEF}", false },
	{ L"Index", L"{4FD10E49-F598-45AB-9E40-5ABA13905D14}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_ReadStruct_o[] =
{
	{ L"Output", L"{DD079887-A171-407B-AD10-E0CE5B164BC1}" },
	{ 0 }
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ReadStruct2", 0, ReadStruct2, Node)

ReadStruct2::ReadStruct2()
{
	updatePins();
}

int ReadStruct2::getInputPinCount() const
{
	return (int)m_inputPins.size();
}

const InputPin* ReadStruct2::getInputPin(int index) const
{
	return &m_inputPins[index];
}

int ReadStruct2::getOutputPinCount() const
{
	return (int)m_outputPins.size();
}

const OutputPin* ReadStruct2::getOutputPin(int index) const
{
	return &m_outputPins[index];
}

void ReadStruct2::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> MemberAlignedVector< std::wstring >(L"names", m_names);

	if (s.getDirection() == ISerializer::Direction::Read)
		updatePins();
}

void ReadStruct2::updatePins()
{
	Guid id(L"{0FF6511C-0293-41A8-830E-81978BD01F7F}");

	m_inputPins.resize(2);
	m_inputPins[0] = InputPin(this, id, L"Struct", false); id.permutate();
	m_inputPins[1] = InputPin(this, id, L"Index", false); id.permutate();

	m_outputPins.resize(m_names.size());
	for (int32_t i = 0; i < (int32_t)m_names.size(); ++i)
	{
		m_outputPins[i] = OutputPin(this, id, m_names[i]);
		id.permutate();
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RecipSqrt", 0, RecipSqrt, ImmutableNode)

const ImmutableNode::InputPinDesc c_RecipSqrt_i[] =
{
	{ L"Input", L"{771D8BDC-E118-418D-9CA2-43A8B71F9D79}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_RecipSqrt_o[] =
{
	{ L"Output", L"{66F22047-5285-4274-A3D8-27614185E30C}" },
	{ 0 }
};

RecipSqrt::RecipSqrt()
:	ImmutableNode(c_RecipSqrt_i, c_RecipSqrt_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Renderer", 0, Renderer, ImmutableNode)

const ImmutableNode::InputPinDesc c_Renderer_i[] =
{
	{ L"Vulkan", L"{26C3B4A1-CBBC-4F07-85D2-D493F40C212A}", true },
	{ L"Other", L"{37801E29-B776-42E5-A71E-C44FBDD19992}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Renderer_o[] =
{
	{ L"Output", L"{C7F0969C-A9E9-4C5A-B110-78095B267380}" },
	{ 0 }
};

Renderer::Renderer()
:	ImmutableNode(c_Renderer_i, c_Renderer_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Reflect", 0, Reflect, ImmutableNode)

const ImmutableNode::InputPinDesc c_Reflect_i[] =
{
	{ L"Normal", L"{24E97134-2FDB-4E95-B7D6-EF81F3E17837}", false },
	{ L"Direction", L"{0AAE2EAE-B01B-425A-A8C3-539576B45ECC}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Reflect_o[] =
{
	{ L"Output", L"{15053FB1-8C13-4C18-8958-193FEDDCFB11}" },
	{ 0 }
};

Reflect::Reflect()
:	ImmutableNode(c_Reflect_i, c_Reflect_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Repeat", 0, Repeat, ImmutableNode)

const ImmutableNode::InputPinDesc c_Repeat_i[] =
{
	{ L"Input", L"{D4105C5D-982B-4ECA-A9F2-1F62797D1515}", false },
	{ L"Initial", L"{FD0BDE23-8D5C-464A-BD43-18A47E64FF19}", true },
	{ L"Condition", L"{BE0520F2-C5BC-4781-B558-025B5839BD45}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Repeat_o[] =
{
	{ L"N", L"{F8D8BDE3-2059-4B35-A616-2127FF64F3C4}" },
	{ L"Output", L"{E755A1B9-CA5A-4B85-B5C5-32D817195D0B}" },
	{ 0 }
};

Repeat::Repeat()
:	ImmutableNode(c_Repeat_i, c_Repeat_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Round", 0, Round, ImmutableNode)

const ImmutableNode::InputPinDesc c_Round_i[] =
{
	{ L"Input", L"{FFCC8EF4-A7AD-4D1A-8F77-0E95961D76A3}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Round_o[] =
{
	{ L"Output", L"{1B0067AD-7ED9-423E-A04D-E6DCBED62481}" },
	{ 0 }
};

Round::Round()
:	ImmutableNode(c_Round_i, c_Round_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sampler", 5, Sampler, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sampler_i[] =
{
	{ L"Texture", L"{32EB5230-1F0D-40B8-93F6-9C8E5469454E}", false },
	{ L"TexCoord", L"{6D5C5EFE-A35C-4748-B81E-B8EBACE433BC}", false },
	{ L"Mip", L"{A790CEB7-0729-490D-94B0-4D7982C139F9}", true },
	{ L"State", L"{10F0235A-9174-4DB8-978C-FC55A07C94A2}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Sampler_o[] =
{
	{ L"Output", L"{AF6C971B-D67A-42EE-A12B-97D36927C89F}" },
	{ 0 }
};

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
			{ L"FtPoint", Filter::Point },
			{ L"FtLinear", Filter::Linear },
			{ 0 }
		};

		const MemberEnum< Address >::Key kAddress[] =
		{
			{ L"AdWrap", Address::Wrap },
			{ L"AdMirror", Address::Mirror },
			{ L"AdClamp", Address::Clamp },
			{ L"AdBorder", Address::Border },
			{ 0 }
		};

		const MemberEnum< CompareFunction >::Key kCompare[] =
		{
			{ L"CmNo", CompareFunction::None },
			{ L"CmAlways", CompareFunction::Always },
			{ L"CmNever", CompareFunction::Never },
			{ L"CmLess", CompareFunction::Less },
			{ L"CmLessEqual", CompareFunction::LessEqual },
			{ L"CmGreater", CompareFunction::Greater },
			{ L"CmGreaterEqual", CompareFunction::GreaterEqual },
			{ L"CmEqual", CompareFunction::Equal },
			{ L"CmNotEqual", CompareFunction::NotEqual },
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

const ImmutableNode::OutputPinDesc c_Scalar_o[] =
{
	{ L"Output", L"{D33F8931-C90C-4EBA-8A04-A31D3E08FAB7}" },
	{ 0 }
};

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

const ImmutableNode::InputPinDesc c_Sign_i[] =
{
	{ L"Input", L"{8EC793C5-36F2-4490-8A5C-E1084D1EC400}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Sign_o[] =
{
	{ L"Output", L"{408B350C-B173-4C75-83FD-64C3AA70CF77}" },
	{ 0 }
};

Sign::Sign()
:	ImmutableNode(c_Sign_i, c_Sign_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sin", 0, Sin, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sin_i[] =
{
	{ L"Theta", L"{B3BDF230-63F4-4908-A76B-7ACE2D548C3E}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Sin_o[] =
{
	{ L"Output", L"{6CCEDC7B-43DA-48EA-B342-0F628A3A4BD8}" },
	{ 0 }
};

Sin::Sin()
:	ImmutableNode(c_Sin_i, c_Sin_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sqrt", 0, Sqrt, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sqrt_i[] =
{
	{ L"Input", L"{B60C292C-649F-48CF-A344-914B0831FE04}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Sqrt_o[] =
{
	{ L"Output", L"{1C843C59-B7CA-415B-A537-D8E2C2B0ED10}" },
	{ 0 }
};

Sqrt::Sqrt()
:	ImmutableNode(c_Sqrt_i, c_Sqrt_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Step", 0, Step, ImmutableNode)

const ImmutableNode::InputPinDesc c_Step_i[] =
{
	{ L"Edge", L"{7CEDA35F-46C0-465D-9C5D-EABAB3FF5838}", false },
	{ L"X", L"{4853FA43-A9C2-4427-9B87-9E372EC50182}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Step_o[] =
{
	{ L"Output", L"{DB775C57-E48D-484B-B438-6EAA9A69B1CC}" },
	{ 0 }
};

Step::Step()
:	ImmutableNode(c_Step_i, c_Step_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Struct", 0, Struct, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Struct_o[] =
{
	{ L"Output", L"{ACC77B35-91B5-4405-ABC8-D0DA24D68178}" },
	{ 0 }
};

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

const ImmutableNode::InputPinDesc c_Sub_i[] =
{
	{ L"Input1", L"{AA571ACC-7699-4D10-BCD7-4E857EFB35EA}", false },
	{ L"Input2", L"{8C41D88A-1D17-4237-B720-CCC0B7FF71B9}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Sub_o[] =
{
	{ L"Output", L"{2D90AB77-694C-4586-AA05-5CF062EFFFAB}" },
	{ 0 }
};

Sub::Sub()
:	ImmutableNode(c_Sub_i, c_Sub_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Sum", 0, Sum, ImmutableNode)

const ImmutableNode::InputPinDesc c_Sum_i[] =
{
	{ L"Input", L"{BBF4CC03-E1C6-4A44-B665-EA0C44E7C44C}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Sum_o[] =
{
	{ L"N", L"{688F879C-3CC1-436C-AF6D-11188221D18F}" },
	{ L"Output", L"{B90DEAE3-E53C-4925-B4E8-5DF015400991}" },
	{ 0 }
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Switch", 3, Switch, Node)

Switch::Switch()
:	m_branch(Branch::Auto)
,	m_width(1)
{
	updatePins();
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
	return &m_inputPins[index];
}

int Switch::getOutputPinCount() const
{
	return m_width;
}

const OutputPin* Switch::getOutputPin(int index) const
{
	return &m_outputPins[index];
}

void Switch::serialize(ISerializer& s)
{
	Node::serialize(s);

	if (s.getVersion< Switch >() >= 3)
	{
		const MemberEnum< Branch >::Key kBranch[] =
		{
			{ L"Auto", Branch::Auto },
			{ L"Static", Branch::Static },
			{ L"Dynamic", Branch::Dynamic },
			{ 0 }
		};
		s >> MemberEnum< Branch >(L"branch", m_branch, kBranch);
	}
	else if (s.getVersion< Switch >() >= 1)
	{
		const MemberEnum< Branch >::Key kBranch[] =
		{
			{ L"BrAuto", Branch::Auto },
			{ L"BrStatic", Branch::Static },
			{ L"BrDynamic", Branch::Dynamic },
			{ 0 }
		};
		s >> MemberEnum< Branch >(L"branch", m_branch, kBranch);
	}

	if (s.getVersion< Switch >() >= 2)
		s >> Member< int32_t >(L"width", m_width, AttributeRange(1));

	s >> MemberAlignedVector< int32_t >(L"cases", m_cases);

	if (s.getDirection() == ISerializer::Direction::Read)
		updatePins();
}

void Switch::updatePins()
{
	Guid id(L"{3BFF07B7-0B69-42D7-8BD0-0F315B985C8E}");

	m_inputPins.resize(1 + (1 + m_cases.size()) * m_width);
	m_inputPins[0] = InputPin(this, id, L"Select", false);
	id.permutate();
	for (int32_t c = 0; c < m_width; ++c)
	{
		StringOutputStream ss;
		ss << L"Default";
		if (m_width > 1)
			ss << L" (" << c << L")";
		m_inputPins[1 + c] = InputPin(this, id, ss.str(), false);
		id.permutate();
	}
	for (uint32_t i = 0; i < (uint32_t)m_cases.size(); ++i)
	{
		for (int32_t c = 0; c < m_width; ++c)
		{
			StringOutputStream ss;
			ss << L"Case " << m_cases[i];
			if (m_width > 1)
				ss << L" (" << c << L")";
			m_inputPins[1 + (1 + i) * m_width + c] = InputPin(this, id, ss.str(), false);
			id.permutate();
		}
	}

	m_outputPins.resize(m_width);
	for (int32_t c = 0; c < m_width; ++c)
	{
		StringOutputStream ss;
		ss << L"Output";
		if (m_width > 1)
			ss << L" (" << c << L")";
		m_outputPins[c] = OutputPin(this, id, ss.str());
		id.permutate();
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Swizzle", 0, Swizzle, ImmutableNode)

const ImmutableNode::InputPinDesc c_Swizzle_i[] =
{
	{ L"Input", L"{F2E22CA6-DFF3-4B20-A70A-0D7A44EACD8C}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Swizzle_o[] =
{
	{ L"Output", L"{ADB4FC1D-3726-4CC5-B4D5-1E2468274325}" },
	{ 0 }
};

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

const ImmutableNode::InputPinDesc c_Tan_i[] =
{
	{ L"Theta", L"{D9CF48DF-27F8-4F5D-8C90-EAE28F19D757}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Tan_o[] =
{
	{ L"Output", L"{3C10ABA8-AB56-410A-AEB4-89D1DC0B7755}" },
	{ 0 }
};

Tan::Tan()
:	ImmutableNode(c_Tan_i, c_Tan_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TargetSize", 0, TargetSize, ImmutableNode)

const ImmutableNode::InputPinDesc c_TargetSize_i[] = { { 0 } };
const ImmutableNode::OutputPinDesc c_TargetSize_o[] =
{
	{ L"Output", L"{853B6F75-1464-40E7-BEDC-2716C763046E}" },
	{ 0 }
};

TargetSize::TargetSize()
:	ImmutableNode(c_TargetSize_i, c_TargetSize_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Texture", 1, Texture, ImmutableNode)

const ImmutableNode::InputPinDesc c_Texture_i[] = { { 0 } };
const ImmutableNode::OutputPinDesc c_Texture_o[] =
{
	{ L"Output", L"{FFE19E4E-24A8-47A4-AE80-307495E31066}" },
	{ 0 }
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureState", 0, TextureState, ImmutableNode)

const ImmutableNode::OutputPinDesc c_TextureState_o[] =
{
	{ L"Output", L"{82C966B2-7B19-48B2-8FE0-B85FF4E3C504}" },
	{ 0 }
};

TextureState::TextureState()
:	ImmutableNode(nullptr, c_TextureState_o)
{
}

void TextureState::setSamplerState(const SamplerState& state)
{
	m_samplerState = state;
}

const SamplerState& TextureState::getSamplerState() const
{
	return m_samplerState;
}

void TextureState::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);
	s >> MemberSamplerState(m_samplerState);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureSize", 0, TextureSize, ImmutableNode)

const ImmutableNode::InputPinDesc c_TextureSize_i[] =
{
	{ L"Input", L"{E2A0DADF-10C2-4699-8EA4-78AC796C5158}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_TextureSize_o[] =
{
	{ L"Output", L"{C6373DB1-EC15-47B5-A4B9-D301E446C95A}" },
	{ 0 }
};

TextureSize::TextureSize()
:	ImmutableNode(c_TextureSize_i, c_TextureSize_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Transform", 0, Transform, ImmutableNode)

const ImmutableNode::InputPinDesc c_Transform_i[] =
{
	{ L"Input", L"{345BDBFA-3326-40BF-B9DE-8ECA78A3EEF4}", false },
	{ L"Transform", L"{71A0310C-3928-44C3-8D4F-7D9AFF9EAE70}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Transform_o[] =
{
	{ L"Output", L"{70C35B91-1C36-454C-98EC-7514AACF73C8}" },
	{ 0 }
};

Transform::Transform()
:	ImmutableNode(c_Transform_i, c_Transform_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Transpose", 0, Transpose, ImmutableNode)

const ImmutableNode::InputPinDesc c_Transpose_i[] =
{
	{ L"Input", L"{CD126085-5A74-404A-A1DB-A01F2380B427}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Transpose_o[] =
{
	{ L"Output", L"{2AB76522-5A35-44B4-B3E9-BEB766EA4A23}" },
	{ 0 }
};

Transpose::Transpose()
:	ImmutableNode(c_Transpose_i, c_Transpose_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Truncate", 0, Truncate, ImmutableNode)

const ImmutableNode::InputPinDesc c_Truncate_i[] =
{
	{ L"Input", L"{58E01BB0-182E-4FF2-8629-A1EA043A57F1}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Truncate_o[] =
{
	{ L"Output", L"{7E002805-FA4B-4EEE-9D3B-6FA3AB0B2DAB}" },
	{ 0 }
};

Truncate::Truncate()
:	ImmutableNode(c_Truncate_i, c_Truncate_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Type", 0, Type, ImmutableNode)

const ImmutableNode::InputPinDesc c_Type_i[] =
{
	{ L"Type", L"{324A6471-CD60-4928-A4E2-2B085856B91C}", true },
	{ L"Scalar", L"{EF748FE9-7B56-4008-AA99-483F5AAD8F91}", true },
	{ L"Vector", L"{C458F1E6-5144-4289-ADF4-C07453D38038}", true },
	{ L"Matrix", L"{1D1C012E-A545-4858-9BB2-B989CE01ED41}", true },
	{ L"Texture", L"{E830E538-4DEA-4402-9C34-E9BAF5DD2393}", true },
	{ L"State", L"{4E32E8F5-1C4E-4A13-82C3-B32D711490F4}", true },
	{ L"Default", L"{1B4A2BC3-42EE-4852-B198-6390A449A0F5}", false },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Type_o[] =
{
	{ L"Output", L"{4EE768A0-1734-4FC2-B15E-6CD78C14933C}" },
	{ 0 }
};

Type::Type()
:	ImmutableNode(c_Type_i, c_Type_o)
{
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Uniform", 3, Uniform, ImmutableNode)

const ImmutableNode::InputPinDesc c_Uniform_i[] =
{
	{ L"Initial", L"{CDD51665-CD39-444D-8D5C-5FD37C1646B7}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Uniform_o[] =
{
	{ L"Output", L"{1E6639B6-8B58-4694-99E7-C058E3583522}" },
	{ 0 }
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Variable", 2, Variable, ImmutableNode)

const ImmutableNode::InputPinDesc c_Variable_i[] =
{
	{ L"Input", L"{11585EBC-914D-4E6D-A10D-D01694FF9840}", true },
	{ 0 }
};
const ImmutableNode::OutputPinDesc c_Variable_o[] =
{
	{ L"Output", L"{84BF3C26-64A7-4032-B775-1D369052B243}" },
	{ 0 }
};

Variable::Variable()
:	ImmutableNode(c_Variable_i, c_Variable_o)
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

std::wstring Variable::getInformation() const
{
	return m_name;
}

void Variable::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion< Variable >() == 1)
	{
		bool global;
		s >> Member< bool >(L"global", global);
	}
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Vector", 0, Vector, ImmutableNode)

const ImmutableNode::OutputPinDesc c_Vector_o[] =
{
	{ L"Output", L"{D51E6077-37E9-4B1C-86D2-92DA544DC613}" },
	{ 0 }
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.VertexInput", 1, VertexInput, ImmutableNode)

const ImmutableNode::OutputPinDesc c_VertexInput_o[] =
{
	{ L"Output", L"{BD91C9E9-9950-4EC8-BAD2-60D2E8699107}" },
	{ 0 }
};

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
	
	if (s.getVersion< VertexInput >() >= 1)
	{
		const MemberEnum< DataUsage >::Key kDataUsage[] =
		{
			{ L"Position", DataUsage::Position },
			{ L"Normal", DataUsage::Normal },
			{ L"Tangent", DataUsage::Tangent },
			{ L"Binormal", DataUsage::Binormal },
			{ L"Color", DataUsage::Color },
			{ L"Custom", DataUsage::Custom },
			{ 0 }
		};
		s >> MemberEnum< DataUsage >(L"usage", m_usage, kDataUsage);
	}
	else
	{
		const MemberEnum< DataUsage >::Key kDataUsage[] =
		{
			{ L"DuPosition", DataUsage::Position },
			{ L"DuNormal", DataUsage::Normal },
			{ L"DuTangent", DataUsage::Tangent },
			{ L"DuBinormal", DataUsage::Binormal },
			{ L"DuColor", DataUsage::Color },
			{ L"DuCustom", DataUsage::Custom },
			{ 0 }
		};
		s >> MemberEnum< DataUsage >(L"usage", m_usage, kDataUsage);
	}

	s >> MemberEnum< DataType >(L"type", m_type, kDataType);
	s >> Member< int32_t >(L"index", m_index);
}

/*---------------------------------------------------------------------------*/

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.VertexOutput", 3, VertexOutput, ImmutableNode)

const ImmutableNode::InputPinDesc c_VertexOutput_i[] =
{
	{ L"Input", L"{BFBE8191-F6E6-4A4F-A2CC-6CBC1D19BF70}", false },
	{ 0 }
};

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
