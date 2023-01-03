/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/SkeletonComponentData.h"
#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Runtime/Editor/GameEntityWizardDialog.h"
#include "Script/Editor/Script.h"
#include "Ui/Application.h"
#include "Ui/DropDown.h"
#include "Ui/CheckBox.h"
#include "Ui/Edit.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/MiniButton.h"
#include "World/EntityData.h"
#include "World/Entity/ScriptComponentData.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.GameEntityWizardDialog", GameEntityWizardDialog, ui::ConfigDialog)

GameEntityWizardDialog::GameEntityWizardDialog(editor::IEditor* editor, db::Group* group)
:	m_editor(editor)
,	m_group(group)
,	m_nameEdited(false)
{
}

bool GameEntityWizardDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"GAMEENTITY_WIZARD_DIALOG_TITLE"),
		ui::dpi96(700),
		ui::dpi96(400),
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"*", ui::dpi96(8), ui::dpi96(8))
	))
		return false;

	// Name
	Ref< ui::Container > containerName = new ui::Container();
	containerName->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, ui::dpi96(8)));

	Ref< ui::Static > staticName = new ui::Static();
	staticName->create(containerName, i18n::Text(L"GAMEENTITY_WIZARD_ENTITY_NAME"));

	m_editName = new ui::Edit();
	m_editName->create(containerName, i18n::Text(L"GAMEENTITY_WIZARD_UNNAMED"));
	m_editName->addEventHandler< ui::ContentChangeEvent >(this, &GameEntityWizardDialog::eventNameChange);

	// Visual mesh
	Ref< ui::Container > containerVisualMesh = new ui::Container();
	containerVisualMesh->create(this, ui::WsNone, new ui::TableLayout(L"*,100%,*,*", L"*", 0, ui::dpi96(8)));

	Ref< ui::Static > staticVisualMesh = new ui::Static();
	staticVisualMesh->create(containerVisualMesh, i18n::Text(L"GAMEENTITY_WIZARD_VISUAL_MESH"));

	m_editVisualMesh = new ui::Edit();
	m_editVisualMesh->create(containerVisualMesh, L"");

	Ref< ui::MiniButton > buttonBrowseVisualMesh = new ui::MiniButton();
	buttonBrowseVisualMesh->create(containerVisualMesh, L"...");
	buttonBrowseVisualMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseVisualMeshClick);

	Ref< ui::MiniButton > buttonCopyVisualMesh = new ui::MiniButton();
	buttonCopyVisualMesh->create(containerVisualMesh, L"Copy");
	buttonCopyVisualMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopyVisualMeshClick);

	// Skeleton mesh
	Ref< ui::Container > containerSkeletonMesh = new ui::Container();
	containerSkeletonMesh->create(this, ui::WsNone, new ui::TableLayout(L"*,100%,*,*", L"*", 0, ui::dpi96(8)));

	Ref< ui::Static > staticSkeletonMesh = new ui::Static();
	staticSkeletonMesh->create(containerSkeletonMesh, i18n::Text(L"GAMEENTITY_WIZARD_SKELETON_MESH"));

	m_editSkeletonMesh = new ui::Edit();
	m_editSkeletonMesh->create(containerSkeletonMesh, L"");

	Ref< ui::MiniButton > buttonBrowseSkeletonMesh = new ui::MiniButton();
	buttonBrowseSkeletonMesh->create(containerSkeletonMesh, L"...");
	buttonBrowseSkeletonMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseSkeletonMeshClick);

	Ref< ui::MiniButton > buttonCopySkeletonMesh = new ui::MiniButton();
	buttonCopySkeletonMesh->create(containerSkeletonMesh, L"Copy");
	buttonCopySkeletonMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopySkeletonMeshClick);

	// Animation mesh
	Ref< ui::Container > containerAnimationMesh = new ui::Container();
	containerAnimationMesh->create(this, ui::WsNone, new ui::TableLayout(L"*,100%,*,*", L"*", 0, ui::dpi96(8)));

	Ref< ui::Static > staticAnimationMesh = new ui::Static();
	staticAnimationMesh->create(containerAnimationMesh, i18n::Text(L"GAMEENTITY_WIZARD_ANIMATION_MESH"));

	m_editAnimationMesh = new ui::Edit();
	m_editAnimationMesh->create(containerAnimationMesh, L"");

	Ref< ui::MiniButton > buttonBrowseAnimationMesh = new ui::MiniButton();
	buttonBrowseAnimationMesh->create(containerAnimationMesh, L"...");
	buttonBrowseAnimationMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseAnimationMeshClick);

	Ref< ui::MiniButton > buttonCopyAnimationMesh = new ui::MiniButton();
	buttonCopyAnimationMesh->create(containerAnimationMesh, L"Copy");
	buttonCopyAnimationMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopyAnimationMeshClick);

	// Collision mesh
	Ref< ui::Container > containerCollisionMesh = new ui::Container();
	containerCollisionMesh->create(this, ui::WsNone, new ui::TableLayout(L"*,100%,*,*", L"*", 0, ui::dpi96(8)));

	Ref< ui::Static > staticCollisionMesh = new ui::Static();
	staticCollisionMesh->create(containerCollisionMesh, i18n::Text(L"GAMEENTITY_WIZARD_COLLISION_MESH"));

	m_editCollisionMesh = new ui::Edit();
	m_editCollisionMesh->create(containerCollisionMesh, L"");

	Ref< ui::MiniButton > buttonBrowseCollisionMesh = new ui::MiniButton();
	buttonBrowseCollisionMesh->create(containerCollisionMesh, L"...");
	buttonBrowseCollisionMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseCollisionMeshClick);

	Ref< ui::MiniButton > buttonCopyCollisionMesh = new ui::MiniButton();
	buttonCopyCollisionMesh->create(containerCollisionMesh, L"Copy");
	buttonCopyCollisionMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopyCollisionMeshClick);

	// Type of entity.
	Ref< ui::Container > containerType = new ui::Container();
	containerType->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, ui::dpi96(8)));

	Ref< ui::Static > staticType = new ui::Static();
	staticType->create(containerType, i18n::Text(L"GAMEENTITY_WIZARD_TYPE"));

	m_dropEntityType = new ui::DropDown();
	m_dropEntityType->create(containerType);
	m_dropEntityType->add(i18n::Text(L"GAMEENTITY_WIZARD_TYPE_STATIC"));
	m_dropEntityType->add(i18n::Text(L"GAMEENTITY_WIZARD_TYPE_KINEMATIC"));
	m_dropEntityType->add(i18n::Text(L"GAMEENTITY_WIZARD_TYPE_DYNAMIC"));
	m_dropEntityType->add(i18n::Text(L"GAMEENTITY_WIZARD_TYPE_ANIMATED"));
	m_dropEntityType->add(i18n::Text(L"GAMEENTITY_WIZARD_TYPE_RAGDOLL"));
	m_dropEntityType->select(0);

	// Get all collision specifications in the database.
	RefArray< db::Instance > collisionSpecificationInstances;
	db::recursiveFindChildInstances(m_editor->getSourceDatabase()->getRootGroup(), db::FindInstanceByType(type_of< physics::CollisionSpecification >()), collisionSpecificationInstances);

	Ref< ui::Static > staticCollisionGroup = new ui::Static();
	staticCollisionGroup->create(containerType, i18n::Text(L"GAMEENTITY_WIZARD_COLLISION_GROUP"));

	m_dropCollisionGroup = new ui::DropDown();
	m_dropCollisionGroup->create(containerType);

	for (auto collisionSpecificationInstance : collisionSpecificationInstances)
		m_dropCollisionGroup->add(collisionSpecificationInstance->getName(), collisionSpecificationInstance);

	Ref< ui::Static > staticCollisionMask = new ui::Static();
	staticCollisionMask->create(containerType, i18n::Text(L"GAMEENTITY_WIZARD_COLLISION_MASK"));

	m_dropCollisionMask = new ui::DropDown();
	m_dropCollisionMask->create(containerType, ui::DropDown::WsMultiple);

	for (auto collisionSpecificationInstance : collisionSpecificationInstances)
		m_dropCollisionMask->add(collisionSpecificationInstance->getName(), collisionSpecificationInstance);

	Ref< ui::Static > staticMaterial = new ui::Static();
	staticMaterial->create(containerType, i18n::Text(L"GAMEENTITY_WIZARD_MATERIAL"));

	m_editMaterial = new ui::Edit();
	m_editMaterial->create(containerType, L"0");

	Ref< ui::Static > staticFriction = new ui::Static();
	staticFriction->create(containerType, i18n::Text(L"GAMEENTITY_WIZARD_FRICTION"));

	m_editFriction = new ui::Edit();
	m_editFriction->create(containerType, L"0.75");

	// Script
	m_checkBoxCreateScript = new ui::CheckBox();
	m_checkBoxCreateScript->create(this, i18n::Text(L"GAMEENTITY_WIZARD_CREATE_SCRIPT"));

	addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventDialogClick);
	return true;
}

