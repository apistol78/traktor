/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Misc/String.h"
#include "World/EntityBuilder.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/EntityData.h"
#include "World/EntityEventSet.h"
#include "World/EntitySchema.h"
#include "World/IEntityComponent.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"
#include "World/IEntityEventInstance.h"
#include "World/IEntityEventManager.h"
#include "World/IEntityFactory.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldClassFactory.h"
#include "World/Entity/CameraComponent.h"
#include "World/Entity/CameraComponentData.h"
#include "World/Entity/ComponentEntity.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/ScriptComponent.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Entity/VolumeComponent.h"
#include "World/Entity/VolumeComponentData.h"

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

IEntityEventInstance* IEntityEventManager_raise_1(IEntityEventManager* this_, const IEntityEvent* event, Entity* sender, const Transform& Toffset)
{
	return this_->raise(event, sender, Toffset);
}

IEntityEventInstance* IEntityEventManager_raise_2(IEntityEventManager* this_, const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset)
{
	return this_->raise(eventSet, eventId, sender, Toffset);
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

void Entity_setTransform(Entity* this_, const Transform& transform)
{
	this_->setTransform(transform);
}

Transform Entity_getTransform(Entity* this_)
{
	Transform transform;
	this_->getTransform(transform);
	return transform;
}

void CameraComponentData_setCameraType(CameraComponentData* this_, int32_t type)
{
	this_->setCameraType((CameraType)type);
}

int32_t CameraComponentData_getCameraType(CameraComponentData* this_)
{
	return (int32_t)this_->getCameraType();
}

void CameraComponent_setCameraType(CameraComponent* this_, int32_t type)
{
	this_->setCameraType((CameraType)type);
}

int32_t CameraComponent_getCameraType(CameraComponent* this_)
{
	return (int32_t)this_->getCameraType();
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

void Entity_update(Entity* this_, float totalTime, float deltaTime)
{
	UpdateParams up;
	up.totalTime = totalTime;
	up.deltaTime = deltaTime;
	up.alternateTime = totalTime;
	this_->update(up);
}

IEntityComponentData* ComponentEntityData_getComponent(ComponentEntityData* this_, const TypeInfo& componentDataType)
{
	return this_->getComponent(componentDataType);
}

IEntityComponent* ComponentEntity_getComponent(ComponentEntity* this_, const TypeInfo& componentType)
{
	return this_->getComponent(componentType);
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
	classIEntityEventManager->addMethod("raise", &IEntityEventManager_raise_1);
	classIEntityEventManager->addMethod("raise", &IEntityEventManager_raise_2);
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
	classEntityData->addProperty("name", &EntityData::setName, &EntityData::getName);
	classEntityData->addProperty("transform", &EntityData::setTransform, &EntityData::getTransform);
	registrar->registerClass(classEntityData);

	Ref< AutoRuntimeClass< Entity > > classEntity = new AutoRuntimeClass< Entity >();
	classEntity->addProperty("transform", &Entity_setTransform, &Entity_getTransform);
	classEntity->addProperty("boundingBox", &Entity::getBoundingBox);
	classEntity->addProperty("worldBoundingBox", &Entity::getWorldBoundingBox);
	classEntity->addMethod("destroy", &Entity::destroy);
	classEntity->addMethod("update", &Entity_update);
	registrar->registerClass(classEntity);

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
	classPointLightEntity->addProperty("sunColor", &PointLightEntity::getSunColor);
	classPointLightEntity->addProperty("baseColor", &PointLightEntity::getBaseColor);
	classPointLightEntity->addProperty("shadowColor", &PointLightEntity::getShadowColor);
	classPointLightEntity->addProperty("range", &PointLightEntity::getRange);
	classPointLightEntity->addProperty("randomFlicker", &PointLightEntity::getRandomFlicker);
	registrar->registerClass(classPointLightEntity);

	Ref< AutoRuntimeClass< DirectionalLightEntity > > classDirectionalLightEntity = new AutoRuntimeClass< DirectionalLightEntity >();
	//classDirectionalLightEntity->addConstructor< const Transform&, const Vector4&, const Vector4&, const Vector4&, bool >();
	classDirectionalLightEntity->addProperty("sunColor", &DirectionalLightEntity::setSunColor, &DirectionalLightEntity::getSunColor);
	classDirectionalLightEntity->addProperty("baseColor", &DirectionalLightEntity::setBaseColor, &DirectionalLightEntity::getBaseColor);
	classDirectionalLightEntity->addProperty("shadowColor", &DirectionalLightEntity::setShadowColor, &DirectionalLightEntity::getShadowColor);
	classDirectionalLightEntity->addProperty("castShadow", &DirectionalLightEntity::setCastShadow, &DirectionalLightEntity::getCastShadow);
	registrar->registerClass(classDirectionalLightEntity);

	Ref< AutoRuntimeClass< IEntityComponentData > > classIEntityComponentData = new AutoRuntimeClass< IEntityComponentData >();
	registrar->registerClass(classIEntityComponentData);

	Ref< AutoRuntimeClass< IEntityComponent > > classIEntityComponent = new AutoRuntimeClass< IEntityComponent >();
	classIEntityComponent->addProperty("boundingBox", &IEntityComponent::getBoundingBox);
	classIEntityComponent->addMethod("setTransform", &IEntityComponent::setTransform);
	registrar->registerClass(classIEntityComponent);

	Ref< AutoRuntimeClass< CameraComponentData > > classCameraComponentData = new AutoRuntimeClass< CameraComponentData >();
	classCameraComponentData->addConstant("CtOrthographic", Any::fromInt32(CtOrthographic));
	classCameraComponentData->addConstant("CtPerspective", Any::fromInt32(CtPerspective));
	classCameraComponentData->addConstructor();
	classCameraComponentData->addProperty("cameraType", &CameraComponentData_setCameraType, &CameraComponentData_getCameraType);
	classCameraComponentData->addProperty("fieldOfView", &CameraComponentData::setFieldOfView, &CameraComponentData::getFieldOfView);
	classCameraComponentData->addProperty("width", &CameraComponentData::setWidth, &CameraComponentData::getWidth);
	classCameraComponentData->addProperty("height", &CameraComponentData::setHeight, &CameraComponentData::getHeight);
	registrar->registerClass(classCameraComponentData);

	Ref< AutoRuntimeClass< CameraComponent > > classCameraComponent = new AutoRuntimeClass< CameraComponent >();
	classCameraComponent->addConstant("CtOrthographic", Any::fromInt32(CtOrthographic));
	classCameraComponent->addConstant("CtPerspective", Any::fromInt32(CtPerspective));
	classCameraComponent->addProperty("cameraType", &CameraComponent_setCameraType, &CameraComponent_getCameraType);
	classCameraComponent->addProperty("fieldOfView", &CameraComponent::setFieldOfView, &CameraComponent::getFieldOfView);
	classCameraComponent->addProperty("width", &CameraComponent::setWidth, &CameraComponent::getWidth);
	classCameraComponent->addProperty("height", &CameraComponent::setHeight, &CameraComponent::getHeight);
	registrar->registerClass(classCameraComponent);

	Ref< AutoRuntimeClass< LightComponentData > > classLightComponentData = new AutoRuntimeClass< LightComponentData >();
	classLightComponentData->addProperty("sunColor", &LightComponentData::setSunColor, &LightComponentData::getSunColor);
	classLightComponentData->addProperty("baseColor", &LightComponentData::setBaseColor, &LightComponentData::getBaseColor);
	classLightComponentData->addProperty("shadowColor", &LightComponentData::setShadowColor, &LightComponentData::getShadowColor);
	classLightComponentData->addProperty("castShadow", &LightComponentData::setCastShadow, &LightComponentData::getCastShadow);
	classLightComponentData->addProperty("range", &LightComponentData::setRange, &LightComponentData::getRange);
	classLightComponentData->addProperty("radius", &LightComponentData::setRadius, &LightComponentData::getRadius);
	classLightComponentData->addProperty("flickerAmount", &LightComponentData::setFlickerAmount, &LightComponentData::getFlickerAmount);
	classLightComponentData->addProperty("flickerFilter", &LightComponentData::setFlickerFilter, &LightComponentData::getFlickerFilter);
	registrar->registerClass(classLightComponentData);

	Ref< AutoRuntimeClass< LightComponent > > classLightComponent = new AutoRuntimeClass< LightComponent >();
	classLightComponent->addProperty("sunColor", &LightComponent::setSunColor, &LightComponent::getSunColor);
	classLightComponent->addProperty("baseColor", &LightComponent::setBaseColor, &LightComponent::getBaseColor);
	classLightComponent->addProperty("shadowColor", &LightComponent::setShadowColor, &LightComponent::getShadowColor);
	classLightComponent->addProperty("castShadow", &LightComponent::setCastShadow, &LightComponent::getCastShadow);
	classLightComponent->addProperty("range", &LightComponent::setRange, &LightComponent::getRange);
	classLightComponent->addProperty("radius", &LightComponent::setRadius, &LightComponent::getRadius);
	classLightComponent->addProperty("flickerAmount", &LightComponent::setFlickerAmount, &LightComponent::getFlickerAmount);
	classLightComponent->addProperty("flickerFilter", &LightComponent::setFlickerFilter, &LightComponent::getFlickerFilter);
	registrar->registerClass(classLightComponent);

	Ref< AutoRuntimeClass< ScriptComponentData > > classScriptComponentData = new AutoRuntimeClass< ScriptComponentData >();
	registrar->registerClass(classScriptComponentData);

	Ref< AutoRuntimeClass< ScriptComponent > > classScriptComponent = new AutoRuntimeClass< ScriptComponent >();
	registrar->registerClass(classScriptComponent);

	Ref< AutoRuntimeClass< VolumeComponentData > > classVolumeComponentData = new AutoRuntimeClass< VolumeComponentData >();
	registrar->registerClass(classVolumeComponentData);

	Ref< AutoRuntimeClass< VolumeComponent > > classVolumeComponent = new AutoRuntimeClass< VolumeComponent >();
	classVolumeComponent->addMethod("inside", &VolumeComponent::inside);
	registrar->registerClass(classVolumeComponent);

	Ref< AutoRuntimeClass< ComponentEntityData > > classComponentEntityData = new AutoRuntimeClass< ComponentEntityData >();
	classComponentEntityData->addConstructor();
	classComponentEntityData->addMethod("setComponent", &ComponentEntityData::setComponent);
	classComponentEntityData->addMethod("getComponent", &ComponentEntityData_getComponent);
	registrar->registerClass(classComponentEntityData);

	Ref< AutoRuntimeClass< ComponentEntity > > classComponentEntity = new AutoRuntimeClass< ComponentEntity >();
	classComponentEntity->addConstructor();
	classComponentEntity->addConstructor< const Transform& >();
	classComponentEntity->addMethod("setComponent", &ComponentEntity::setComponent);
	classComponentEntity->addMethod("getComponent", &ComponentEntity_getComponent);
	registrar->registerClass(classComponentEntity);

	Ref< AutoRuntimeClass< IWorldRenderer > > classIWorldRenderer = new AutoRuntimeClass< IWorldRenderer >();
	classIWorldRenderer->addConstant("QuDisabled", Any::fromInt32(QuDisabled));
	classIWorldRenderer->addConstant("QuLow", Any::fromInt32(QuLow));
	classIWorldRenderer->addConstant("QuMedium", Any::fromInt32(QuMedium));
	classIWorldRenderer->addConstant("QuHigh", Any::fromInt32(QuHigh));
	classIWorldRenderer->addConstant("QuUltra", Any::fromInt32(QuUltra));
	registrar->registerClass(classIWorldRenderer);
}

	}
}
