#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedAabb3.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "World/EntityBuilder.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/EntityEventSet.h"
#include "World/EntitySchema.h"
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
#include "World/Entity/GroupComponent.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/LightComponentData.h"
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

void IEntityEventInstance_cancelImmediate(IEntityEventInstance* self)
{
	self->cancel(CtImmediate);
}

void IEntityEventInstance_cancelEnd(IEntityEventInstance* self)
{
	self->cancel(CtEnd);
}

IEntityEventInstance* IEntityEventManager_raise_1(IEntityEventManager* self, const IEntityEvent* event, Entity* sender, const Transform& Toffset)
{
	return self->raise(event, sender, Toffset);
}

IEntityEventInstance* IEntityEventManager_raise_2(IEntityEventManager* self, const EntityEventSet* eventSet, const std::wstring& eventId, Entity* sender, const Transform& Toffset)
{
	return self->raise(eventSet, eventId, sender, Toffset);
}

void IEntityEventManager_cancelAllImmediate(IEntityEventManager* self)
{
	self->cancelAll(CtImmediate);
}

void IEntityEventManager_cancelAllEnd(IEntityEventManager* self)
{
	self->cancelAll(CtEnd);
}

Ref< Entity > IEntityBuilder_create(IEntityBuilder* self, const EntityData* entityData)
{
	return self->create(entityData);
}

void Entity_setTransform(Entity* self, const Transform& transform)
{
	self->setTransform(transform);
}

Transform Entity_getTransform(Entity* self)
{
	Transform transform;
	self->getTransform(transform);
	return transform;
}

void CameraComponentData_setCameraType(CameraComponentData* self, int32_t type)
{
	self->setCameraType((CameraType)type);
}

int32_t CameraComponentData_getCameraType(CameraComponentData* self)
{
	return (int32_t)self->getCameraType();
}

void CameraComponent_setCameraType(CameraComponent* self, int32_t type)
{
	self->setCameraType((CameraType)type);
}

int32_t CameraComponent_getCameraType(CameraComponent* self)
{
	return (int32_t)self->getCameraType();
}

RefArray< Entity > GroupComponent_getEntitiesOf(GroupComponent* self, const TypeInfo& entityType)
{
	RefArray< Entity > entities;
	self->getEntitiesOf(entityType, entities);
	return entities;
}

Ref< Entity > GroupComponent_getFirstEntityOf(GroupComponent* self, const TypeInfo& entityType)
{
	return self->getFirstEntityOf(entityType);
}

RefArray< Entity > GroupEntity_getEntitiesOf(GroupEntity* self, const TypeInfo& entityType)
{
	RefArray< Entity > entities;
	self->getEntitiesOf(entityType, entities);
	return entities;
}

Ref< Entity > GroupEntity_getFirstEntityOf(GroupEntity* self, const TypeInfo& entityType)
{
	return self->getFirstEntityOf(entityType);
}

void Entity_update(Entity* self, float totalTime, float deltaTime)
{
	UpdateParams up;
	up.totalTime = totalTime;
	up.deltaTime = deltaTime;
	up.alternateTime = totalTime;
	self->update(up);
}

IEntityComponentData* ComponentEntityData_getComponent(ComponentEntityData* self, const TypeInfo& componentDataType)
{
	return self->getComponent(componentDataType);
}

