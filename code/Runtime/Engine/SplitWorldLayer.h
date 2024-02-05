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
#include "Spray/Feedback/IFeedbackListener.h"
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

}

namespace traktor::runtime
{

class IEnvironment;

/*! World stage layer.
 * \ingroup Runtime
 */
class T_DLLCLASS SplitWorldLayer
:	public Layer
,	public spray::IFeedbackListener
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

	world::IWorldRenderer* getWorldRenderer() const;

	world::Entity* getEntity(const std::wstring& name) const;

	world::Entity* getEntity(const std::wstring& name, int32_t index) const;

	RefArray< world::Entity > getEntities(const std::wstring& name) const;

	RefArray< world::Entity > getEntitiesWithinRange(const Vector4& position, float range) const;

	Ref< world::Entity > createEntity(const Guid& entityDataId) const;

	void addEntity(world::Entity* entity);

	void removeEntity(world::Entity* entity);

	bool isEntityAdded(const world::Entity* entity) const;

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

	void setFeedbackScale(float feedbackScale);

	float getFeedbackScale() const;

	void setCamera(int32_t split, const world::Entity* cameraEntity);

	const world::Entity* getCamera(int32_t split) const;

protected:
	virtual void feedbackValues(spray::FeedbackType type, const float* values, int32_t count) override final;

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
	float m_feedbackScale = 1.0f;
	bool m_controllerEnable = true;
	SmallMap< Guid, Transform > m_entityTransforms;
};

}
