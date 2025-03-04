/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/GameEntityWizardDialog.h"

#include "Animation/AnimatedMeshComponentData.h"
#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Animation/SkeletonComponentData.h"
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
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/MeshComponentData.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Script/Editor/Script.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/CheckBox.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/MessageBox.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/EntityData.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.GameEntityWizardDialog", GameEntityWizardDialog, ui::ConfigDialog)

GameEntityWizardDialog::GameEntityWizardDialog(editor::IEditor* editor, db::Group* group)
	: m_editor(editor)
	, m_group(group)
	, m_nameEdited(false)
{
}

bool GameEntityWizardDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
			parent,
			i18n::Text(L"GAMEENTITY_WIZARD_DIALOG_TITLE"),
			700_ut,
			450_ut,
			ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
			new ui::TableLayout(L"100%", L"*", 8_ut, 8_ut)))
		return false;

	// Name
	Ref< ui::Container > containerName = new ui::Container();
	containerName->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 8_ut));

	Ref< ui::Static > staticName = new ui::Static();
	staticName->create(containerName, i18n::Text(L"GAMEENTITY_WIZARD_ENTITY_NAME"));

	m_editName = new ui::Edit();
	m_editName->create(containerName, i18n::Text(L"GAMEENTITY_WIZARD_UNNAMED"));
	m_editName->addEventHandler< ui::ContentChangeEvent >(this, &GameEntityWizardDialog::eventNameChange);

	// Visual mesh
	Ref< ui::Container > containerMesh = new ui::Container();
	containerMesh->create(this, ui::WsNone, new ui::TableLayout(L"*,100%,*,*", L"*", 0_ut, 8_ut));

	Ref< ui::Static > staticVisualMesh = new ui::Static();
	staticVisualMesh->create(containerMesh, i18n::Text(L"GAMEENTITY_WIZARD_VISUAL_MESH"));

	m_editVisualMesh = new ui::Edit();
	m_editVisualMesh->create(containerMesh, L"");

	Ref< ui::Button > buttonBrowseVisualMesh = new ui::Button();
	buttonBrowseVisualMesh->create(containerMesh, L"...");
	buttonBrowseVisualMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseVisualMeshClick);

	Ref< ui::Button > buttonCopyVisualMesh = new ui::Button();
	buttonCopyVisualMesh->create(containerMesh, L"Copy");
	buttonCopyVisualMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopyVisualMeshClick);

	// Skeleton mesh
	Ref< ui::Static > staticSkeletonMesh = new ui::Static();
	staticSkeletonMesh->create(containerMesh, i18n::Text(L"GAMEENTITY_WIZARD_SKELETON_MESH"));

	m_editSkeletonMesh = new ui::Edit();
	m_editSkeletonMesh->create(containerMesh, L"");

	Ref< ui::Button > buttonBrowseSkeletonMesh = new ui::Button();
	buttonBrowseSkeletonMesh->create(containerMesh, L"...");
	buttonBrowseSkeletonMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseSkeletonMeshClick);

	Ref< ui::Button > buttonCopySkeletonMesh = new ui::Button();
	buttonCopySkeletonMesh->create(containerMesh, L"Copy");
	buttonCopySkeletonMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopySkeletonMeshClick);

	// Animation mesh
	Ref< ui::Static > staticAnimationMesh = new ui::Static();
	staticAnimationMesh->create(containerMesh, i18n::Text(L"GAMEENTITY_WIZARD_ANIMATION_MESH"));

	m_editAnimationMesh = new ui::Edit();
	m_editAnimationMesh->create(containerMesh, L"");

	Ref< ui::Button > buttonBrowseAnimationMesh = new ui::Button();
	buttonBrowseAnimationMesh->create(containerMesh, L"...");
	buttonBrowseAnimationMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseAnimationMeshClick);

	Ref< ui::Button > buttonCopyAnimationMesh = new ui::Button();
	buttonCopyAnimationMesh->create(containerMesh, L"Copy");
	buttonCopyAnimationMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopyAnimationMeshClick);

	// Collision mesh
	Ref< ui::Static > staticCollisionMesh = new ui::Static();
	staticCollisionMesh->create(containerMesh, i18n::Text(L"GAMEENTITY_WIZARD_COLLISION_MESH"));

	m_editCollisionMesh = new ui::Edit();
	m_editCollisionMesh->create(containerMesh, L"");

	Ref< ui::Button > buttonBrowseCollisionMesh = new ui::Button();
	buttonBrowseCollisionMesh->create(containerMesh, L"...");
	buttonBrowseCollisionMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseCollisionMeshClick);

	Ref< ui::Button > buttonCopyCollisionMesh = new ui::Button();
	buttonCopyCollisionMesh->create(containerMesh, L"Copy");
	buttonCopyCollisionMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventCopyCollisionMeshClick);

	// Type of entity.
	Ref< ui::Container > containerType = new ui::Container();
	containerType->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 8_ut));

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
	m_editMaterial->create(containerType, L"0", ui::WsNone, new ui::NumericEditValidator(false, 0));

	Ref< ui::Static > staticFriction = new ui::Static();
	staticFriction->create(containerType, i18n::Text(L"GAMEENTITY_WIZARD_FRICTION"));

	m_editFriction = new ui::Edit();
	m_editFriction->create(containerType, L"0.75", ui::WsNone, new ui::NumericEditValidator(true, 0.0f));

	Ref< ui::Static > staticScale = new ui::Static();
	staticScale->create(containerType, i18n::Text(L"GAMEENTITY_WIZARD_SCALE"));

	m_editScale = new ui::Edit();
	m_editScale->create(containerType, L"1", ui::WsNone, new ui::NumericEditValidator(true, 0.0f));

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
	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

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
		fileName);

	m_editVisualMesh->setText(fileName.getPathName());

	if (!m_nameEdited)
		m_editName->setText(fileName.getFileNameNoExtension());
}

