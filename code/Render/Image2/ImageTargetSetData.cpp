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
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/Image2/ImageTargetSet.h"
#include "Render/Image2/ImageTargetSetData.h"

namespace traktor::render
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

class MemberRenderGraphTargetDesc : public MemberComplex
{
public:
	typedef RenderGraphTargetDesc value_type;

	MemberRenderGraphTargetDesc(const wchar_t* const name, value_type& ref)
	:   MemberComplex(name, true)
	,   m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> MemberTextureFormat(L"colorFormat", m_ref.colorFormat);
	}

private:
	value_type& m_ref;
};

class MemberRenderGraphTargetSetDesc : public MemberComplex
{
public:
	typedef RenderGraphTargetSetDesc value_type;

	MemberRenderGraphTargetSetDesc(const wchar_t* const name, value_type& ref)
	:   MemberComplex(name, true)
	,   m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< int32_t >(L"count", m_ref.count);
		s >> Member< int32_t >(L"width", m_ref.width);
		s >> Member< int32_t >(L"height", m_ref.height);
		if (s.getVersion() >= 1)
		{
			s >> Member< int32_t >(L"referenceWidthDenom", m_ref.referenceWidthDenom);
			s >> Member< int32_t >(L"referenceHeightDenom", m_ref.referenceHeightDenom);
		}
		else
		{
			s >> Member< int32_t >(L"screenWidthDenom", m_ref.referenceWidthDenom);
			s >> Member< int32_t >(L"screenHeightDenom", m_ref.referenceHeightDenom);
		}
		s >> Member< int32_t >(L"maxWidth", m_ref.maxWidth);
		s >> Member< int32_t >(L"maxHeight", m_ref.maxHeight);
		s >> Member< bool >(L"createDepthStencil", m_ref.createDepthStencil);
		s >> Member< bool >(L"usingDepthStencilAsTexture", m_ref.usingDepthStencilAsTexture);
		s >> Member< bool >(L"generateMips", m_ref.generateMips);
		s >> MemberStaticArray< RenderGraphTargetDesc, RenderGraphTargetSetDesc::MaxColorTargets, MemberRenderGraphTargetDesc >(L"targets", m_ref.targets);
	}

private:
	value_type& m_ref;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageTargetSetData", 2, ImageTargetSetData, ISerializable)

Ref< const ImageTargetSet > ImageTargetSetData::createInstance() const
{
	handle_t textureIds[RenderGraphTargetSetDesc::MaxColorTargets];
	for (int32_t i = 0; i < sizeof_array(textureIds); ++i)
		textureIds[i] = getParameterHandle(m_textureIds[i]);

	return new ImageTargetSet(
		m_targetSetId,
		!m_persistentHandle.empty() ? getParameterHandle(m_persistentHandle) : 0,
		textureIds,
		m_targetSetDesc
	);
}

void ImageTargetSetData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"targetSetId", m_targetSetId);

	if (s.getVersion() >= 2)
		s >> Member< std::wstring >(L"persistentHandle", m_persistentHandle);

	s >> MemberStaticArray< std::wstring, RenderGraphTargetSetDesc::MaxColorTargets >(L"textureIds", m_textureIds);
	s >> MemberRenderGraphTargetSetDesc(L"targetSetDesc", m_targetSetDesc);
}

}
