#ifndef traktor_amalgam_WorldLayer_H
#define traktor_amalgam_WorldLayer_H

#include <map>
#include "Amalgam/Engine/Layer.h"
#include "Core/RefArray.h"
#include "Resource/Proxy.h"
#include "Spray/Feedback/IFeedbackListener.h"
#include "World/WorldRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
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
class PostProcess;

	}

	namespace amalgam
	{

class T_DLLCLASS WorldLayer
:	public Layer
,	public spray::IFeedbackListener
{
	T_RTTI_CLASS;

public:
	WorldLayer(
		Stage* stage,
		const std::wstring& name,
		amalgam::IEnvironment* environment,
		const resource::Proxy< scene::Scene >& scene,
		const std::map< std::wstring, resource::Proxy< world::EntityData > >& entities
	);

	virtual ~WorldLayer();

	void destroy();

	virtual void transition(Layer* fromLayer);

	virtual void prepare();

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void preReconfigured();

	virtual void postReconfigured();

	virtual void suspend();

	virtual void resume();

	Ref< world::EntityData > getEntityData(const std::wstring& name) const;

	world::Entity* getEntity(const std::wstring& name) const;

	RefArray< world::Entity > getEntities(const std::wstring& name) const;

	RefArray< world::Entity > getEntitiesOf(const TypeInfo& entityType) const;

	Ref< world::Entity > createEntity(const std::wstring& name, world::IEntitySchema* entitySchema);

	int32_t getEntityIndex(const world::Entity* entity) const;

	int32_t getEntityIndexOf(const world::Entity* entity) const;

	world::Entity* getEntityByIndex(int32_t index) const;

	world::Entity* getEntityOf(const TypeInfo& entityType, int32_t index) const;

	void addEntity(world::Entity* entity);

	void addTransientEntity(world::Entity* entity, float duration);

	void removeEntity(world::Entity* entity);

	world::IEntitySchema* getEntitySchema() const;

	void setControllerEnable(bool controllerEnable);

	void resetController();

	world::PostProcess* getPostProcess() const;

	const Frustum& getViewFrustum() const;

	bool worldToView(const Vector4& worldPosition, Vector4& outViewPosition) const;

	bool viewToWorld(const Vector4& viewPosition, Vector4& outWorldPosition) const;

	bool worldToScreen(const Vector4& worldPosition, Vector2& outScreenPosition) const;

	bool viewToScreen(const Vector4& viewPosition, Vector2& outScreenPosition) const;

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
	virtual void feedbackValues(spray::FeedbackType type, const float* values, int32_t count);

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< scene::Scene > m_scene;
	std::map< std::wstring, resource::Proxy< world::EntityData > > m_entities;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	Ref< world::GroupEntity > m_renderGroup;
	Ref< world::GroupEntity > m_dynamicEntities;
	Ref< const world::Entity > m_cameraEntity;
	Ref< const world::Entity > m_listenerEntity;
	Transform m_cameraOffset;
	float m_alternateTime;
	float m_deltaTime;
	float m_controllerTime;
	float m_fieldOfView;
	float m_feedbackScale;
	bool m_controllerEnable;

	void createWorldRenderer();
};

	}
}

#endif	// traktor_amalgam_WorldLayer_H
