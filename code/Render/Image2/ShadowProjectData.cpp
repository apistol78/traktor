/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Render/Image2/ShadowProject.h"
#include "Render/Image2/ShadowProjectData.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
    namespace render
    {
		namespace
		{
		
Ref< ISimpleTexture > createRandomRotationTexture(IRenderSystem* renderSystem)
{
	static Random random;

	AutoArrayPtr< uint8_t > data(new uint8_t [128 * 128 * 4]);
	for (uint32_t y = 0; y < 128; ++y)
	{
		for (uint32_t x = 0; x < 128; ++x)
		{
			float angle = (random.nextFloat() * 2.0f - 1.0f) * PI;
			float xa =  cosf(angle) * 127.5f + 127.5f;
			float xb =  sinf(angle) * 127.5f + 127.5f;
			float ya =  sinf(angle) * 127.5f + 127.5f;
			float yb = -cosf(angle) * 127.5f + 127.5f;
			data[(x + y * 128) * 4 + 0] = uint8_t(xa);
			data[(x + y * 128) * 4 + 1] = uint8_t(xb);
			data[(x + y * 128) * 4 + 2] = uint8_t(ya);
			data[(x + y * 128) * 4 + 3] = uint8_t(yb);
		}
	}

	SimpleTextureCreateDesc desc;
	desc.width = 128;
	desc.height = 128;
	desc.mipCount = 1;
	desc.format = TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 128 * 4;
	desc.initialData[0].slicePitch = 0;

	return renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
}
		
		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShadowProjectData", 0, ShadowProjectData, ImagePassOpData)

Ref< const ImagePassOp > ShadowProjectData::createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const
{
	Ref< ShadowProject > instance = new ShadowProject();

	// Bind shader.
	if (!resourceManager->bind(m_shader, instance->m_shader))
		return nullptr;

	// Get handles of sources.
	for (const auto& source : m_sources)
	{
		instance->m_sources.push_back({
			getParameterHandle(source.textureId),
			getParameterHandle(source.parameter)
		});
	}

	// Generate screen-space random rotation textures.
	if ((instance->m_shadowMapDiscRotation[0] = createRandomRotationTexture(renderSystem)) == nullptr)
		return nullptr;
	if ((instance->m_shadowMapDiscRotation[1] = createRandomRotationTexture(renderSystem)) == nullptr)
		return nullptr;

	return instance; 
}

    }
}