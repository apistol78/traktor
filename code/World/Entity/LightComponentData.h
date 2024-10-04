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

namespace traktor::render
{

class ITexture;

}

namespace traktor::world
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

	virtual int32_t getOrdinal() const override final;

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

	void setNearRange(float range) { m_nearRange = range;  }

	float getNearRange() const { return m_nearRange; }

	void setFarRange(float range) { m_farRange = range;  }

	float getFarRange() const { return m_farRange; }

	void setRadius(float radius) { m_radius = radius; }

	float getRadius() const { return m_radius;  }

	void setFlickerAmount(float flickerAmount) { m_flickerAmount = flickerAmount; }

	float getFlickerAmount() const { return m_flickerAmount; }

	void setFlickerFilter(float flickerFilter) { m_flickerFilter = flickerFilter; }

	float getFlickerFilter() const { return m_flickerFilter; }

	LightBakeMode getBakeMode() const { return m_bakeMode; }

private:
	LightType m_lightType = LightType::Disabled;
	Color4f m_color = Color4f(1.0f, 1.0f, 1.0f, 0.0f);
	float m_intensity = 10.0f;
	bool m_castShadow = true;
	float m_nearRange = 0.0f;
	float m_farRange = 10.0f;
	float m_radius = HALF_PI;
	float m_flickerAmount = 0.0f;
	float m_flickerFilter = 0.0f;
	LightBakeMode m_bakeMode = LbmIndirect;
};

}
