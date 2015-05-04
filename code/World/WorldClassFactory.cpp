#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Misc/String.h"
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
#include "World/WorldClassFactory.h"
#include "World/PostProcess/PostProcess.h"
#include "World/Entity/CameraEntity.h"
#include "World/Entity/CameraEntityData.h"
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
	namespace world
	{
		namespace
		{

void IEntityEventInstance_cancelImmediate(IEntityEventInstance* this_)
{
	this_->cancel(CtImmediate);
}

void IEntityEventInstance_cancelEnd(IEntityEventInstance* this_)
{
	this_->cancel(CtEnd);
}

void IEntityEventManager_cancelAllImmediate(IEntityEventManager* this_)
{
	this_->cancelAll(CtImmediate);
}

void IEntityEventManager_cancelAllEnd(IEntityEventManager* this_)
{
	this_->cancelAll(CtEnd);
}

Ref< Entity > IEntityBuilder_create(IEntityBuilder* this_, const EntityData* entityData)
{
	return this_->create(entityData);
}

Transform Entity_getTransform(Entity* this_)
{
	Transform transform;
	this_->getTransform(transform);
	return transform;
}

void CameraEntityData_setCameraType(CameraEntityData* this_, const std::wstring& type)
{
	if (compareIgnoreCase< std::wstring >(type, L"orthographic") == 0)
		this_->setCameraType(CtOrthographic);
	else if (compareIgnoreCase< std::wstring >(type, L"perspective") == 0)
		this_->setCameraType(CtPerspective);
}

std::wstring CameraEntityData_getCameraType(CameraEntityData* this_)
{
	switch (this_->getCameraType())
	{
	case CtOrthographic:
		return L"orthographic";
	case CtPerspective:
		return L"perspective";
	}
	return L"";
}

void CameraEntity_setCameraType(CameraEntity* this_, const std::wstring& type)
{
	if (compareIgnoreCase< std::wstring >(type, L"orthographic") == 0)
		this_->setCameraType(CtOrthographic);
	else if (compareIgnoreCase< std::wstring >(type, L"perspective") == 0)
		this_->setCameraType(CtPerspective);
}

std::wstring CameraEntity_getCameraType(CameraEntity* this_)
{
	switch (this_->getCameraType())
	{
	case CtOrthographic:
		return L"orthographic";
	case CtPerspective:
		return L"perspective";
	}
	return L"";
}

RefArray< Entity > GroupEntity_getEntitiesOf(GroupEntity* this_, const TypeInfo& entityType)
{
	RefArray< Entity > entities;
	this_->getEntitiesOf(entityType, entities);
	return entities;
}

Ref< Entity > GroupEntity_getFirstEntityOf(GroupEntity* this_, const TypeInfo& entityType)
{
	return this_->getFirstEntityOf(entityType);
}

void PostProcess_setCombination(PostProcess* this_, const std::wstring& name, bool value)
{
	this_->setCombination(render::getParameterHandle(name), value);
}

void PostProcess_setParameter(PostProcess* this_, const std::wstring& name, float value)
{
	this_->setFloatParameter(render::getParameterHandle(name), value);
}

void Entity_update(Entity* this_, float totalTime, float deltaTime)
{
	UpdateParams up;
	up.totalTime = totalTime;
	up.deltaTime = deltaTime;
	up.alternateTime = totalTime;
	this_->update(up);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldClassFactory", 0, WorldClassFactory, IRuntimeClassFactory)

void WorldClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IEntityEventInstance > > classIEntityEventInstance = new AutoRuntimeClass< IEntityEventInstance >();
	classIEntityEventInstance->addMethod("cancelImmediate", &IEntityEventInstance_cancelImmediate);
	classIEntityEventInstance->addMethod("cancelEnd", &IEntityEventInstance_cancelEnd);
	registrar->registerClass(classIEntityEventInstance);

	Ref< AutoRuntimeClass< IEntityEventData > > classIEntityEventData = new AutoRuntimeClass< IEntityEventData >();
	registrar->registerClass(classIEntityEventData);

	Ref< AutoRuntimeClass< IEntityEvent > > classIEntityEvent = new AutoRuntimeClass< IEntityEvent >();
	registrar->registerClass(classIEntityEvent);

	Ref< AutoRuntimeClass< IEntityEventManager > > classIEntityEventManager = new AutoRuntimeClass< IEntityEventManager >();
	classIEntityEventManager->addMethod("cancelAllImmediate", &IEntityEventManager_cancelAllImmediate);
	classIEntityEventManager->addMethod("cancelAllEnd", &IEntityEventManager_cancelAllEnd);
	registrar->registerClass(classIEntityEventManager);

	Ref< AutoRuntimeClass< IEntitySchema > > classIEntitySchema = new AutoRuntimeClass< IEntitySchema >();
	classIEntitySchema->addMethod< Entity*, uint32_t >("getEntity", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const std::wstring&, uint32_t >("getEntity", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const TypeInfo&, uint32_t >("getEntityOf", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getEntityOf", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, uint32_t >("getChildEntity", &IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, const std::wstring&, uint32_t >("getChildEntity", &IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, const TypeInfo&, uint32_t >("getChildEntityOf", &IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getChildEntityOf", &IEntitySchema::getChildEntity);
	registrar->registerClass(classIEntitySchema);

	Ref< AutoRuntimeClass< EntitySchema > > classEntitySchema = new AutoRuntimeClass< EntitySchema >();
	classEntitySchema->addConstructor();
	registrar->registerClass(classEntitySchema);

	Ref< AutoRuntimeClass< IEntityFactory > > classIEntityFactory = new AutoRuntimeClass< IEntityFactory >();
	registrar->registerClass(classIEntityFactory);

	Ref< AutoRuntimeClass< IEntityBuilder > > classIEntityBuilder = new AutoRuntimeClass< IEntityBuilder >();
	classIEntityBuilder->addMethod("addFactory", &IEntityBuilder::addFactory);
	classIEntityBuilder->addMethod("removeFactory", &IEntityBuilder::removeFactory);
	classIEntityBuilder->addMethod("create", &IEntityBuilder_create);
	registrar->registerClass(classIEntityBuilder);

	Ref< AutoRuntimeClass< EntityBuilder > > classEntityBuilder = new AutoRuntimeClass< EntityBuilder >();
	classEntityBuilder->addConstructor();
	registrar->registerClass(classEntityBuilder);

	Ref< AutoRuntimeClass< EntityBuilderWithSchema > > classEntityBuilderWithSchema = new AutoRuntimeClass< EntityBuilderWithSchema >();
	classEntityBuilderWithSchema->addConstructor< IEntityBuilder*, IEntitySchema* >();
	registrar->registerClass(classEntityBuilderWithSchema);

	Ref< AutoRuntimeClass< IEntityRenderer > > classIEntityRenderer = new AutoRuntimeClass< IEntityRenderer >();
	registrar->registerClass(classIEntityRenderer);

	Ref< AutoRuntimeClass< EntityData > > classEntityData = new AutoRuntimeClass< EntityData >();
	classEntityData->addMethod("setName", &EntityData::setName);
	classEntityData->addMethod("getName", &EntityData::getName);
	classEntityData->addMethod("setTransform", &EntityData::setTransform);
	classEntityData->addMethod("getTransform", &EntityData::getTransform);
	registrar->registerClass(classEntityData);

	Ref< AutoRuntimeClass< Entity > > classEntity = new AutoRuntimeClass< Entity >();
	classEntity->addMethod("destroy", &Entity::destroy);
	classEntity->addMethod("setTransform", &Entity::setTransform);
	classEntity->addMethod("getTransform", &Entity_getTransform);
	classEntity->addMethod("getBoundingBox", &Entity::getBoundingBox);
	classEntity->addMethod("getWorldBoundingBox", &Entity::getWorldBoundingBox);
	classEntity->addMethod("update", &Entity_update);
	registrar->registerClass(classEntity);

	Ref< AutoRuntimeClass< CameraEntityData > > classCameraEntityData = new AutoRuntimeClass< CameraEntityData >();
	classCameraEntityData->addConstructor();
	classCameraEntityData->addMethod("setCameraType", &CameraEntityData_setCameraType);
	classCameraEntityData->addMethod("getCameraType", &CameraEntityData_getCameraType);
	classCameraEntityData->addMethod("setFieldOfView", &CameraEntityData::setFieldOfView);
	classCameraEntityData->addMethod("getFieldOfView", &CameraEntityData::getFieldOfView);
	classCameraEntityData->addMethod("setWidth", &CameraEntityData::setWidth);
	classCameraEntityData->addMethod("getWidth", &CameraEntityData::getWidth);
	classCameraEntityData->addMethod("setHeight", &CameraEntityData::setHeight);
	classCameraEntityData->addMethod("getHeight", &CameraEntityData::getHeight);
	registrar->registerClass(classCameraEntityData);

	Ref< AutoRuntimeClass< CameraEntity > > classCameraEntity = new AutoRuntimeClass< CameraEntity >();
	classCameraEntity->addMethod("setCameraType", &CameraEntity_setCameraType);
	classCameraEntity->addMethod("getCameraType", &CameraEntity_getCameraType);
	classCameraEntity->addMethod("setFieldOfView", &CameraEntity::setFieldOfView);
	classCameraEntity->addMethod("getFieldOfView", &CameraEntity::getFieldOfView);
	classCameraEntity->addMethod("setWidth", &CameraEntity::setWidth);
	classCameraEntity->addMethod("getWidth", &CameraEntity::getWidth);
	classCameraEntity->addMethod("setHeight", &CameraEntity::setHeight);
	classCameraEntity->addMethod("getHeight", &CameraEntity::getHeight);
	registrar->registerClass(classCameraEntity);

	Ref< AutoRuntimeClass< GroupEntity > > classGroupEntity = new AutoRuntimeClass< GroupEntity >();
	classGroupEntity->addConstructor< const Transform& >();
	classGroupEntity->addMethod("addEntity", &GroupEntity::addEntity);
	classGroupEntity->addMethod("removeEntity", &GroupEntity::removeEntity);
	classGroupEntity->addMethod("removeAllEntities", &GroupEntity::removeAllEntities);
	classGroupEntity->addMethod("getEntities", &GroupEntity::getEntities);
	classGroupEntity->addMethod("getEntitiesOf", &GroupEntity_getEntitiesOf);
	classGroupEntity->addMethod("getFirstEntityOf", &GroupEntity_getFirstEntityOf);
	registrar->registerClass(classGroupEntity);

	Ref< AutoRuntimeClass< PointLightEntity > > classPointLightEntity = new AutoRuntimeClass< PointLightEntity >();
	classPointLightEntity->addMethod("getSunColor", &PointLightEntity::getSunColor);
	classPointLightEntity->addMethod("getBaseColor", &PointLightEntity::getBaseColor);
	classPointLightEntity->addMethod("getShadowColor", &PointLightEntity::getShadowColor);
	classPointLightEntity->addMethod("getRange", &PointLightEntity::getRange);
	classPointLightEntity->addMethod("getRandomFlicker", &PointLightEntity::getRandomFlicker);
	registrar->registerClass(classPointLightEntity);

	Ref< AutoRuntimeClass< DirectionalLightEntity > > classDirectionalLightEntity = new AutoRuntimeClass< DirectionalLightEntity >();
	//classDirectionalLightEntity->addConstructor< const Transform&, const Vector4&, const Vector4&, const Vector4&, bool >();
	classDirectionalLightEntity->addMethod("setSunColor", &DirectionalLightEntity::setSunColor);
	classDirectionalLightEntity->addMethod("getSunColor", &DirectionalLightEntity::getSunColor);
	classDirectionalLightEntity->addMethod("setBaseColor", &DirectionalLightEntity::setBaseColor);
	classDirectionalLightEntity->addMethod("getBaseColor", &DirectionalLightEntity::getBaseColor);
	classDirectionalLightEntity->addMethod("setShadowColor", &DirectionalLightEntity::setShadowColor);
	classDirectionalLightEntity->addMethod("getShadowColor", &DirectionalLightEntity::getShadowColor);
	classDirectionalLightEntity->addMethod("setCastShadow", &DirectionalLightEntity::setCastShadow);
	classDirectionalLightEntity->addMethod("getCastShadow", &DirectionalLightEntity::getCastShadow);
	registrar->registerClass(classDirectionalLightEntity);

	Ref< AutoRuntimeClass< NullEntity > > classNullEntity = new AutoRuntimeClass< NullEntity >();
	classNullEntity->addConstructor< const Transform& >();
	registrar->registerClass(classNullEntity);

	Ref< AutoRuntimeClass< SwitchEntity > > classSwitchEntity = new AutoRuntimeClass< SwitchEntity >();
	classSwitchEntity->addConstructor();
	classSwitchEntity->addConstructor< const Transform&, int32_t >();
	classSwitchEntity->addMethod("addEntity", &SwitchEntity::addEntity);
	classSwitchEntity->addMethod("getEntity", &SwitchEntity::getEntity);
	classSwitchEntity->addMethod("setActive", &SwitchEntity::setActive);
	classSwitchEntity->addMethod("getActive", &SwitchEntity::getActive);
	classSwitchEntity->addMethod("getActiveEntity", &SwitchEntity::getActiveEntity);
	registrar->registerClass(classSwitchEntity);

	Ref< AutoRuntimeClass< TransientEntity > > classTransientEntity = new AutoRuntimeClass< TransientEntity >();
	classTransientEntity->addConstructor< GroupEntity*, Entity*, float >();
	registrar->registerClass(classTransientEntity);

	Ref< AutoRuntimeClass< VolumeEntity > > classVolumeEntity = new AutoRuntimeClass< VolumeEntity >();
	classVolumeEntity->addMethod("inside", &VolumeEntity::inside);
	registrar->registerClass(classVolumeEntity);

	Ref< AutoRuntimeClass< PostProcess > > classPostProcess = new AutoRuntimeClass< PostProcess >();
	classPostProcess->addMethod("setCombination", &PostProcess_setCombination);
	classPostProcess->addMethod("setParameter", &PostProcess_setParameter);
	classPostProcess->addMethod("requireHighRange", &PostProcess::requireHighRange);
	registrar->registerClass(classPostProcess);
}

	}
}
