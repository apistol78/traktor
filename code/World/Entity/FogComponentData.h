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
#if defined(T_WORLD_EXPORT)
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
class Shader;

}

namespace traktor::world
{

class FogComponent;

/*!
 * \ingroup World
 */
class T_DLLCLASS FogComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< FogComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	const resource::Id< render::Shader >& getShader() const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class FogComponent;

	// Distance fog.
	float m_fogDistance = 90.0f;
	float m_fogDensity = 0.0f;
	float m_fogDensityMax = 1.0f;
	Color4f m_fogColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Volumetric fog.
	bool m_volumetricFogEnable = false;
	float m_maxDistance = 100.0f;
	float m_maxScattering = 10.0f;
	int32_t m_sliceCount = 128;
	Color4f m_mediumColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	float m_mediumDensity = 0.2f;
};

}
