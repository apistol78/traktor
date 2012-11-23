#ifndef traktor_parade_WorldLayer_H
#define traktor_parade_WorldLayer_H

#include <map>
#include "Core/RefArray.h"
#include "Resource/Proxy.h"
#include "World/WorldRenderView.h"
#include "Parade/Layer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
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

	namespace parade
	{

class T_DLLCLASS WorldLayer : public Layer
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

	virtual void prepare();

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void leave();

	virtual void reconfigured();

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

	world::PostProcess* getPostProcess() const;

	bool getViewPosition(const Vector4& worldPosition, Vector4& outViewPosition) const;

	bool getScreenPosition(const Vector4& viewPosition, Vector2& outScreenPosition) const;

	void setFieldOfView(float fieldOfView);

	float getFieldOfView() const;

	void setAlternateTime(float alternateTime);

	float getAlternateTime() const;

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< scene::Scene > m_scene;
	std::map< std::wstring, resource::Proxy< world::EntityData > > m_entities;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	Ref< world::GroupEntity > m_renderGroup;
	Ref< world::GroupEntity > m_dynamicEntities;
	float m_alternateTime;
	float m_deltaTime;
	float m_fieldOfView;
	bool m_controllerEnable;

	void createWorldRenderer();
};

	}
}

#endif	// traktor_parade_WorldLayer_H
