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
#include "Render/Shader.h"
#include "Render/Image2/ImagePassStep.h"
#include "Render/Image2/ImagePassStepData.h"
#include "Resource/Member.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImagePassStepData", ImagePassStepData, ISerializable)

void ImagePassStepData::setSourceHandles(ImagePassStep* instance) const
{
	for (const auto& source : m_textureSources)
	{
		instance->m_textureSources.push_back({
			getParameterHandle(source.id),
			getParameterHandle(source.shaderParameter)
		});
	}
	for (const auto& source : m_sbufferSources)
	{
		instance->m_sbufferSources.push_back({
			getParameterHandle(source.id),
			getParameterHandle(source.shaderParameter)
		});
	}
}

void ImagePassStepData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< Source, MemberComposite< Source > >(L"textureSources", m_textureSources);
	s >> MemberAlignedVector< Source, MemberComposite< Source > >(L"sbufferSources", m_sbufferSources);
}

void ImagePassStepData::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"id", id);
	s >> Member< std::wstring >(L"shaderParameter", shaderParameter);
}

}