void GameEntityWizardDialog::eventNameChange(ui::ContentChangeEvent* event)
{
	m_nameEdited = true;
}

void GameEntityWizardDialog::eventBrowseVisualMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"GAMEENTITY_WIZARD_FILE_TITLE"), L"All files;*.*", assetPath))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	FileSystem::getInstance().getRelativePath(
		FileSystem::getInstance().getAbsolutePath(fileName),
		FileSystem::getInstance().getAbsolutePath(assetPath),
		fileName
	);

	m_editVisualMesh->setText(fileName.getPathName());

	if (!m_nameEdited)
		m_editName->setText(fileName.getFileNameNoExtension());
}

void GameEntityWizardDialog::eventCopyVisualMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring path = m_editCollisionMesh->getText();
	m_editVisualMesh->setText(path);

	if (!m_nameEdited)
		m_editName->setText(Path(path).getFileNameNoExtension());
}

void GameEntityWizardDialog::eventBrowseSkeletonMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"GAMEENTITY_WIZARD_FILE_TITLE"), L"All files;*.*"))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	FileSystem::getInstance().getRelativePath(
		FileSystem::getInstance().getAbsolutePath(fileName),
		FileSystem::getInstance().getAbsolutePath(assetPath),
		fileName
	);

	m_editSkeletonMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventCopySkeletonMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring path = m_editVisualMesh->getText();
	m_editSkeletonMesh->setText(path);
}

