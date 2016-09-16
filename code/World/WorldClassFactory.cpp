#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Misc/String.h"
#include "World/EntityBuilder.h"
#include "World/EntityBuilderWithSchema.h"
#include "World/EntityData.h"
#include "World/EntitySchema.h"
#include "World/IEntityComponent.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"
#include "World/IEntityEventInstance.h"
#include "World/IEntityEventManager.h"
#include "World/IEntityFactory.h"
#include "World/IEntityRenderer.h"
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

void CameraComponentData_setCameraType(CameraComponentData* this_, const std::wstring& type)
{
	if (compareIgnoreCase< std::wstring >(type, L"orthographic") == 0)
		this_->setCameraType(CtOrthographic);
	else if (compareIgnoreCase< std::wstring >(type, L"perspective") == 0)
		this_->setCameraType(CtPerspective);
}

std::wstring CameraComponentData_getCameraType(CameraComponentData* this_)
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

void CameraComponent_setCameraType(CameraComponent* this_, const std::wstring& type)
{
	if (compareIgnoreCase< std::wstring >(type, L"orthographic") == 0)
		this_->setCameraType(CtOrthographic);
	else if (compareIgnoreCase< std::wstring >(type, L"perspective") == 0)
		this_->setCameraType(CtPerspective);
}

std::wstring CameraComponent_getCameraType(CameraComponent* this_)
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

	Ref< AutoRuntimeClass< IEntityComponentData > > classIEntityComponentData = new AutoRuntimeClass< IEntityComponentData >();
	registrar->registerClass(classIEntityComponentData);

	Ref< AutoRuntimeClass< IEntityComponent > > classIEntityComponent = new AutoRuntimeClass< IEntityComponent >();
	registrar->registerClass(classIEntityComponent);

	Ref< AutoRuntimeClass< CameraComponentData > > classCameraComponentData = new AutoRuntimeClass< CameraComponentData >();
	classCameraComponentData->addConstructor();
	classCameraComponentData->addMethod("setCameraType", &CameraComponentData_setCameraType);
	classCameraComponentData->addMethod("getCameraType", &CameraComponentData_getCameraType);
	classCameraComponentData->addMethod("setFieldOfView", &CameraComponentData::setFieldOfView);
	classCameraComponentData->addMethod("getFieldOfView", &CameraComponentData::getFieldOfView);
	classCameraComponentData->addMethod("setWidth", &CameraComponentData::setWidth);
	classCameraComponentData->addMethod("getWidth", &CameraComponentData::getWidth);
	classCameraComponentData->addMethod("setHeight", &CameraComponentData::setHeight);
	classCameraComponentData->addMethod("getHeight", &CameraComponentData::getHeight);
	registrar->registerClass(classCameraComponentData);

	Ref< AutoRuntimeClass< CameraComponent > > classCameraComponent = new AutoRuntimeClass< CameraComponent >();
	classCameraComponent->addMethod("setCameraType", &CameraComponent_setCameraType);
	classCameraComponent->addMethod("getCameraType", &CameraComponent_getCameraType);
	classCameraComponent->addMethod("setFieldOfView", &CameraComponent::setFieldOfView);
	classCameraComponent->addMethod("getFieldOfView", &CameraComponent::getFieldOfView);
	classCameraComponent->addMethod("setWidth", &CameraComponent::setWidth);
	classCameraComponent->addMethod("getWidth", &CameraComponent::getWidth);
	classCameraComponent->addMethod("setHeight", &CameraComponent::setHeight);
	classCameraComponent->addMethod("getHeight", &CameraComponent::getHeight);
	registrar->registerClass(classCameraComponent);

	Ref< AutoRuntimeClass< LightComponentData > > classLightComponentData = new AutoRuntimeClass< LightComponentData >();
	classLightComponentData->addMethod("setSunColor", &LightComponentData::setSunColor);
	classLightComponentData->addMethod("getSunColor", &LightComponentData::getSunColor);
	classLightComponentData->addMethod("setBaseColor", &LightComponentData::setBaseColor);
	classLightComponentData->addMethod("getBaseColor", &LightComponentData::getBaseColor);
	classLightComponentData->addMethod("setShadowColor", &LightComponentData::setShadowColor);
	classLightComponentData->addMethod("getShadowColor", &LightComponentData::getShadowColor);
	classLightComponentData->addMethod("setCastShadow", &LightComponentData::setCastShadow);
	classLightComponentData->addMethod("getCastShadow", &LightComponentData::getCastShadow);
	classLightComponentData->addMethod("setRange", &LightComponentData::setRange);
	classLightComponentData->addMethod("getRange", &LightComponentData::getRange);
	classLightComponentData->addMethod("setRadius", &LightComponentData::setRadius);
	classLightComponentData->addMethod("getRadius", &LightComponentData::getRadius);
	classLightComponentData->addMethod("setFlickerAmount", &LightComponentData::setFlickerAmount);
	classLightComponentData->addMethod("getFlickerAmount", &LightComponentData::getFlickerAmount);
	classLightComponentData->addMethod("setFlickerFilter", &LightComponentData::setFlickerFilter);
	classLightComponentData->addMethod("getFlickerFilter", &LightComponentData::getFlickerFilter);
	registrar->registerClass(classLightComponentData);

	Ref< AutoRuntimeClass< LightComponent > > classLightComponent = new AutoRuntimeClass< LightComponent >();
	classLightComponent->addMethod("setSunColor", &LightComponent::setSunColor);
	classLightComponent->addMethod("getSunColor", &LightComponent::getSunColor);
	classLightComponent->addMethod("setBaseColor", &LightComponent::setBaseColor);
	classLightComponent->addMethod("getBaseColor", &LightComponent::getBaseColor);
	classLightComponent->addMethod("setShadowColor", &LightComponent::setShadowColor);
	classLightComponent->addMethod("getShadowColor", &LightComponent::getShadowColor);
	classLightComponent->addMethod("setCastShadow", &LightComponent::setCastShadow);
	classLightComponent->addMethod("getCastShadow", &LightComponent::getCastShadow);
	classLightComponent->addMethod("setRange", &LightComponent::setRange);
	classLightComponent->addMethod("getRange", &LightComponent::getRange);
	classLightComponent->addMethod("setRadius", &LightComponent::setRadius);
	classLightComponent->addMethod("getRadius", &LightComponent::getRadius);
	classLightComponent->addMethod("setFlickerAmount", &LightComponent::setFlickerAmount);
	classLightComponent->addMethod("getFlickerAmount", &LightComponent::getFlickerAmount);
	classLightComponent->addMethod("setFlickerFilter", &LightComponent::setFlickerFilter);
	classLightComponent->addMethod("getFlickerFilter", &LightComponent::getFlickerFilter);
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
}

	}
}
