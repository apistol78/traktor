/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/Engine/Layer.h"
#include "Core/RefArray.h"
#include "Core/Math/IntervalTransform.h"
#include "Core/Math/Ray3.h"
#include "Resource/Proxy.h"
#include "World/WorldRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ScreenRenderer;
class Shader;

}

namespace traktor::scene
{

class Scene;

}

namespace traktor::world
{

class Entity;
class EntityData;
class IWorldRenderer;
class World;

}

namespace traktor::runtime
{

class IEnvironment;

/*! World stage layer.
 * \ingroup Runtime
 */
class T_DLLCLASS SplitWorldLayer : public Layer
{
	T_RTTI_CLASS;

public:
	explicit SplitWorldLayer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		IEnvironment* environment,
		const resource::Proxy< scene::Scene >& scene,
		const resource::Proxy< render::Shader >& shader
	);

	virtual void destroy() override;

	virtual void transition(Layer* fromLayer) override final;

	virtual void preUpdate(const UpdateInfo& info) override final;

	virtual void update(const UpdateInfo& info) override final;

	virtual void postUpdate(const UpdateInfo& info) override final;

	virtual void preSetup(const UpdateInfo& info) override final;

	virtual void setup(const UpdateInfo& info, render::RenderGraph& renderGraph) override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

	virtual void hotReload() override final;

	scene::Scene* getScene() const;

	world::World* getWorld() const;

	world::IWorldRenderer* getWorldRenderer() const;

	Ref< world::Entity > createEntity(const Guid& entityDataId) const;

	void setControllerEnable(bool controllerEnable);

	void resetController();

	const Frustum& getViewFrustum(int32_t split) const;

	bool worldToView(int32_t split, const Vector4& worldPosition, Vector4& outViewPosition) const;

	bool viewToWorld(int32_t split, const Vector4& viewPosition, Vector4& outWorldPosition) const;

	bool worldToScreen(int32_t split, const Vector4& worldPosition, Vector2& outScreenPosition) const;

	bool viewToScreen(int32_t split, const Vector4& viewPosition, Vector2& outScreenPosition) const;

	bool screenToView(int32_t split, const Vector2& screenPosition, Ray3& outViewRay) const;

	bool screenToWorld(int32_t split, const Vector2& screenPosition, Ray3& outWorldRay) const;

	void setFieldOfView(float fieldOfView);

	float getFieldOfView() const;

	void setAlternateTime(double alternateTime);

	double getAlternateTime() const;

	void setCamera(int32_t split, const world::Entity* cameraEntity);

	const world::Entity* getCamera(int32_t split) const;

private:
	IEnvironment* m_environment;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_shader;
	resource::Proxy< scene::Scene > m_scene;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderViews[2];
	Ref< const world::Entity > m_cameraEntities[2];
	IntervalTransform m_cameraTransforms[2];
	Transform m_cameraOffsets[2];
	double m_alternateTime = 0.0;
	double m_deltaTime = 0.0;
	float m_fieldOfView = 70.0f;
	bool m_controllerEnable = true;
	SmallMap< Guid, Transform > m_entityTransforms;
};

}
