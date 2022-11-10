/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/VideoLayer.h"
#include "Runtime/Engine/VideoLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Video/Video.h"
#include "Video/VideoResource.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.VideoLayerData", 0, VideoLayerData, LayerData)

Ref< Layer > VideoLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< video::Video > video;
	resource::Proxy< render::Shader > shader;

	// Bind proxies to resource manager.
	if (
		!resourceManager->bind(m_video, video) ||
		!resourceManager->bind(m_shader, shader)
	)
		return nullptr;

	// Create layer instance.
	return new VideoLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		video,
		shader,
		m_screenBounds,
		m_visible,
		m_autoPlay,
		m_repeat
	);
}

void VideoLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< video::Video >(L"video", m_video);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAabb2(L"screenBounds", m_screenBounds);
	s >> Member< bool >(L"visible", m_visible);
	s >> Member< bool >(L"autoPlay", m_autoPlay);
	s >> Member< bool >(L"repeat", m_repeat);
}

	}
}
