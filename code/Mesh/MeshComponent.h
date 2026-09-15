/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Math/IntervalTransform.h"
#include "World/IEntityComponent.h"
#include "World/Entity/IntervalTransformComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderContext;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;
class WorldSetupContext;

}

namespace traktor::mesh
{

class IMeshParameterCallback;

/*! Abstract mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS MeshComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	virtual ~MeshComponent();

	virtual void destroy() override;

	virtual void setOwner(world::Entity* owner) override;

	virtual void setWorld(world::World* world) override;

	virtual void setTransform(const Transform& transform) override;

	virtual void update(const world::UpdateParams& update) override;

	/*! Set parameter callback.
	 * 
	 * Callback is invoked each time a piece of a
	 * mesh is rendered, ie for each draw call,
	 * to let user specify custom parameters.
	 * 
	 * \param parameterCallback Parameter callback.
	 */
	void setParameterCallback(const IMeshParameterCallback* parameterCallback) { m_parameterCallback = parameterCallback; }

	const IMeshParameterCallback* getParameterCallback() const { return m_parameterCallback; }

protected:
	world::Entity* m_owner = nullptr;
	world::World* m_world = nullptr;
	const IMeshParameterCallback* m_parameterCallback = nullptr;
	world::IntervalTransformComponent::Entry* m_transform = nullptr;

private:
	void freeIntervalTransform();
};

}
