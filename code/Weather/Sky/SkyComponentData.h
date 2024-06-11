/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class ITexture;
class Shader;

}

namespace traktor::weather
{

class SkyComponent;

/*! Sky background component data.
 * \ingroup Weather
 */
class T_DLLCLASS SkyComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	SkyComponentData();

	Ref< SkyComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const resource::Id< render::ITexture >& getTexture() const { return m_texture; }

	float getIntensity() const { return m_intensity; }

private:
	friend class SkyComponent;

	resource::Id< render::Shader > m_shader;
	resource::Id< render::ITexture > m_texture;
	float m_intensity = 1.0f;

	Color4f m_skyOverHorizon = Color4f(0.2f, 0.5f, 0.85f, 1.0f);
	Color4f m_skyUnderHorizon = Color4f(0.1f, 0.1f, 0.12f, 1.0f);

	bool m_clouds = true;
	Color4f m_cloudAmbientTop = Color4f(0.99f, 0.98f, 1.18f, 1.0f);
	Color4f m_cloudAmbientBottom = Color4f(0.23f, 0.39f, 0.51f, 1.0f);
};

}
