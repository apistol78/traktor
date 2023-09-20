/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"
#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Editor/EntityPipeline.h"

namespace traktor::world
{
	namespace
	{

bool removeMember(RfmCompound* owner, ReflectionMember* member)
{
	if (owner->removeMember(member))
		return true;

	// Unable to remove from this compound, recurse with each member compound.
	for (auto c : owner->getMembers())
	{
		if (auto cc = dynamic_type_cast< RfmCompound* >(c))
		{
			if (removeMember(cc, member))
				return true;
		}
	}

	return false;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityPipeline", 1, EntityPipeline, editor::IPipeline)

bool EntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_editor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false);
	return true;
}

void EntityPipeline::destroy()
{
}

TypeInfoSet EntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EntityData >();
	typeSet.insert< IEntityEventData >();
	typeSet.insert< IEntityComponentData >();
	return typeSet;
}

bool EntityPipeline::shouldCache() const
{
	return false;
}

uint32_t EntityPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool EntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return false;

	// Find all members which reference child entities.
	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< const RfmObject > objectMember = checked_type_cast< const RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const EntityData* entityData = dynamic_type_cast< const EntityData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityData);
		}
		else if (const IEntityEventData* entityEventData = dynamic_type_cast< const IEntityEventData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityEventData);
		}
		else if (const IEntityComponentData* entityComponentData = dynamic_type_cast< const IEntityComponentData* >(objectMember->get()))
		{
			pipelineDepends->addDependency(entityComponentData);
		}
		else if (objectMember->get())
		{
			Ref< Reflection > childReflection = Reflection::create(objectMember->get());
			if (childReflection)
				childReflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
		}
	}

	return true;
}

bool EntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< ISerializable > outputAsset = pipelineBuilder->buildProduct(sourceInstance, sourceAsset, buildParams);
	if (!outputAsset)
		return false;

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
		return false;

	outputInstance->setObject(outputAsset);

	if (!outputInstance->commit())
		return false;

	return true;
}

