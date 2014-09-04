#include "Amalgam/Engine/Classes/WorldClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"
#include "World/EntityBuilder.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/EntityData.h"
#include "World/EntitySchema.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"
#include "World/IEntityEventInstance.h"
#include "World/IEntityEventManager.h"
#include "World/IEntityFactory.h"
#include "World/IEntityRenderer.h"
#include "World/PostProcess/PostProcess.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/SwitchEntity.h"
#include "World/Entity/TransientEntity.h"
#include "World/Entity/VolumeEntity.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

void world_IEntityEventInstance_cancelImmediate(world::IEntityEventInstance* this_)
{
	this_->cancel(world::CtImmediate);
}

void world_IEntityEventInstance_cancelEnd(world::IEntityEventInstance* this_)
{
	this_->cancel(world::CtEnd);
}

void world_IEntityEventManager_cancelAllImmediate(world::IEntityEventManager* this_)
{
	this_->cancelAll(world::CtImmediate);
}

void world_IEntityEventManager_cancelAllEnd(world::IEntityEventManager* this_)
{
	this_->cancelAll(world::CtEnd);
}

Ref< world::Entity > world_IEntityBuilder_create(world::IEntityBuilder* this_, const world::EntityData* entityData)
{
	return this_->create(entityData);
}

Transform world_Entity_getTransform(world::Entity* this_)
{
	Transform transform;
	this_->getTransform(transform);
	return transform;
}

RefArray< world::Entity > world_GroupEntity_getEntitiesOf(world::GroupEntity* this_, const TypeInfo& entityType)
{
	RefArray< world::Entity > entities;
	this_->getEntitiesOf(entityType, entities);
	return entities;
}

Ref< world::Entity > world_GroupEntity_getFirstEntityOf(world::GroupEntity* this_, const TypeInfo& entityType)
{
	return this_->getFirstEntityOf(entityType);
}

void world_PostProcess_setCombination(world::PostProcess* this_, const std::wstring& name, bool value)
{
	this_->setCombination(render::getParameterHandle(name), value);
}

void world_PostProcess_setParameter(world::PostProcess* this_, const std::wstring& name, float value)
{
	this_->setFloatParameter(render::getParameterHandle(name), value);
}

void world_Entity_update(world::Entity* this_, float totalTime, float deltaTime)
{
	world::UpdateParams up;
	up.totalTime = totalTime;
	up.deltaTime = deltaTime;
	up.alternateTime = totalTime;
	this_->update(up);
}

		}

void registerWorldClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< world::IEntityEventInstance > > classIEntityEventInstance = new script::AutoScriptClass< world::IEntityEventInstance >();
	classIEntityEventInstance->addMethod("cancelImmediate", &world_IEntityEventInstance_cancelImmediate);
	classIEntityEventInstance->addMethod("cancelEnd", &world_IEntityEventInstance_cancelEnd);
	scriptManager->registerClass(classIEntityEventInstance);

	Ref< script::AutoScriptClass< world::IEntityEventData > > classIEntityEventData = new script::AutoScriptClass< world::IEntityEventData >();
	scriptManager->registerClass(classIEntityEventData);

	Ref< script::AutoScriptClass< world::IEntityEvent > > classIEntityEvent = new script::AutoScriptClass< world::IEntityEvent >();
	scriptManager->registerClass(classIEntityEvent);

	Ref< script::AutoScriptClass< world::IEntityEventManager > > classIEntityEventManager = new script::AutoScriptClass< world::IEntityEventManager >();
	classIEntityEventManager->addMethod("cancelAllImmediate", &world_IEntityEventManager_cancelAllImmediate);
	classIEntityEventManager->addMethod("cancelAllEnd", &world_IEntityEventManager_cancelAllEnd);
	scriptManager->registerClass(classIEntityEventManager);

	Ref< script::AutoScriptClass< world::IEntitySchema > > classIEntitySchema = new script::AutoScriptClass< world::IEntitySchema >();
	classIEntitySchema->addMethod< world::Entity*, uint32_t >("getEntity", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const std::wstring&, uint32_t >("getEntity", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const TypeInfo&, uint32_t >("getEntityOf", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getEntityOf", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, uint32_t >("getChildEntity", &world::IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, const std::wstring&, uint32_t >("getChildEntity", &world::IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, const TypeInfo&, uint32_t >("getChildEntityOf", &world::IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getChildEntityOf", &world::IEntitySchema::getChildEntity);
	scriptManager->registerClass(classIEntitySchema);

	Ref< script::AutoScriptClass< world::EntitySchema > > classEntitySchema = new script::AutoScriptClass< world::EntitySchema >();
	classEntitySchema->addConstructor();
	scriptManager->registerClass(classEntitySchema);

	Ref< script::AutoScriptClass< world::IEntityFactory > > classIEntityFactory = new script::AutoScriptClass< world::IEntityFactory >();
	scriptManager->registerClass(classIEntityFactory);

	Ref< script::AutoScriptClass< world::IEntityBuilder > > classIEntityBuilder = new script::AutoScriptClass< world::IEntityBuilder >();
	classIEntityBuilder->addMethod("addFactory", &world::IEntityBuilder::addFactory);
	classIEntityBuilder->addMethod("removeFactory", &world::IEntityBuilder::removeFactory);
	classIEntityBuilder->addMethod("create", &world_IEntityBuilder_create);
	scriptManager->registerClass(classIEntityBuilder);

	Ref< script::AutoScriptClass< world::EntityBuilder > > classEntityBuilder = new script::AutoScriptClass< world::EntityBuilder >();
	classEntityBuilder->addConstructor();
	scriptManager->registerClass(classEntityBuilder);

	Ref< script::AutoScriptClass< world::EntityBuilderWithSchema > > classEntityBuilderWithSchema = new script::AutoScriptClass< world::EntityBuilderWithSchema >();
	classEntityBuilderWithSchema->addConstructor< world::IEntityBuilder*, world::IEntitySchema* >();
	scriptManager->registerClass(classEntityBuilderWithSchema);

	Ref< script::AutoScriptClass< world::IEntityRenderer > > classIEntityRenderer = new script::AutoScriptClass< world::IEntityRenderer >();
	scriptManager->registerClass(classIEntityRenderer);

	Ref< script::AutoScriptClass< world::EntityData > > classEntityData = new script::AutoScriptClass< world::EntityData >();
	classEntityData->addMethod("setName", &world::EntityData::setName);
	classEntityData->addMethod("getName", &world::EntityData::getName);
	classEntityData->addMethod("setTransform", &world::EntityData::setTransform);
	classEntityData->addMethod("getTransform", &world::EntityData::getTransform);
	scriptManager->registerClass(classEntityData);

	Ref< script::AutoScriptClass< world::Entity > > classEntity = new script::AutoScriptClass< world::Entity >();
	classEntity->addMethod("destroy", &world::Entity::destroy);
	classEntity->addMethod("setTransform", &world::Entity::setTransform);
	classEntity->addMethod("getTransform", &world_Entity_getTransform);
	classEntity->addMethod("getBoundingBox", &world::Entity::getBoundingBox);
	classEntity->addMethod("getWorldBoundingBox", &world::Entity::getWorldBoundingBox);
	classEntity->addMethod("update", &world_Entity_update);
	scriptManager->registerClass(classEntity);

	Ref< script::AutoScriptClass< world::GroupEntity > > classGroupEntity = new script::AutoScriptClass< world::GroupEntity >();
	classGroupEntity->addConstructor< const Transform& >();
	classGroupEntity->addMethod("addEntity", &world::GroupEntity::addEntity);
	classGroupEntity->addMethod("removeEntity", &world::GroupEntity::removeEntity);
	classGroupEntity->addMethod("removeAllEntities", &world::GroupEntity::removeAllEntities);
	classGroupEntity->addMethod("getEntities", &world::GroupEntity::getEntities);
	classGroupEntity->addMethod("getEntitiesOf", &world_GroupEntity_getEntitiesOf);
	classGroupEntity->addMethod("getFirstEntityOf", &world_GroupEntity_getFirstEntityOf);
	scriptManager->registerClass(classGroupEntity);

	Ref< script::AutoScriptClass< world::PointLightEntity > > classPointLightEntity = new script::AutoScriptClass< world::PointLightEntity >();
	classPointLightEntity->addMethod("getSunColor", &world::PointLightEntity::getSunColor);
	classPointLightEntity->addMethod("getBaseColor", &world::PointLightEntity::getBaseColor);
	classPointLightEntity->addMethod("getShadowColor", &world::PointLightEntity::getShadowColor);
	classPointLightEntity->addMethod("getRange", &world::PointLightEntity::getRange);
	classPointLightEntity->addMethod("getRandomFlicker", &world::PointLightEntity::getRandomFlicker);
	scriptManager->registerClass(classPointLightEntity);

	Ref< script::AutoScriptClass< world::DirectionalLightEntity > > classDirectionalLightEntity = new script::AutoScriptClass< world::DirectionalLightEntity >();
	classDirectionalLightEntity->addConstructor< const Transform&, const Vector4&, const Vector4&, const Vector4&, bool >();
	classDirectionalLightEntity->addMethod("setSunColor", &world::DirectionalLightEntity::setSunColor);
	classDirectionalLightEntity->addMethod("getSunColor", &world::DirectionalLightEntity::getSunColor);
	classDirectionalLightEntity->addMethod("setBaseColor", &world::DirectionalLightEntity::setBaseColor);
	classDirectionalLightEntity->addMethod("getBaseColor", &world::DirectionalLightEntity::getBaseColor);
	classDirectionalLightEntity->addMethod("setShadowColor", &world::DirectionalLightEntity::setShadowColor);
	classDirectionalLightEntity->addMethod("getShadowColor", &world::DirectionalLightEntity::getShadowColor);
	classDirectionalLightEntity->addMethod("setCastShadow", &world::DirectionalLightEntity::setCastShadow);
	classDirectionalLightEntity->addMethod("getCastShadow", &world::DirectionalLightEntity::getCastShadow);
	scriptManager->registerClass(classDirectionalLightEntity);

	Ref< script::AutoScriptClass< world::NullEntity > > classNullEntity = new script::AutoScriptClass< world::NullEntity >();
	classNullEntity->addConstructor< const Transform& >();
	scriptManager->registerClass(classNullEntity);

	Ref< script::AutoScriptClass< world::SwitchEntity > > classSwitchEntity = new script::AutoScriptClass< world::SwitchEntity >();
	classSwitchEntity->addConstructor();
	classSwitchEntity->addConstructor< const Transform&, int32_t >();
	classSwitchEntity->addMethod("addEntity", &world::SwitchEntity::addEntity);
	classSwitchEntity->addMethod("setActive", &world::SwitchEntity::setActive);
	classSwitchEntity->addMethod("getActive", &world::SwitchEntity::getActive);
	classSwitchEntity->addMethod("getActiveEntity", &world::SwitchEntity::getActiveEntity);
	scriptManager->registerClass(classSwitchEntity);

	Ref< script::AutoScriptClass< world::TransientEntity > > classTransientEntity = new script::AutoScriptClass< world::TransientEntity >();
	classTransientEntity->addConstructor< world::GroupEntity*, world::Entity*, float >();
	scriptManager->registerClass(classTransientEntity);

	Ref< script::AutoScriptClass< world::VolumeEntity > > classVolumeEntity = new script::AutoScriptClass< world::VolumeEntity >();
	classVolumeEntity->addMethod("inside", &world::VolumeEntity::inside);
	scriptManager->registerClass(classVolumeEntity);

	Ref< script::AutoScriptClass< world::PostProcess > > classPostProcess = new script::AutoScriptClass< world::PostProcess >();
	classPostProcess->addMethod("setCombination", &world_PostProcess_setCombination);
	classPostProcess->addMethod("setParameter", &world_PostProcess_setParameter);
	classPostProcess->addMethod("requireHighRange", &world::PostProcess::requireHighRange);
	scriptManager->registerClass(classPostProcess);
}

	}
}