void GameEntityWizardDialog::eventBrowseAnimationMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"GAMEENTITY_WIZARD_FILE_TITLE"), L"All files;*.*"))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	FileSystem::getInstance().getRelativePath(
		FileSystem::getInstance().getAbsolutePath(fileName),
		FileSystem::getInstance().getAbsolutePath(assetPath),
		fileName
	);

	m_editAnimationMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventCopyAnimationMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring path = m_editVisualMesh->getText();
	m_editAnimationMesh->setText(path);
}

void GameEntityWizardDialog::eventBrowseCollisionMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), i18n::Text(L"GAMEENTITY_WIZARD_FILE_TITLE"), L"All files;*.*", assetPath))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	FileSystem::getInstance().getRelativePath(
		FileSystem::getInstance().getAbsolutePath(fileName),
		FileSystem::getInstance().getAbsolutePath(assetPath),
		fileName
	);

	m_editCollisionMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventCopyCollisionMeshClick(ui::ButtonClickEvent* event)
{
	std::wstring path = m_editVisualMesh->getText();
	m_editCollisionMesh->setText(path);
}

void GameEntityWizardDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if ((ui::DialogResult)event->getCommand().getId() == ui::DialogResult::Ok)
	{
		std::wstring name = m_editName->getText();
		std::wstring visualMesh = m_editVisualMesh->getText();
		std::wstring skeletonMesh = m_editSkeletonMesh->getText();
		std::wstring animationMesh = m_editAnimationMesh->getText();
		std::wstring collisionMesh = m_editCollisionMesh->getText();
		int32_t entityType = m_dropEntityType->getSelected();

		if (name.empty())
		{
			log::error << L"Game entity wizard failed; entity must have a name." << Endl;
			return;
		}

		if ((entityType == 3 || entityType == 4) && skeletonMesh.empty())
		{
			log::error << L"Game entity wizard failed; \"animated\" and \"rag dolls\" must have a skeleton." << Endl;
			return;
		}
		if (entityType == 3 && animationMesh.empty())
		{
			log::error << L"Game entity wizard failed; \"animated\" and \"rag dolls\" must have an animation." << Endl;
			return;
		}

		Ref< world::EntityData > entityData = new world::EntityData();
		entityData->setId(Guid::create());
		entityData->setName(name);

		Ref< db::Instance > skeletonAssetInstance;
		if (!skeletonMesh.empty())
		{
			// Create skeleton mesh asset.
			Ref< animation::SkeletonAsset > skeletonAsset = new animation::SkeletonAsset();
			skeletonAsset->setFileName(skeletonMesh);

			// Create asset instance.
			skeletonAssetInstance = m_group->createInstance(
				name + L"-Skeleton",
				db::CifReplaceExisting | db::CifKeepExistingGuid
			);
			if (!skeletonAssetInstance)
			{
				log::error << L"Game entity wizard failed; unable to create skeleton asset instance." << Endl;
				return;
			}

			skeletonAssetInstance->setObject(skeletonAsset);

			if (!skeletonAssetInstance->commit())
			{
				log::error << L"Game entity wizard failed; unable to commit skeleton asset instance." << Endl;
				return;
			}
		}

		Ref< db::Instance > animationAssetInstance;
		if (!animationMesh.empty())
		{
			// Create animation asset.
			Ref< animation::AnimationAsset > animationAsset = new animation::AnimationAsset();
			animationAsset->setFileName(animationMesh);

			// Create asset instance.
			animationAssetInstance = m_group->createInstance(
				name + L"-Animation",
				db::CifReplaceExisting | db::CifKeepExistingGuid
			);
			if (!animationAssetInstance)
			{
				log::error << L"Game entity wizard failed; unable to create animation asset instance." << Endl;
				return;
			}

			animationAssetInstance->setObject(animationAsset);

			if (!animationAssetInstance->commit())
			{
				log::error << L"Game entity wizard failed; unable to commit animation asset instance." << Endl;
				return;
			}			
		}

		if (!visualMesh.empty())
		{
			// Create visual mesh asset.
			Ref< mesh::MeshAsset > meshAsset = new mesh::MeshAsset();
			meshAsset->setFileName(visualMesh);
			if (entityType == 3 || entityType == 4)
				meshAsset->setMeshType(mesh::MeshAsset::MtSkinned);
			else
				meshAsset->setMeshType(mesh::MeshAsset::MtStatic);

			// Create asset instance.
			Ref< db::Instance > meshAssetInstance = m_group->createInstance(
				name + L"-Visual",
				db::CifReplaceExisting | db::CifKeepExistingGuid
			);
			if (!meshAssetInstance)
			{
				log::error << L"Game entity wizard failed; unable to create visual mesh asset instance." << Endl;
				return;
			}

			meshAssetInstance->setObject(meshAsset);

			if (!meshAssetInstance->commit())
			{
				log::error << L"Game entity wizard failed; unable to commit visual mesh asset instance." << Endl;
				return;
			}

			if (entityType == 3)
			{
				entityData->setComponent(new animation::SkeletonComponentData(
					resource::Id< animation::Skeleton >(skeletonAssetInstance->getGuid()),
					new animation::SimpleAnimationControllerData(
						resource::Id< animation::Animation >(animationAssetInstance->getGuid())
					)
				));
				entityData->setComponent(new animation::AnimatedMeshComponentData(
					resource::Id< mesh::SkinnedMesh >(meshAssetInstance->getGuid())
				));
			}
			else if (entityType == 4)
			{
				std::set< resource::Id< physics::CollisionSpecification > > group;
				std::set< resource::Id< physics::CollisionSpecification > > mask;

				auto groupInstance = m_dropCollisionGroup->getSelectedData< db::Instance >();
				if (groupInstance)
					group.insert(resource::Id< physics::CollisionSpecification >(groupInstance->getGuid()));

				std::vector< int32_t > selectedMasks;
				m_dropCollisionMask->getSelected(selectedMasks);
				for (auto selectedMask : selectedMasks)
				{
					auto maskInstance = m_dropCollisionMask->getData< db::Instance >(selectedMask);
					if (maskInstance)
						mask.insert(resource::Id< physics::CollisionSpecification >(maskInstance->getGuid()));
				}
				
				entityData->setComponent(new animation::SkeletonComponentData(
					resource::Id< animation::Skeleton >(skeletonAssetInstance->getGuid()),
					new animation::RagDollPoseControllerData(group, mask)
				));
				entityData->setComponent(new animation::AnimatedMeshComponentData(
					resource::Id< mesh::SkinnedMesh >(meshAssetInstance->getGuid())
				));
			}
			else
				entityData->setComponent(new mesh::MeshComponentData(
					resource::Id< mesh::IMesh >(meshAssetInstance->getGuid())
				));
		}

		if (!collisionMesh.empty())
		{
			// Create physics mesh asset.
			Ref< physics::MeshAsset > meshAsset = new physics::MeshAsset();
			meshAsset->setFileName(collisionMesh);
			meshAsset->setCalculateConvexHull(entityType == 2);
			meshAsset->setMargin((entityType == 2) ? 0.04f : 0.0f);

			// Create asset instance.
			Ref< db::Instance > meshAssetInstance = m_group->createInstance(
				name + L"-Collision",
				db::CifReplaceExisting | db::CifKeepExistingGuid
			);
			if (!meshAssetInstance)
			{
				log::error << L"Game entity wizard failed; unable to create collision mesh asset instance." << Endl;
				return;
			}

			meshAssetInstance->setObject(meshAsset);

			if (!meshAssetInstance->commit())
			{
				log::error << L"Game entity wizard failed; unable to commit collision mesh asset instance." << Endl;
				return;
			}

			Ref< physics::MeshShapeDesc > meshShapeDesc = new physics::MeshShapeDesc();
			meshShapeDesc->setMesh(resource::Id< physics::Mesh >(meshAssetInstance->getGuid()));

			auto groupInstance = m_dropCollisionGroup->getSelectedData< db::Instance >();
			if (groupInstance)
			{
				std::set< resource::Id< physics::CollisionSpecification > > group;
				group.insert(resource::Id< physics::CollisionSpecification >(groupInstance->getGuid()));
				meshShapeDesc->setCollisionGroup(group);
			}

			std::set< resource::Id< physics::CollisionSpecification > > mask;
			std::vector< int32_t > selectedMasks;
			m_dropCollisionMask->getSelected(selectedMasks);
			for (auto selectedMask : selectedMasks)
			{
				auto maskInstance = m_dropCollisionMask->getData< db::Instance >(selectedMask);
				if (maskInstance)
					mask.insert(resource::Id< physics::CollisionSpecification >(maskInstance->getGuid()));
			}
			meshShapeDesc->setCollisionMask(mask);

			meshShapeDesc->setMaterial(parseString< int32_t >(m_editMaterial->getText()));

			if (entityType == 0 || entityType == 1 || entityType == 4)
			{
				Ref< physics::StaticBodyDesc > bodyDesc = new physics::StaticBodyDesc();
				bodyDesc->setFriction(parseString< float >(m_editFriction->getText()));
				bodyDesc->setKinematic(entityType == 1);
				bodyDesc->setShape(meshShapeDesc);

				Ref< physics::RigidBodyComponentData > componentData = new physics::RigidBodyComponentData();
				entityData->setComponent(new physics::RigidBodyComponentData(bodyDesc));
			}
			else if (entityType == 2)
			{
				Ref< physics::DynamicBodyDesc > bodyDesc = new physics::DynamicBodyDesc();
				bodyDesc->setFriction(parseString< float >(m_editFriction->getText()));
				bodyDesc->setShape(meshShapeDesc);

				Ref< physics::RigidBodyComponentData > componentData = new physics::RigidBodyComponentData();
				entityData->setComponent(new physics::RigidBodyComponentData(bodyDesc));
			}
		}

		// Create script.
		if (m_checkBoxCreateScript->isChecked())
		{
			StringOutputStream ss;
			ss << L"-- " << name << Endl;
			ss << Endl;
			ss << name << L" = " << name << L" or class(\"" << name << L"\", traktor.world.Entity)" << Endl;
			ss << Endl;
			ss << L"function " << name << L":new()" << Endl;
			ss << L"end" << Endl;
			ss << Endl;
			ss << L"function " << name << L":update(totalTime, deltaTime)" << Endl;
			ss << L"end" << Endl;
			ss << Endl;

			Ref< script::Script > s = new script::Script(ss.str());

			// Create database instance.
			Ref< db::Instance > scriptInstance = m_group->createInstance(
				name,
				db::CifReplaceExisting | db::CifKeepExistingGuid
			);
			if (!scriptInstance)
			{
				log::error << L"Game entity wizard failed; unable to create script instance." << Endl;
				return;
			}

			scriptInstance->setObject(s);

			if (!scriptInstance->commit())
			{
				log::error << L"Game entity wizard failed; unable to commit script instance." << Endl;
				return;
			}

			entityData->setComponent(new world::ScriptComponentData(
				resource::Id< IRuntimeClass >(scriptInstance->getGuid())
			));
		}

		Ref< world::EntityData > instanceEntityData = entityData;

		// Create entity asset instance.
		Ref< db::Instance > entityDataInstance = m_group->createInstance(
			name + L"-Entity",
			db::CifReplaceExisting | db::CifKeepExistingGuid
		);
		if (!entityDataInstance)
		{
			log::error << L"Game entity wizard failed; unable to create game entity instance." << Endl;
			return;
		}

		entityDataInstance->setObject(instanceEntityData);

		if (!entityDataInstance->commit())
		{
			log::error << L"Game entity wizard failed; unable to commit game entity instance." << Endl;
			return;
		}

		log::info << L"Game entity created successfully." << Endl;
	}
}

	}
}
