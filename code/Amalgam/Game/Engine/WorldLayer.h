/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_WorldLayer_H
#define traktor_amalgam_WorldLayer_H

#include <map>
#include "Amalgam/Game/Engine/Layer.h"
#include "Core/RefArray.h"
#include "Core/Math/IntervalTransform.h"
#include "Core/Math/Ray3.h"
#include "Resource/Proxy.h"
#include "Spray/Feedback/IFeedbackListener.h"
#include "World/WorldRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace render
	{

class ImageProcess;

	}

	namespace scene
	{

class Scene;

	}

	namespace world
	{

class Entity;
class EntityData;
class GroupEntity;
class IEntitySchema;
class IWorldRenderer;

	}

	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS WorldLayer
:	public Layer
,	public spray::IFeedbackListener
{
	T_RTTI_CLASS;

public:
	WorldLayer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		IEnvironment* environment,
		const resource::Proxy< scene::Scene >& scene,
		const std::map< std::wstring, resource::Proxy< world::EntityData > >& entities
	);

	virtual void destroy() T_OVERRIDE;

	virtual void transition(Layer* fromLayer) T_OVERRIDE T_FINAL;

	virtual void prepare(const UpdateInfo& info) T_OVERRIDE T_FINAL;

	virtual void update(const UpdateInfo& info) T_OVERRIDE T_FINAL;

	virtual void build(const UpdateInfo& info, uint32_t frame) T_OVERRIDE T_FINAL;

	virtual void render(render::EyeType eye, uint32_t frame) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

	virtual void preReconfigured() T_OVERRIDE T_FINAL;

	virtual void postReconfigured() T_OVERRIDE T_FINAL;

	virtual void suspend() T_OVERRIDE T_FINAL;

	virtual void resume() T_OVERRIDE T_FINAL;

	Ref< world::EntityData > getEntityData(const std::wstring& name) const;

	world::Entity* getEntity(const std::wstring& name) const;

	world::Entity* getEntity(const std::wstring& name, int32_t index) const;

	RefArray< world::Entity > getEntities(const std::wstring& name) const;

	RefArray< world::Entity > getEntitiesOf(const TypeInfo& entityType) const;

	Ref< world::Entity > createEntity(const std::wstring& name, world::IEntitySchema* entitySchema);

	int32_t getEntityIndex(const world::Entity* entity) const;

	int32_t getEntityIndexOf(const world::Entity* entity) const;

	world::Entity* getEntityByIndex(int32_t index) const;

	world::Entity* getEntityOf(const TypeInfo& entityType, int32_t index) const;

	void addEntity(world::Entity* entity);

	void removeEntity(world::Entity* entity);

	bool isEntityAdded(const world::Entity* entity) const;

	world::IEntitySchema* getEntitySchema() const;

	void setControllerEnable(bool controllerEnable);

	void resetController();

	render::ImageProcess* getImageProcess() const;

	const Frustum& getViewFrustum() const;

	bool worldToView(const Vector4& worldPosition, Vector4& outViewPosition) const;

	bool viewToWorld(const Vector4& viewPosition, Vector4& outWorldPosition) const;

	bool worldToScreen(const Vector4& worldPosition, Vector2& outScreenPosition) const;

	bool viewToScreen(const Vector4& viewPosition, Vector2& outScreenPosition) const;

	bool screenToView(const Vector2& screenPosition, Ray3& outViewRay) const;

	bool screenToWorld(const Vector2& screenPosition, Ray3& outWorldRay) const;

	void setFieldOfView(float fieldOfView);

	float getFieldOfView() const;

	void setAlternateTime(float alternateTime);

	float getAlternateTime() const;

	void setFeedbackScale(float feedbackScale);

	float getFeedbackScale() const;

	void setCamera(const world::Entity* cameraEntity);

	const world::Entity* getCamera() const;

	void setListener(const world::Entity* listenerEntity);

	const world::Entity* getListener() const;

protected:
	virtual void feedbackValues(spray::FeedbackType type, const float* values, int32_t count) T_OVERRIDE T_FINAL;

private:
	Ref< IEnvironment > m_environment;
	resource::Proxy< scene::Scene > m_scene;
	std::map< std::wstring, resource::Proxy< world::EntityData > > m_entities;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	Ref< world::GroupEntity > m_renderGroup;
	Ref< world::GroupEntity > m_dynamicEntities;
	Ref< const world::Entity > m_cameraEntity;
	Ref< const world::Entity > m_listenerEntity;
	IntervalTransform m_cameraTransform;
	Transform m_cameraOffset;
	float m_alternateTime;
	float m_deltaTime;
	float m_controllerTime;
	float m_fieldOfView;
	float m_feedbackScale;
	bool m_controllerEnable;
};

	}
}

#endif	// traktor_amalgam_WorldLayer_H
