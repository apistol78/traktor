/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Image2/ImgTargetSet.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

constexpr static MemberEnum< TextureFormat >::Key c_TextureFormat_Keys[] =
{
	{ L"TfInvalid", TfInvalid },
	{ L"TfR8", TfR8 },
	{ L"TfR8G8B8A8", TfR8G8B8A8 },
	{ L"TfR5G6B5", TfR5G6B5 },
	{ L"TfR5G5B5A1", TfR5G5B5A1 },
	{ L"TfR4G4B4A4", TfR4G4B4A4 },
	{ L"TfR16G16B16A16F", TfR16G16B16A16F },
	{ L"TfR32G32B32A32F", TfR32G32B32A32F },
	{ L"TfR16G16F", TfR16G16F },
	{ L"TfR32G32F", TfR32G32F },
	{ L"TfR16F", TfR16F },
	{ L"TfR32F", TfR32F },
	{ L"TfR11G11B10F", TfR11G11B10F },
	{ L"TfDXT1", TfDXT1 },
	{ L"TfDXT2", TfDXT2 },
	{ L"TfDXT3", TfDXT3 },
	{ L"TfDXT4", TfDXT4 },
	{ L"TfDXT5", TfDXT5 },
	{ L"TfPVRTC1", TfPVRTC1 },
	{ L"TfPVRTC2", TfPVRTC2 },
	{ L"TfPVRTC3", TfPVRTC3 },
	{ L"TfPVRTC4", TfPVRTC4 },
	{ L"TfETC1", TfETC1 },
	{ 0 }
};

class MemberTextureFormat : public MemberEnum< TextureFormat >
{
public:
	MemberTextureFormat(const wchar_t* const name, TextureFormat& en)
	:   MemberEnum< TextureFormat >(name, en, c_TextureFormat_Keys)
	{
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgTargetSet", 1, ImgTargetSet, Node)

ImgTargetSet::ImgTargetSet()
:	m_persistent(false)
,	m_width(0)
,	m_height(0)
,	m_screenWidthDenom(0)
,	m_screenHeightDenom(0)
,	m_maxWidth(0)
,	m_maxHeight(0)
,	m_createDepthStencil(false)
,	m_ignoreStencil(false)
,	m_generateMips(false)
{
	const Guid c_null;
	m_inputPins.push_back(new InputPin(this, c_null, L"Input", false));
}

ImgTargetSet::~ImgTargetSet()
{
	for (auto& inputPin : m_inputPins)
		delete inputPin;
	for (auto& outputPin : m_outputPins)
		delete outputPin;
}

const std::wstring& ImgTargetSet::getTargetSetId() const
{
	return m_targetSetId;
}

bool ImgTargetSet::getPersistent() const
{
	return m_persistent;
}

int32_t ImgTargetSet::getTextureCount() const
{
	return (int32_t)m_targets.size();
}

const std::wstring& ImgTargetSet::getTextureId(int32_t colorIndex) const
{
	return m_targets[colorIndex].textureId;
}

RenderGraphTargetSetDesc ImgTargetSet::getRenderGraphTargetSetDesc() const
{
	RenderGraphTargetSetDesc desc;

	desc.count = (int32_t)m_targets.size();
	desc.width = m_width;
	desc.height = m_height;
	desc.referenceWidthDenom = m_screenWidthDenom;
	desc.referenceHeightDenom = m_screenHeightDenom;
	desc.maxWidth = m_maxWidth;
	desc.maxHeight = m_maxHeight;
	desc.createDepthStencil = m_createDepthStencil;
	desc.usingPrimaryDepthStencil = false;
	desc.usingDepthStencilAsTexture = false;
	desc.ignoreStencil = m_ignoreStencil;
	desc.generateMips = m_generateMips;
	
	for (int32_t i = 0; i < (int32_t)m_targets.size(); ++i)
		desc.targets[i].colorFormat = m_targets[i].colorFormat;

	return desc;
}

int ImgTargetSet::getInputPinCount() const
{
	return (int)m_inputPins.size();
}

const InputPin* ImgTargetSet::getInputPin(int index) const
{
	T_ASSERT(index >= 0 && index < (int)m_inputPins.size());
	return m_inputPins[index];
}

int ImgTargetSet::getOutputPinCount() const
{
	return (int)m_outputPins.size();
}

const OutputPin* ImgTargetSet::getOutputPin(int index) const
{
	T_ASSERT(index >= 0 && index < (int)m_outputPins.size());
	return m_outputPins[index];
}

void ImgTargetSet::serialize(ISerializer& s)
{
	Node::serialize(s);
	
	s >> Member< std::wstring >(L"targetSetId", m_targetSetId);

	if (s.getVersion< ImgTargetSet >() >= 1)
		s >> Member< bool >(L"persistent", m_persistent);

	s >> Member< int32_t >(L"width", m_width);
	s >> Member< int32_t >(L"height", m_height);
	s >> Member< int32_t >(L"screenWidthDenom", m_screenWidthDenom);
	s >> Member< int32_t >(L"screenHeightDenom", m_screenHeightDenom);
	s >> Member< int32_t >(L"maxWidth", m_maxWidth);
	s >> Member< int32_t >(L"maxHeight", m_maxHeight);
	s >> Member< bool >(L"createDepthStencil", m_createDepthStencil);
	s >> Member< bool >(L"ignoreStencil", m_ignoreStencil);
	s >> Member< bool >(L"generateMips", m_generateMips);
	s >> MemberAlignedVector< TargetDesc, MemberComposite< TargetDesc > >(L"targets", m_targets);

	if (s.getDirection() == ISerializer::Direction::Read)
		refresh();
}

void ImgTargetSet::refresh()
{
	const Guid c_null;

	for (auto& outputPin : m_outputPins)
		delete outputPin;

	m_outputPins.clear();

	for (const auto& target : m_targets)
		m_outputPins.push_back(new OutputPin(this, c_null, target.textureId));
}

ImgTargetSet::TargetDesc::TargetDesc()
:	colorFormat(TfInvalid)
{
}

void ImgTargetSet::TargetDesc::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"textureId", textureId);
	s >> MemberTextureFormat(L"colorFormat", colorFormat);
}

	}
}