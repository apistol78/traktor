#include "Parade/Classes/WorldClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"
#include "World/PostProcess/PostProcess.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/EntityBuilder.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/EntitySchema.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/IEntityFactory.h"
#include "World/Entity/IEntityRenderer.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/TransientEntity.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

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

void world_PostProcess_setParameter(world::PostProcess* this_, const std::wstring& name, float value)
{
	this_->setParameter(render::getParameterHandle(name), value);
}

		}

void registerWorldClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< world::IEntitySchema > > classIEntitySchema = new script::AutoScriptClass< world::IEntitySchema >();
	classIEntitySchema->addMethod< world::Entity*, uint32_t >(L"getEntity", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const std::wstring&, uint32_t >(L"getEntity", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const TypeInfo&, uint32_t >(L"getEntityOf", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const std::wstring&, const TypeInfo&, uint32_t >(L"getEntityOf", &world::IEntitySchema::getEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, uint32_t >(L"getChildEntity", &world::IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, const std::wstring&, uint32_t >(L"getChildEntity", &world::IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, const TypeInfo&, uint32_t >(L"getChildEntityOf", &world::IEntitySchema::getChildEntity);
	classIEntitySchema->addMethod< world::Entity*, const world::Entity*, const std::wstring&, const TypeInfo&, uint32_t >(L"getChildEntityOf", &world::IEntitySchema::getChildEntity);
	scriptManager->registerClass(classIEntitySchema);

	Ref< script::AutoScriptClass< world::EntitySchema > > classEntitySchema = new script::AutoScriptClass< world::EntitySchema >();
	classEntitySchema->addConstructor();
	scriptManager->registerClass(classEntitySchema);

	Ref< script::AutoScriptClass< world::IEntityFactory > > classIEntityFactory = new script::AutoScriptClass< world::IEntityFactory >();
	scriptManager->registerClass(classIEntityFactory);

	Ref< script::AutoScriptClass< world::IEntityBuilder > > classIEntityBuilder = new script::AutoScriptClass< world::IEntityBuilder >();
	classIEntityBuilder->addMethod(L"addFactory", &world::IEntityBuilder::addFactory);
	classIEntityBuilder->addMethod(L"removeFactory", &world::IEntityBuilder::removeFactory);
	classIEntityBuilder->addMethod(L"beginBuild", &world::IEntityBuilder::begin);
	classIEntityBuilder->addMethod(L"create", &world::IEntityBuilder::create);
	classIEntityBuilder->addMethod(L"get", &world::IEntityBuilder::get);
	classIEntityBuilder->addMethod(L"endBuild", &world::IEntityBuilder::end);
	scriptManager->registerClass(classIEntityBuilder);

	Ref< script::AutoScriptClass< world::EntityBuilder > > classEntityBuilder = new script::AutoScriptClass< world::EntityBuilder >();
	classEntityBuilder->addConstructor();
	scriptManager->registerClass(classEntityBuilder);

	Ref< script::AutoScriptClass< world::IEntityRenderer > > classIEntityRenderer = new script::AutoScriptClass< world::IEntityRenderer >();
	scriptManager->registerClass(classIEntityRenderer);

	Ref< script::AutoScriptClass< world::EntityData > > classEntityData = new script::AutoScriptClass< world::EntityData >();
	classEntityData->addMethod(L"getName", &world::EntityData::getName);
	classEntityData->addMethod(L"setTransform", &world::EntityData::setTransform);
	classEntityData->addMethod(L"getTransform", &world::EntityData::getTransform);
	scriptManager->registerClass(classEntityData);

	Ref< script::AutoScriptClass< world::Entity > > classEntity = new script::AutoScriptClass< world::Entity >();
	classEntity->addMethod(L"destroy", &world::Entity::destroy);
	classEntity->addMethod(L"setTransform", &world::Entity::setTransform);
	classEntity->addMethod(L"getTransform", &world_Entity_getTransform);
	scriptManager->registerClass(classEntity);

	Ref< script::AutoScriptClass< world::GroupEntity > > classGroupEntity = new script::AutoScriptClass< world::GroupEntity >();
	classGroupEntity->addConstructor< const Transform& >();
	classGroupEntity->addMethod(L"addEntity", &world::GroupEntity::addEntity);
	classGroupEntity->addMethod(L"removeEntity", &world::GroupEntity::removeEntity);
	classGroupEntity->addMethod(L"removeAllEntities", &world::GroupEntity::removeAllEntities);
	classGroupEntity->addMethod(L"getEntities", &world::GroupEntity::getEntities);
	classGroupEntity->addMethod(L"getEntitiesOf", &world_GroupEntity_getEntitiesOf);
	classGroupEntity->addMethod(L"getFirstEntityOf", &world_GroupEntity_getFirstEntityOf);
	scriptManager->registerClass(classGroupEntity);

	Ref< script::AutoScriptClass< world::PointLightEntity > > classPointLightEntity = new script::AutoScriptClass< world::PointLightEntity >();
	classPointLightEntity->addMethod(L"getSunColor", &world::PointLightEntity::getSunColor);
	classPointLightEntity->addMethod(L"getBaseColor", &world::PointLightEntity::getBaseColor);
	classPointLightEntity->addMethod(L"getShadowColor", &world::PointLightEntity::getShadowColor);
	classPointLightEntity->addMethod(L"getRange", &world::PointLightEntity::getRange);
	classPointLightEntity->addMethod(L"getRandomFlicker", &world::PointLightEntity::getRandomFlicker);
	scriptManager->registerClass(classPointLightEntity);

	Ref< script::AutoScriptClass< world::DirectionalLightEntity > > classDirectionalLightEntity = new script::AutoScriptClass< world::DirectionalLightEntity >();
	classDirectionalLightEntity->addConstructor< const Transform&, const Vector4&, const Vector4&, const Vector4&, bool >();
	classDirectionalLightEntity->addMethod(L"setSunColor", &world::DirectionalLightEntity::setSunColor);
	classDirectionalLightEntity->addMethod(L"getSunColor", &world::DirectionalLightEntity::getSunColor);
	classDirectionalLightEntity->addMethod(L"setBaseColor", &world::DirectionalLightEntity::setBaseColor);
	classDirectionalLightEntity->addMethod(L"getBaseColor", &world::DirectionalLightEntity::getBaseColor);
	classDirectionalLightEntity->addMethod(L"setShadowColor", &world::DirectionalLightEntity::setShadowColor);
	classDirectionalLightEntity->addMethod(L"getShadowColor", &world::DirectionalLightEntity::getShadowColor);
	classDirectionalLightEntity->addMethod(L"setCastShadow", &world::DirectionalLightEntity::setCastShadow);
	classDirectionalLightEntity->addMethod(L"getCastShadow", &world::DirectionalLightEntity::getCastShadow);
	scriptManager->registerClass(classDirectionalLightEntity);

	Ref< script::AutoScriptClass< world::NullEntity > > classNullEntity = new script::AutoScriptClass< world::NullEntity >();
	classNullEntity->addConstructor< const Transform& >();
	scriptManager->registerClass(classNullEntity);

	Ref< script::AutoScriptClass< world::TransientEntity > > classTransientEntity = new script::AutoScriptClass< world::TransientEntity >();
	classTransientEntity->addConstructor< world::GroupEntity*, world::Entity*, float >();
	scriptManager->registerClass(classTransientEntity);

	Ref< script::AutoScriptClass< world::PostProcess > > classPostProcess = new script::AutoScriptClass< world::PostProcess >();
	classPostProcess->addMethod(L"setParameter", &world_PostProcess_setParameter);
	classPostProcess->addMethod(L"requireHighRange", &world::PostProcess::requireHighRange);
	scriptManager->registerClass(classPostProcess);
}

	}
}
