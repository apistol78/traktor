/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class StaticMesh;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::weather
{

class PrecipitationComponent;

/*! Precipitation component data.
 * \ingroup Weather
 */
class T_DLLCLASS PrecipitationComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	PrecipitationComponentData();

	Ref< PrecipitationComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< mesh::StaticMesh >& getMesh() const { return m_mesh; }

private:
	resource::Id< mesh::StaticMesh > m_mesh;
	float m_tiltRate = 6.0f;
	float m_parallaxDistance = 1.0f;
	float m_depthDistance = 1.0f;
	float m_opacity = 0.1f;
};

}

