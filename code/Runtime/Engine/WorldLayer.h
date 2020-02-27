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

namespace traktor
{
	namespace runtime
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
class EntitySchema;
class IWorldRenderer;

	}

	namespace runtime
	{

/*! \brief
 * \ingroup Runtime
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
		const resource::Proxy< scene::Scene >& scene
	);

	virtual void destroy() override;

	virtual void transition(Layer* fromLayer) override final;

	virtual void prepare(const UpdateInfo& info) override final;

	virtual void update(const UpdateInfo& info) override final;

	virtual void build(const UpdateInfo& info, uint32_t frame) override final;

	virtual void render(uint32_t frame) override final;

	virtual void flush() override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

	world::Entity* getEntity(const std::wstring& name) const;

	world::Entity* getEntity(const std::wstring& name, int32_t index) const;

	RefArray< world::Entity > getEntities(const std::wstring& name) const;

	RefArray< world::Entity > getEntitiesOf(const TypeInfo& entityType) const;

	int32_t getEntityIndex(const world::Entity* entity) const;

	int32_t getEntityIndexOf(const world::Entity* entity) const;

	world::Entity* getEntityByIndex(int32_t index) const;

	world::Entity* getEntityOf(const TypeInfo& entityType, int32_t index) const;

	void addEntity(world::Entity* entity);

	void removeEntity(world::Entity* entity);

	bool isEntityAdded(const world::Entity* entity) const;

	world::EntitySchema* getEntitySchema() const;

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
	virtual void feedbackValues(spray::FeedbackType type, const float* values, int32_t count) override final;

private:
	Ref< IEnvironment > m_environment;
	resource::Proxy< scene::Scene > m_scene;
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