void GameEntityWizardDialog::eventCopyVisualMeshClick(ui::ButtonClickEvent* event)
{
	const std::wstring path = m_editCollisionMesh->getText();
	m_editVisualMesh->setText(path);

	if (!m_nameEdited)
		m_editName->setText(Path(path).getFileNameNoExtension());
}

void GameEntityWizardDialog::eventBrowseSkeletonMeshClick(ui::ButtonClickEvent* event)
{
	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

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
		fileName);

	m_editSkeletonMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventCopySkeletonMeshClick(ui::ButtonClickEvent* event)
{
	const std::wstring path = m_editVisualMesh->getText();
	m_editSkeletonMesh->setText(path);
}

void GameEntityWizardDialog::eventBrowseAnimationMeshClick(ui::ButtonClickEvent* event)
{
	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

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
		fileName);

	m_editAnimationMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventCopyAnimationMeshClick(ui::ButtonClickEvent* event)
{
	const std::wstring path = m_editVisualMesh->getText();
	m_editAnimationMesh->setText(path);
}

void GameEntityWizardDialog::eventBrowseCollisionMeshClick(ui::ButtonClickEvent* event)
{
	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

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
		fileName);

	m_editCollisionMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventCopyCollisionMeshClick(ui::ButtonClickEvent* event)
{
	const std::wstring path = m_editVisualMesh->getText();
	m_editCollisionMesh->setText(path);
}

void GameEntityWizardDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if ((ui::DialogResult)event->getCommand().getId() == ui::DialogResult::Ok)
	{
		const std::wstring name = m_editName->getText();
		const std::wstring visualMesh = m_editVisualMesh->getText();
		const std::wstring skeletonMesh = m_editSkeletonMesh->getText();
		const std::wstring animationMesh = m_editAnimationMesh->getText();
		const std::wstring collisionMesh = m_editCollisionMesh->getText();
		const int32_t entityType = m_dropEntityType->getSelected();
		const float scale = parseString< float >(m_editScale->getText());

		// Sanity check input.
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

		// Warn if we're about to replace existing instances.
		bool haveSkeleton = false;
		bool haveAnimation = false;
		bool haveVisualMesh = false;
		bool haveCollisionMesh = false;
		bool haveScript = false;
		bool haveEntity = false;

		if (!skeletonMesh.empty())
			haveSkeleton = (m_group->getInstance(name + L"-Skeleton") != nullptr);
		if (!animationMesh.empty())
			haveAnimation = (m_group->getInstance(name + L"-Animation") != nullptr);
		if (!visualMesh.empty())
			haveVisualMesh = (m_group->getInstance(name + L"-Visual") != nullptr);
		if (!collisionMesh.empty())
			haveCollisionMesh = (m_group->getInstance(name + L"-Collision") != nullptr);
		if (m_checkBoxCreateScript->isChecked())
			haveScript = (m_group->getInstance(name + L"-Script") != nullptr);

		haveEntity = (m_group->getInstance(name) != nullptr);

		const bool haveAny = (haveSkeleton || haveAnimation || haveVisualMesh || haveCollisionMesh || haveScript || haveEntity);
		if (haveAny)
		{
			StringOutputStream ss;
			ss << i18n::Text(L"GAMEENTITY_WIZARD_ALREADY_EXIST_MESSAGE") << Endl;

			if (haveSkeleton)
				ss << name + L"-Skeleton" << Endl;
			if (haveAnimation)
				ss << name + L"-Animation" << Endl;
			if (haveVisualMesh)
				ss << name + L"-Visual" << Endl;
			if (haveCollisionMesh)
				ss << name + L"-Collision" << Endl;
			if (haveScript)
				ss << name + L"-Script" << Endl;
			if (haveEntity)
				ss << name << Endl;

			if (ui::MessageBox::show(this, ss.str(), i18n::Text(L"GAMEENTITY_WIZARD_ALREADY_EXIST_CAPTION"), ui::MbIconExclamation | ui::MbYesNo) == ui::DialogResult::No)
			{
				event->consume();
				return;
			}
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
			skeletonAsset->setScale(scale);

			// Create asset instance.
			skeletonAssetInstance = m_group->createInstance(
				name + L"-Skeleton",
				db::CifReplaceExisting | db::CifKeepExistingGuid);
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
			animationAsset->setScale(scale);

			// Create asset instance.
			animationAssetInstance = m_group->createInstance(
				name + L"-Animation",
				db::CifReplaceExisting | db::CifKeepExistingGuid);
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
			meshAsset->setScaleFactor(scale);
			if (entityType == 3 || entityType == 4)
				meshAsset->setMeshType(mesh::MeshAsset::MtSkinned);
			else
				meshAsset->setMeshType(mesh::MeshAsset::MtStatic);

			// Create asset instance.
			Ref< db::Instance > meshAssetInstance = m_group->createInstance(
				name + L"-Visual",
				db::CifReplaceExisting | db::CifKeepExistingGuid);
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
						resource::Id< animation::Animation >(animationAssetInstance->getGuid()))));
				entityData->setComponent(new animation::AnimatedMeshComponentData(
					resource::Id< mesh::SkinnedMesh >(meshAssetInstance->getGuid())));
			}
			else if (entityType == 4)
			{
				SmallSet< resource::Id< physics::CollisionSpecification > > group;
				SmallSet< resource::Id< physics::CollisionSpecification > > mask;

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
					new animation::RagDollPoseControllerData(group, mask)));
				entityData->setComponent(new animation::AnimatedMeshComponentData(
					resource::Id< mesh::SkinnedMesh >(meshAssetInstance->getGuid())));
			}
			else
				entityData->setComponent(new mesh::MeshComponentData(
					resource::Id< mesh::IMesh >(meshAssetInstance->getGuid())));
		}

		if (!collisionMesh.empty())
		{
			// Create physics mesh asset.
			Ref< physics::MeshAsset > meshAsset = new physics::MeshAsset();
			meshAsset->setFileName(collisionMesh);
			meshAsset->setScaleFactor(scale);
			meshAsset->setCalculateConvexHull(entityType == 2);
			meshAsset->setMargin((entityType == 2) ? 0.04f : 0.0f);

			// Create asset instance.
			Ref< db::Instance > meshAssetInstance = m_group->createInstance(
				name + L"-Collision",
				db::CifReplaceExisting | db::CifKeepExistingGuid);
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
				SmallSet< resource::Id< physics::CollisionSpecification > > group;
				group.insert(resource::Id< physics::CollisionSpecification >(groupInstance->getGuid()));
				meshShapeDesc->setCollisionGroup(group);
			}

			SmallSet< resource::Id< physics::CollisionSpecification > > mask;
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
				name + L"-Script",
				db::CifReplaceExisting | db::CifKeepExistingGuid);
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
				resource::Id< IRuntimeClass >(scriptInstance->getGuid())));
		}

		Ref< world::EntityData > instanceEntityData = entityData;

		// Create entity asset instance.
		Ref< db::Instance > entityDataInstance = m_group->createInstance(
			name,
			db::CifReplaceExisting | db::CifKeepExistingGuid);
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