Ref< ISerializable > EntityPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (auto entityData = dynamic_type_cast< const EntityData* >(sourceAsset))
	{
		// Create a clone of the entity and replace components with built components.
		Ref< EntityData > mutableEntityData = DeepClone(entityData).create< EntityData >();
		RefArray< IEntityComponentData > components = mutableEntityData->getComponents();

		// Sort components by ordinal to ensure they are built in predictable order.
		components.sort([](IEntityComponentData* lh, IEntityComponentData* rh)
			{
				return lh->getOrdinal() < rh->getOrdinal();
			}
		);

		for (size_t i = 0; i < components.size(); )
		{
			Ref< ISerializable > product = pipelineBuilder->buildProduct(sourceInstance, components[i], mutableEntityData);
			if (auto replaceEntityData = dynamic_type_cast< EntityData* >(product))
			{
				// Keep the new set of components and restart the loop.
				components = replaceEntityData->getComponents();

				// Once again sort components by ordinal to ensure they are built in predictable order.
				components.sort([](IEntityComponentData* lh, IEntityComponentData* rh)
					{
						return lh->getOrdinal() < rh->getOrdinal();
					}
				);

				i = 0;
			}
			else if (auto replaceComponentData = dynamic_type_cast< IEntityComponentData* >(product))
			{
				// Replace component with built component.
				components[i] = replaceComponentData;
				++i;
			}
			else
			{
				// Remove component all together.
				components.erase(components.begin() + (int32_t)i);
			}

			// Replace components in entity data being built to enure it's always
			// propagated when building rest of the components.
			mutableEntityData->setComponents(components);
		}

		return mutableEntityData;
	}
	else if (auto componentData = dynamic_type_cast< const IEntityComponentData* >(sourceAsset))
	{
		auto ownerEntityData = mandatory_non_null_type_cast< const EntityData* >(buildParams);

		// Remove editor only component data.
		if (is_a< EditorAttributesComponentData >(componentData))
			return nullptr;

		// Create a reflection of the component data.
		Ref< Reflection > reflection = Reflection::create(sourceAsset);
		if (!reflection)
			return nullptr;

		RefArray< ReflectionMember > objectMembers;
		reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
		while (!objectMembers.empty())
		{
			Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
			objectMembers.pop_front();

			if (auto entityData = dynamic_type_cast< const EntityData* >(objectMember->get()))
			{
				// Build entity trough pipeline; replace entity with product.
				Ref< ISerializable > product = pipelineBuilder->buildProduct(sourceInstance, entityData);
				objectMember->set(product);
			}
			else if (auto entityEventData = dynamic_type_cast< const IEntityEventData* >(objectMember->get()))
			{
				// Build event trough pipeline; replace event with product.
				Ref< ISerializable > product = pipelineBuilder->buildProduct(sourceInstance, entityEventData, ownerEntityData);
				objectMember->set(product);
			}
			else if (objectMember->get())
			{
				// Scan recursively through object references; add to member list.
				Ref< Reflection > childReflection = Reflection::create(objectMember->get());
				if (childReflection)
					childReflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
			}
		}

		return reflection->clone(); 
	}
	else if (auto eventData = dynamic_type_cast< const IEntityEventData* >(sourceAsset))
	{
		return nullptr;
	}

	T_FATAL_ERROR;
	return nullptr;

	/*
	// Create a reflection of the source asset, aka the entity data.
	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return nullptr;

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	// Remove editor attributes component; can not be used in runtime.
	if (!m_editor)
	{
		for (;;)
		{
			auto it = std::find_if(objectMembers.begin(), objectMembers.end(), [](ReflectionMember* member) {
				return is_a< EditorAttributesComponentData >(static_cast< RfmObject* >(member)->get());
			});
			if (it != objectMembers.end())
			{
				ReflectionMember* member = *it;

				auto attributes = static_cast< const EditorAttributesComponentData* >(static_cast< RfmObject* >(member)->get());
				if (!attributes->include)
					return nullptr;

				removeMember(reflection, member);
				objectMembers.erase(it);
			}
			else
				break;
		}
	}

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (auto entityData = dynamic_type_cast< const EntityData* >(objectMember->get()))
		{
			// Build entity trough pipeline; replace entity with product.
			Ref< ISerializable > product = pipelineBuilder->buildProduct(sourceInstance, entityData);
			objectMember->set(product);
		}
		else if (auto entityComponentData = dynamic_type_cast< const IEntityComponentData* >(objectMember->get()))
		{
			// Build component trough pipeline; replace component with product.
			auto ownerEntityData = mandatory_non_null_type_cast< const EntityData* >(sourceAsset);

			Ref< ISerializable > product = pipelineBuilder->buildProduct(sourceInstance, entityComponentData, ownerEntityData);
			if (auto replaceEntityData = dynamic_type_cast< EntityData* >(product))
			{
			}
			else if (auto replaceComponentData = dynamic_type_cast< IEntityComponentData* >(product))
			{
				objectMember->set(replaceComponentData);
			}
			else
			{
				const bool result = reflection->removeMember(objectMember);
				T_FATAL_ASSERT(result);
			}
		}
		else if (auto entityEventData = dynamic_type_cast< const IEntityEventData* >(objectMember->get()))
		{
			// Build event trough pipeline; replace event with product.
			auto ownerEntityData = mandatory_non_null_type_cast< const EntityData* >(sourceAsset);

			Ref< ISerializable > product = pipelineBuilder->buildProduct(sourceInstance, entityEventData, ownerEntityData);
			objectMember->set(product);
		}
		else if (objectMember->get())
		{
			// Scan recursively through object references; add to member list.
			Ref< Reflection > childReflection = Reflection::create(objectMember->get());
			if (childReflection)
				childReflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
		}
	}

	return reflection->clone();
	*/
}

}
