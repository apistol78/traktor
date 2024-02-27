/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Plane.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Spray/Types.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spray
{

class Effect;
class EffectInstance;
class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! Effect component instance.
 * \ingroup Spray
 */
class T_DLLCLASS EffectComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit EffectComponent(const resource::Proxy< Effect >& effect);

	explicit EffectComponent(const resource::Proxy< Effect >& effect, EffectInstance* effectInstance, const Context& context);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void render(
		render::handle_t technique,
		const Vector4& cameraPosition,
		const Plane& cameraPlane,
		PointRenderer* pointRenderer,
		MeshRenderer* meshRenderer,
		TrailRenderer* trailRenderer
	) const;

	Aabb3 getWorldBoundingBox() const;

	void reset();

	void setLoopEnable(bool loopEnable);

	bool getLoopEnable() const;

	bool isFinished() const;

	const resource::Proxy< Effect >& getEffect() const { return m_effect; }

	EffectInstance* getEffectInstance() const { return m_effectInstance; }

	void setEnable(bool enable) { m_enable = enable; }

	bool isEnable() const { return m_enable; }

	bool haveTechnique(render::handle_t technique) const { return m_techniques.find(technique) != m_techniques.end(); }

	void setVirtualSourceCallback(VirtualSourceCallback* virtualSourceCallback) { m_context.virtualSourceCallback = virtualSourceCallback; }

private:
	Transform m_transform;
	resource::Proxy< Effect > m_effect;
	Ref< EffectInstance > m_effectInstance;
	SmallSet< render::handle_t > m_techniques;
	Context m_context;
	uint32_t m_counter = 0;
	bool m_enable = true;

	void updateTechniques();
};

}
