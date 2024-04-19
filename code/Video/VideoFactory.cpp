/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Database/Instance.h"
#include "Render/IRenderSystem.h"
#include "Video/Video.h"
#include "Video/VideoFactory.h"
#include "Video/VideoResource.h"
#include "Video/VideoTexture.h"
#include "Video/Decoders/VideoDecoderTheora.h"

namespace traktor::video
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoFactory", 0, VideoFactory, resource::IResourceFactory)

VideoFactory::VideoFactory(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

bool VideoFactory::initialize(const ObjectStore& objectStore)
{
	m_renderSystem = objectStore.get< render::IRenderSystem >();
	return true;
}

const TypeInfoSet VideoFactory::getResourceTypes() const
{
	return makeTypeInfoSet< VideoResource >();
}

const TypeInfoSet VideoFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Video, render::ITexture >();
}

bool VideoFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > VideoFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return nullptr;

	Ref< VideoDecoderTheora > decoder = new VideoDecoderTheora();
	if (!decoder->create(stream))
		return nullptr;

	if (is_type_of< Video >(productType))
	{
		Ref< Video > video = new Video();
		if (video->create(m_renderSystem, decoder))
			return video;
	}
	else if (is_type_of< render::ITexture >(productType))
	{
		Ref< VideoTexture > videoTexture = new VideoTexture();
		if (videoTexture->create(m_renderSystem, decoder))
			return videoTexture;
	}

	return nullptr;
}

}
