/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityBuilder.h"
#include "World/Entity/CameraComponent.h"
#include "World/Entity/CameraComponentData.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/Entity/DecalComponent.h"
#include "World/Entity/DecalComponentData.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventData.h"
#include "World/Entity/EventSetComponent.h"
#include "World/Entity/EventSetComponentData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/FacadeComponent.h"
#include "World/Entity/FacadeComponentData.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/LightComponent.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/OccluderComponent.h"
#include "World/Entity/OccluderComponentData.h"
#include "World/Entity/PathComponent.h"
#include "World/Entity/PathComponentData.h"
#include "World/Entity/PersistentIdComponent.h"
#include "World/Entity/PersistentIdComponentData.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Entity/ProbeComponentData.h"
#include "World/Entity/ScriptComponent.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Entity/VolumeComponent.h"
#include "World/Entity/VolumeComponentData.h"
#include "World/Entity/VolumetricFogComponent.h"
#include "World/Entity/VolumetricFogComponentData.h"
#include "World/Entity/WorldEntityFactory.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldEntityFactory", WorldEntityFactory, IEntityFactory)

WorldEntityFactory::WorldEntityFactory(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	EntityEventManager* eventManager,
	bool editor
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_eventManager(eventManager)
,	m_editor(editor)
{
}

const TypeInfoSet WorldEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< EntityData >();
	typeSet.insert< ExternalEntityData >();
	return typeSet;
}

const TypeInfoSet WorldEntityFactory::getEntityEventTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< DecalEventData >();
	return typeSet;
}

const TypeInfoSet WorldEntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< CameraComponentData >();
	typeSet.insert< DecalComponentData >();
	typeSet.insert< EventSetComponentData >();
	typeSet.insert< FacadeComponentData >();
	typeSet.insert< GroupComponentData >();
	typeSet.insert< LightComponentData >();
	typeSet.insert< OccluderComponentData >();
	typeSet.insert< PathComponentData >();
	typeSet.insert< PersistentIdComponentData >();
	typeSet.insert< ProbeComponentData >();
	typeSet.insert< ScriptComponentData >();
	typeSet.insert< VolumeComponentData >();
	typeSet.insert< VolumetricFogComponentData >();
	return typeSet;
}

Ref< Entity > WorldEntityFactory::createEntity(const IEntityBuilder* builder, const EntityData& entityData) const
{
	if (auto externalEntityData = dynamic_type_cast< const ExternalEntityData* >(&entityData))
	{
		resource::Proxy< EntityData > resolvedEntityData;
		if (!m_resourceManager->bind(externalEntityData->getEntityData(), resolvedEntityData))
			return nullptr;

		Ref< EntityData > mutableEntityData = DeepClone(resolvedEntityData.getResource()).create< EntityData >();
		if (!mutableEntityData)
			return nullptr;

		mutableEntityData->setId(externalEntityData->getId());
		mutableEntityData->setName(externalEntityData->getName());
		mutableEntityData->setTransform(externalEntityData->getTransform());
		mutableEntityData->setState(externalEntityData->getState());

		// Override component data.
		for (auto componentData : entityData.getComponents())
			mutableEntityData->setComponent(componentData);

		return builder->create(mutableEntityData);
	}
	else
	{
		// Sort component data so they get added in expected order.
		RefArray< IEntityComponentData > componentDatas = entityData.getComponents();
		componentDatas.sort([](IEntityComponentData* lh, IEntityComponentData* rh) {
			return lh->getOrdinal() < rh->getOrdinal();
		});

		// Instantiate all components.
		RefArray< IEntityComponent > components;
		for (auto componentData : componentDatas)
		{
			if (!componentData)
				continue;

			Ref< IEntityComponent > component = builder->create(componentData);
			if (!component)
			{
				if (!m_editor)
					return nullptr;
				else
					continue;
			}

			components.push_back(component);
		}

		// Create entity and attach all components to it.
		return new Entity(entityData.getName(), entityData.getTransform(), entityData.getState(), components);
	}
	return nullptr;
}

Ref< IEntityEvent > WorldEntityFactory::createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const
{
	if (auto decalData = dynamic_type_cast< const DecalEventData* >(&entityEventData))
	{
		Ref< DecalEvent > decal = new DecalEvent();

		decal->m_size = decalData->getSize();
		decal->m_thickness = decalData->getThickness();
		decal->m_alpha = decalData->getAlpha();
		decal->m_cullDistance = decalData->getCullDistance();

		if (m_resourceManager->bind(decalData->getShader(), decal->m_shader))
			return decal;
	}
	return nullptr;
}

