/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetRegion.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor::render
{
namespace
{

const MemberEnum< TrimSheetRegion::Rotation >::Key c_Rotation_Keys[] = {
	{ L"Deg0", TrimSheetRegion::Rotation::Deg0 },
	{ L"Deg90", TrimSheetRegion::Rotation::Deg90 },
	{ L"Deg180", TrimSheetRegion::Rotation::Deg180 },
	{ L"Deg270", TrimSheetRegion::Rotation::Deg270 },
	{ 0 }
};

const MemberEnum< TrimSheetRegion::Tiling >::Key c_Tiling_Keys[] = {
	{ L"None", TrimSheetRegion::Tiling::None },
	{ L"Horizontal", TrimSheetRegion::Tiling::Horizontal },
	{ L"Vertical", TrimSheetRegion::Tiling::Vertical },
	{ L"Both", TrimSheetRegion::Tiling::Both },
	{ 0 }
};

/*! Image of a layer as serialized by version 0, where each layer had its own placement. */
struct LegacyImage
{
	Path fileName;
	TrimSheetRegion::Rotation rotation = TrimSheetRegion::Rotation::Deg0;
	int32_t offsetX = 0;
	int32_t offsetY = 0;
	float scale = 1.0f;
	TrimSheetRegion::Tiling tiling = TrimSheetRegion::Tiling::None;

	void serialize(ISerializer& s)
	{
		s >> Member< Path >(L"fileName", fileName);
		s >> MemberEnum< TrimSheetRegion::Rotation >(L"rotation", rotation, c_Rotation_Keys);
		s >> Member< int32_t >(L"offsetX", offsetX);
		s >> Member< int32_t >(L"offsetY", offsetY);
		s >> Member< float >(L"scale", scale);
		s >> MemberEnum< TrimSheetRegion::Tiling >(L"tiling", tiling, c_Tiling_Keys);
	}
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TrimSheetRegion", 5, TrimSheetRegion, ISerializable)

void TrimSheetRegion::Image::serialize(ISerializer& s)
{
	s >> Member< Path >(L"fileName", fileName);
	s >> Member< std::wstring >(L"swizzle", swizzle);
}

TrimSheetRegion::TrimSheetRegion(int32_t size)
:	m_size(size)
{
}

bool TrimSheetRegion::hasImage() const
{
	for (const auto& image : m_images)
	{
		if (!image.fileName.empty())
			return true;
	}
	return false;
}

void TrimSheetRegion::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< int32_t >(L"size", m_size, AttributeRange(0));

	if (s.getVersion< TrimSheetRegion >() >= 4)
		s >> Member< int32_t >(L"margin", m_margin, AttributeRange(0, 256));

	if (s.getVersion< TrimSheetRegion >() < 1)
	{
		LegacyImage images[TrimSheetLayerCount];
		s >> MemberComposite< LegacyImage >(L"albedo", images[(int32_t)TrimSheetLayer::Albedo]);
		s >> MemberComposite< LegacyImage >(L"specular", images[(int32_t)TrimSheetLayer::Specular]);
		s >> MemberComposite< LegacyImage >(L"normal", images[(int32_t)TrimSheetLayer::Normal]);
		s >> MemberComposite< LegacyImage >(L"height", images[(int32_t)TrimSheetLayer::Height]);

		// Placement is shared by all layers now; keep placement of first layer which has an image.
		const LegacyImage* placement = nullptr;
		for (int32_t i = 0; i < TrimSheetLayerCount; ++i)
		{
			m_images[i].fileName = images[i].fileName;
			if (!placement && !images[i].fileName.empty())
				placement = &images[i];
		}
		if (!placement)
			placement = &images[(int32_t)TrimSheetLayer::Albedo];

		m_rotation = placement->rotation;
		m_offsetX = placement->offsetX;
		m_offsetY = placement->offsetY;
		m_scale = placement->scale;
		m_tiling = placement->tiling;
		return;
	}

	if (s.getVersion< TrimSheetRegion >() >= 3)
	{
		s >> MemberComposite< Image >(L"albedo", m_images[(int32_t)TrimSheetLayer::Albedo]);
		s >> MemberComposite< Image >(L"specular", m_images[(int32_t)TrimSheetLayer::Specular]);
		s >> MemberComposite< Image >(L"roughness", m_images[(int32_t)TrimSheetLayer::Roughness]);
		if (s.getVersion< TrimSheetRegion >() >= 5)
			s >> MemberComposite< Image >(L"metallic", m_images[(int32_t)TrimSheetLayer::Metallic]);
		s >> MemberComposite< Image >(L"normal", m_images[(int32_t)TrimSheetLayer::Normal]);
		s >> MemberComposite< Image >(L"height", m_images[(int32_t)TrimSheetLayer::Height]);
	}
	else
	{
		s >> Member< Path >(L"albedo", m_images[(int32_t)TrimSheetLayer::Albedo].fileName);
		s >> Member< Path >(L"specular", m_images[(int32_t)TrimSheetLayer::Specular].fileName);
		if (s.getVersion< TrimSheetRegion >() >= 2)
			s >> Member< Path >(L"roughness", m_images[(int32_t)TrimSheetLayer::Roughness].fileName);
		s >> Member< Path >(L"normal", m_images[(int32_t)TrimSheetLayer::Normal].fileName);
		s >> Member< Path >(L"height", m_images[(int32_t)TrimSheetLayer::Height].fileName);
	}

	s >> MemberEnum< Rotation >(L"rotation", m_rotation, c_Rotation_Keys);
	s >> Member< int32_t >(L"offsetX", m_offsetX);
	s >> Member< int32_t >(L"offsetY", m_offsetY);
	s >> Member< float >(L"scale", m_scale, AttributeRange(0.01f, 16.0f));
	s >> MemberEnum< Tiling >(L"tiling", m_tiling, c_Tiling_Keys);
}

}
