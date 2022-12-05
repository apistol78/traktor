/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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

/*! Reflection probe component.
 * \ingroup World
 */
class T_DLLCLASS ProbeComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit ProbeComponent(
		const resource::Proxy< render::ITexture >& texture,
		float intensity,
		const Aabb3& volume,
		bool local,
		bool capture,
		bool dirty
	);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	Transform getTransform() const;

	bool hasOwner() const { return m_owner != nullptr; }

	void setTexture(const resource::Proxy< render::ITexture >& texture) { m_texture = texture; }

	const resource::Proxy< render::ITexture >& getTexture() const { return m_texture; }

	float getIntensity() const { return m_intensity; }

	bool getLocal() const { return m_local; }

	const Aabb3& getVolume() const { return m_volume; }

	bool shouldCapture() const { return m_capture; }

	void setDirty(bool dirty) { m_dirty = dirty; }

	bool getDirty() const { return m_dirty; }

private:
	Entity* m_owner;
	resource::Proxy< render::ITexture > m_texture;
	float m_intensity;
	Aabb3 m_volume;
	bool m_local;
	bool m_capture;
	bool m_dirty;
};

}