IEntityComponent* ComponentEntity_getComponent(ComponentEntity* self, const TypeInfo& componentType)
{
	return self->getComponent(componentType);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldClassFactory", 0, WorldClassFactory, IRuntimeClassFactory)

void WorldClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classIEntityEventInstance = new AutoRuntimeClass< IEntityEventInstance >();
	classIEntityEventInstance->addMethod("cancelImmediate", &IEntityEventInstance_cancelImmediate);
	classIEntityEventInstance->addMethod("cancelEnd", &IEntityEventInstance_cancelEnd);
	registrar->registerClass(classIEntityEventInstance);

	auto classIEntityEventData = new AutoRuntimeClass< IEntityEventData >();
	registrar->registerClass(classIEntityEventData);

	auto classIEntityEvent = new AutoRuntimeClass< IEntityEvent >();
	registrar->registerClass(classIEntityEvent);

	auto classIEntityEventManager = new AutoRuntimeClass< IEntityEventManager >();
	classIEntityEventManager->addMethod("raise", &IEntityEventManager_raise_1);
	classIEntityEventManager->addMethod("raise", &IEntityEventManager_raise_2);
	classIEntityEventManager->addMethod("cancelAllImmediate", &IEntityEventManager_cancelAllImmediate);
	classIEntityEventManager->addMethod("cancelAllEnd", &IEntityEventManager_cancelAllEnd);
	registrar->registerClass(classIEntityEventManager);

	auto classEntityEventSet = new AutoRuntimeClass< EntityEventSet >();
	classEntityEventSet->addMethod("getEvent", &EntityEventSet::getEvent);
	registrar->registerClass(classEntityEventSet);

	auto classIEntitySchema = new AutoRuntimeClass< IEntitySchema >();
	classIEntitySchema->addMethod< Entity*, uint32_t >("getEntity", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const std::wstring&, uint32_t >("getEntity", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const TypeInfo&, uint32_t >("getEntityOf", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getEntityOf", &IEntitySchema::getEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, uint32_t >("getChildEntity", &IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, const std::wstring&, uint32_t >("getChildEntity", &IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, const TypeInfo&, uint32_t >("getChildEntityOf", &IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< Entity*, const Entity*, const std::wstring&, const TypeInfo&, uint32_t >("getChildEntityOf", &IEntitySchema::getChildEntity);
	registrar->registerClass(classIEntitySchema);

	auto classEntitySchema = new AutoRuntimeClass< EntitySchema >();
	classEntitySchema->addConstructor();
	registrar->registerClass(classEntitySchema);

	auto classIEntityFactory = new AutoRuntimeClass< IEntityFactory >();
	registrar->registerClass(classIEntityFactory);

	auto classIEntityBuilder = new AutoRuntimeClass< IEntityBuilder >();
	classIEntityBuilder->addMethod("addFactory", &IEntityBuilder::addFactory);
	classIEntityBuilder->addMethod("removeFactory", &IEntityBuilder::removeFactory);
	classIEntityBuilder->addMethod("create", &IEntityBuilder_create);
	registrar->registerClass(classIEntityBuilder);

	auto classEntityBuilder = new AutoRuntimeClass< EntityBuilder >();
	classEntityBuilder->addConstructor();
	registrar->registerClass(classEntityBuilder);

	auto classEntityBuilderWithSchema = new AutoRuntimeClass< EntityBuilderWithSchema >();
	classEntityBuilderWithSchema->addConstructor< IEntityBuilder*, IEntitySchema* >();
	registrar->registerClass(classEntityBuilderWithSchema);

	auto classIEntityRenderer = new AutoRuntimeClass< IEntityRenderer >();
	registrar->registerClass(classIEntityRenderer);

	auto classEntityData = new AutoRuntimeClass< EntityData >();
	classEntityData->addProperty("name", &EntityData::setName, &EntityData::getName);
	classEntityData->addProperty("transform", &EntityData::setTransform, &EntityData::getTransform);
	registrar->registerClass(classEntityData);

	auto classEntity = new AutoRuntimeClass< Entity >();
	classEntity->addProperty("transform", &Entity_setTransform, &Entity_getTransform);
	classEntity->addProperty("boundingBox", &Entity::getBoundingBox);
	classEntity->addProperty("worldBoundingBox", &Entity::getWorldBoundingBox);
	classEntity->addMethod("destroy", &Entity::destroy);
	classEntity->addMethod("update", &Entity_update);
	registrar->registerClass(classEntity);

	auto classGroupComponent = new AutoRuntimeClass< GroupComponent >();
	classGroupComponent->addConstructor();
	classGroupComponent->addMethod("addEntity", &GroupComponent::addEntity);
	classGroupComponent->addMethod("removeEntity", &GroupComponent::removeEntity);
	classGroupComponent->addMethod("removeAllEntities", &GroupComponent::removeAllEntities);
	classGroupComponent->addMethod("getEntities", &GroupComponent::getEntities);
	classGroupComponent->addMethod("getEntitiesOf", &GroupComponent_getEntitiesOf);
	classGroupComponent->addMethod("getFirstEntityOf", &GroupComponent_getFirstEntityOf);
	registrar->registerClass(classGroupComponent);

	auto classGroupEntity = new AutoRuntimeClass< GroupEntity >();
	classGroupEntity->addConstructor();
	classGroupEntity->addConstructor< const Transform&, uint32_t >();
	classGroupEntity->addMethod("addEntity", &GroupEntity::addEntity);
	classGroupEntity->addMethod("removeEntity", &GroupEntity::removeEntity);
	classGroupEntity->addMethod("removeAllEntities", &GroupEntity::removeAllEntities);
	classGroupEntity->addMethod("getEntities", &GroupEntity::getEntities);
	classGroupEntity->addMethod("getEntitiesOf", &GroupEntity_getEntitiesOf);
	classGroupEntity->addMethod("getFirstEntityOf", &GroupEntity_getFirstEntityOf);
	registrar->registerClass(classGroupEntity);

	auto classIEntityComponentData = new AutoRuntimeClass< IEntityComponentData >();
	registrar->registerClass(classIEntityComponentData);

	auto classIEntityComponent = new AutoRuntimeClass< IEntityComponent >();
	classIEntityComponent->addProperty("boundingBox", &IEntityComponent::getBoundingBox);
	classIEntityComponent->addMethod("setTransform", &IEntityComponent::setTransform);
	registrar->registerClass(classIEntityComponent);

	auto classCameraComponentData = new AutoRuntimeClass< CameraComponentData >();
	classCameraComponentData->addConstant("CtOrthographic", Any::fromInt32(CameraType::CtOrthographic));
	classCameraComponentData->addConstant("CtPerspective", Any::fromInt32(CameraType::CtPerspective));
	classCameraComponentData->addConstructor();
	classCameraComponentData->addProperty("cameraType", &CameraComponentData_setCameraType, &CameraComponentData_getCameraType);
	classCameraComponentData->addProperty("fieldOfView", &CameraComponentData::setFieldOfView, &CameraComponentData::getFieldOfView);
	classCameraComponentData->addProperty("width", &CameraComponentData::setWidth, &CameraComponentData::getWidth);
	classCameraComponentData->addProperty("height", &CameraComponentData::setHeight, &CameraComponentData::getHeight);
	registrar->registerClass(classCameraComponentData);

	auto classCameraComponent = new AutoRuntimeClass< CameraComponent >();
	classCameraComponent->addConstant("CtOrthographic", Any::fromInt32(CameraType::CtOrthographic));
	classCameraComponent->addConstant("CtPerspective", Any::fromInt32(CameraType::CtPerspective));
	classCameraComponent->addProperty("cameraType", &CameraComponent_setCameraType, &CameraComponent_getCameraType);
	classCameraComponent->addProperty("fieldOfView", &CameraComponent::setFieldOfView, &CameraComponent::getFieldOfView);
	classCameraComponent->addProperty("width", &CameraComponent::setWidth, &CameraComponent::getWidth);
	classCameraComponent->addProperty("height", &CameraComponent::setHeight, &CameraComponent::getHeight);
	registrar->registerClass(classCameraComponent);

	auto classLightComponentData = new AutoRuntimeClass< LightComponentData >();
	classLightComponentData->addProperty("color", &LightComponentData::setColor, &LightComponentData::getColor);
	classLightComponentData->addProperty("castShadow", &LightComponentData::setCastShadow, &LightComponentData::getCastShadow);
	classLightComponentData->addProperty("range", &LightComponentData::setRange, &LightComponentData::getRange);
	classLightComponentData->addProperty("radius", &LightComponentData::setRadius, &LightComponentData::getRadius);
	classLightComponentData->addProperty("flickerAmount", &LightComponentData::setFlickerAmount, &LightComponentData::getFlickerAmount);
	classLightComponentData->addProperty("flickerFilter", &LightComponentData::setFlickerFilter, &LightComponentData::getFlickerFilter);
	registrar->registerClass(classLightComponentData);

	auto classLightComponent = new AutoRuntimeClass< LightComponent >();
	classLightComponent->addProperty("color", &LightComponent::setColor, &LightComponent::getColor);
	classLightComponent->addProperty("castShadow", &LightComponent::setCastShadow, &LightComponent::getCastShadow);
	classLightComponent->addProperty("range", &LightComponent::setRange, &LightComponent::getRange);
	classLightComponent->addProperty("radius", &LightComponent::setRadius, &LightComponent::getRadius);
	classLightComponent->addProperty("flickerAmount", &LightComponent::setFlickerAmount, &LightComponent::getFlickerAmount);
	classLightComponent->addProperty("flickerFilter", &LightComponent::setFlickerFilter, &LightComponent::getFlickerFilter);
	registrar->registerClass(classLightComponent);

	auto classScriptComponentData = new AutoRuntimeClass< ScriptComponentData >();
	registrar->registerClass(classScriptComponentData);

	auto classScriptComponent = new AutoRuntimeClass< ScriptComponent >();
	registrar->registerClass(classScriptComponent);

	auto classVolumeComponentData = new AutoRuntimeClass< VolumeComponentData >();
	registrar->registerClass(classVolumeComponentData);

	auto classVolumeComponent = new AutoRuntimeClass< VolumeComponent >();
	classVolumeComponent->addMethod("inside", &VolumeComponent::inside);
	registrar->registerClass(classVolumeComponent);

	auto classComponentEntityData = new AutoRuntimeClass< ComponentEntityData >();
	classComponentEntityData->addConstructor();
	classComponentEntityData->addMethod("setComponent", &ComponentEntityData::setComponent);
	classComponentEntityData->addMethod("getComponent", &ComponentEntityData_getComponent);
	registrar->registerClass(classComponentEntityData);

	auto classComponentEntity = new AutoRuntimeClass< ComponentEntity >();
	classComponentEntity->addConstructor();
	classComponentEntity->addConstructor< const Transform& >();
	classComponentEntity->addMethod("setComponent", &ComponentEntity::setComponent);
	classComponentEntity->addMethod("getComponent", &ComponentEntity_getComponent);
	registrar->registerClass(classComponentEntity);

	auto classIWorldRenderer = new AutoRuntimeClass< IWorldRenderer >();
	classIWorldRenderer->addConstant("QuDisabled", Any::fromInt32(Quality::QuDisabled));
	classIWorldRenderer->addConstant("QuLow", Any::fromInt32(Quality::QuLow));
	classIWorldRenderer->addConstant("QuMedium", Any::fromInt32(Quality::QuMedium));
	classIWorldRenderer->addConstant("QuHigh", Any::fromInt32(Quality::QuHigh));
	classIWorldRenderer->addConstant("QuUltra", Any::fromInt32(Quality::QuUltra));
	registrar->registerClass(classIWorldRenderer);
}

	}
}