Ref< IEntityComponent > WorldEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const
{
	if (auto cameraComponentData = dynamic_type_cast< const CameraComponentData* >(&entityComponentData))
		return new CameraComponent(cameraComponentData);

	if (auto decalComponentData = dynamic_type_cast< const DecalComponentData* >(&entityComponentData))
	{
		resource::Proxy< render::Shader > shader;
		if (!m_resourceManager->bind(decalComponentData->getShader(), shader))
			return nullptr;

		Ref< DecalComponent > decalComponent = new DecalComponent(
			decalComponentData->getSize(),
			decalComponentData->getThickness(),
			decalComponentData->getAlpha(),
			decalComponentData->getCullDistance(),
			shader
		);

		return decalComponent;
	}

	if (auto eventSetComponentData = dynamic_type_cast< const EventSetComponentData* >(&entityComponentData))
	{
		return eventSetComponentData->createComponent(m_eventManager, builder);
	}

	if (auto facadeComponentData = dynamic_type_cast< const FacadeComponentData* >(&entityComponentData))
	{
		Ref< FacadeComponent > facadeComponent = new FacadeComponent();
		//if (!facadeComponentData->getShow().empty())
		//	facadeComponent->show(facadeComponentData->getShow());
		return facadeComponent;
	}

	if (auto groupComponentData = dynamic_type_cast< const GroupComponentData* >(&entityComponentData))
	{
		Ref< GroupComponent > groupComponent = new GroupComponent();
		for (auto entityData : groupComponentData->getEntityData())
		{
			Ref< Entity > childEntity = builder->create(entityData);
			if (childEntity)
				groupComponent->addEntity(childEntity);
		}
		return groupComponent;
	}

	if (auto lightComponentData = dynamic_type_cast<const LightComponentData*>(&entityComponentData))
	{
		return new LightComponent(
			lightComponentData->getLightType(),
			lightComponentData->getColor() * Scalar(lightComponentData->getIntensity()),
			lightComponentData->getCastShadow(),
			lightComponentData->getNearRange(),
			lightComponentData->getFarRange(),
			lightComponentData->getRadius(),
			lightComponentData->getFlickerAmount(),
			lightComponentData->getFlickerFilter()
		);
	}

	if (auto occluderComponentData = dynamic_type_cast< const OccluderComponentData* >(&entityComponentData))
	{
		return occluderComponentData->createComponent();
	}

	if (auto pathComponentData = dynamic_type_cast< const PathComponentData* >(&entityComponentData))
	{
		return new PathComponent(
			pathComponentData->getPath()
		);
	}

	if (auto persistentIdComponentData = dynamic_type_cast< const PersistentIdComponentData* >(&entityComponentData))
	{
		return new PersistentIdComponent(
			persistentIdComponentData->getId()
		);
	}

	if (auto probeComponentData = dynamic_type_cast< const ProbeComponentData* >(&entityComponentData))
	{
		resource::Proxy< render::ITexture > texture;
		bool dirty = false;

		if (probeComponentData->getTexture())
		{
			if (!m_resourceManager->bind(probeComponentData->getTexture(), texture))
				return nullptr;
		}
		else
		{
			render::CubeTextureCreateDesc ctcd;
#if !defined(__ANDROID__) && !defined(__IOS__)
			ctcd.side = 1024;
#else
			ctcd.side = 256;
#endif
			ctcd.mipCount = (int32_t)(log2(ctcd.side ) + 1.0f);
#if !defined(__ANDROID__)
			ctcd.format = render::TfR16G16B16A16F;
#else
			ctcd.format = render::TfR8G8B8A8;
#endif
			ctcd.sRGB = false;
			ctcd.immutable = false;

			texture = resource::Proxy< render::ITexture >(m_renderSystem->createCubeTexture(ctcd, T_FILE_LINE_W));
			if (!texture)
				return nullptr;

			dirty = true;
		}

		return new ProbeComponent(
			texture,
			probeComponentData->getIntensity(),
			probeComponentData->getVolume(),
			probeComponentData->getLocal(),
			probeComponentData->getTexture().isNull(),	// Probes with no specified texture should capture it's surrounding dynamically.
			dirty
		);
	}

	if (auto scriptComponentData = dynamic_type_cast< const ScriptComponentData* >(&entityComponentData))
	{
		if (!m_editor || scriptComponentData->getEditorSupport())
			return scriptComponentData->createComponent(m_resourceManager);
		else
			return nullptr;
	}

	if (auto volumeComponentData = dynamic_type_cast< const VolumeComponentData* >(&entityComponentData))
		return new VolumeComponent(volumeComponentData);

	if (auto volumetricFogComponentData = dynamic_type_cast< const VolumetricFogComponentData* >(&entityComponentData))
		return volumetricFogComponentData->createComponent(m_resourceManager, m_renderSystem);

	return nullptr;
}

}
