/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

	}

	namespace world
	{

/*!
 * \ingroup World
 */
class T_DLLCLASS LightComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	enum LightBakeMode
	{
		LbmDisabled,	//!< Not used in baking, light is dynamic.
		LbmIndirect,	//!< Indirect light is baked, direct light is dynamic.
		LbmAll			//!< All lighting, direct and indirect, are baked and this light is not dynamic.
	};

	LightComponentData();

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	void setLightType(LightType lightType) { m_lightType = lightType; }

	LightType getLightType() const { return m_lightType; }

	void setColor(const Color4f& color) { m_color = color; }

	const Color4f& getColor() const { return m_color; }

	void setIntensity(float intensity) { m_intensity = intensity; }

	float getIntensity() const { return m_intensity; }

	void setCastShadow(bool castShadow) { m_castShadow = castShadow; }

	bool getCastShadow() const { return m_castShadow; }

	void setRange(float range) { m_range = range;  }

	float getRange() const { return m_range; }

	void setRadius(float radius) { m_radius = radius; }

	float getRadius() const { return m_radius;  }

	void setFlickerAmount(float flickerAmount) { m_flickerAmount = flickerAmount; }

	float getFlickerAmount() const { return m_flickerAmount; }

	void setFlickerFilter(float flickerFilter) { m_flickerFilter = flickerFilter; }

	float getFlickerFilter() const { return m_flickerFilter; }

	LightBakeMode getBakeMode() const { return m_bakeMode; }

private:
	LightType m_lightType;
	Color4f m_color;
	float m_intensity;
	bool m_castShadow;
	float m_range;
	float m_radius;
	float m_flickerAmount;
	float m_flickerFilter;
	LightBakeMode m_bakeMode;
};

	}
}
