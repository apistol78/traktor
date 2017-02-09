#include "Amalgam/Editor/GameEntityWizardDialog.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Script/Editor/Script.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/DropDown.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/MiniButton.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ScriptComponentData.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.GameEntityWizardDialog", GameEntityWizardDialog, ui::ConfigDialog)

GameEntityWizardDialog::GameEntityWizardDialog(editor::IEditor* editor, db::Group* group)
:	m_editor(editor)
,	m_group(group)
{
}

bool GameEntityWizardDialog::create(ui::Widget* parent)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"GAMEENTITY_WIZARD_DIALOG_TITLE"),
		ui::scaleBySystemDPI(700),
		ui::scaleBySystemDPI(400),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"*", ui::scaleBySystemDPI(8), ui::scaleBySystemDPI(8))
	))
		return false;

	// Name
	Ref< ui::Container > containerName = new ui::Container();
	containerName->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, ui::scaleBySystemDPI(8)));

	Ref< ui::Static > staticName = new ui::Static();
	staticName->create(containerName, i18n::Text(L"GAMEENTITY_WIZARD_ENTITY_NAME"));

	m_editName = new ui::Edit();
	m_editName->create(containerName, i18n::Text(L"GAMEENTITY_WIZARD_UNNAMED"));

	// Visual mesh
	Ref< ui::Container > containerVisualMesh = new ui::Container();
	containerVisualMesh->create(this, ui::WsNone, new ui::TableLayout(L"*,100%,*", L"*", 0, ui::scaleBySystemDPI(8)));

	Ref< ui::Static > staticVisualMesh = new ui::Static();
	staticVisualMesh->create(containerVisualMesh, i18n::Text(L"GAMEENTITY_WIZARD_VISUAL_MESH"));

	m_editVisualMesh = new ui::Edit();
	m_editVisualMesh->create(containerVisualMesh, L"");

	Ref< ui::custom::MiniButton > buttonBrowseVisualMesh = new ui::custom::MiniButton();
	buttonBrowseVisualMesh->create(containerVisualMesh, L"...");
	buttonBrowseVisualMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseVisualMeshClick);

	// Collision mesh
	Ref< ui::Container > containerCollisionMesh = new ui::Container();
	containerCollisionMesh->create(this, ui::WsNone, new ui::TableLayout(L"*,100%,*", L"*", 0, ui::scaleBySystemDPI(8)));

	Ref< ui::Static > staticCollisionMesh = new ui::Static();
	staticCollisionMesh->create(containerCollisionMesh, i18n::Text(L"GAMEENTITY_WIZARD_COLLISION_MESH"));

	m_editCollisionMesh = new ui::Edit();
	m_editCollisionMesh->create(containerCollisionMesh, L"");

	Ref< ui::custom::MiniButton > buttonBrowseCollisionMesh = new ui::custom::MiniButton();
	buttonBrowseCollisionMesh->create(containerCollisionMesh, L"...");
	buttonBrowseCollisionMesh->addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventBrowseCollisionMeshClick);

	// Physics
	Ref< ui::Container > containerPhysics = new ui::Container();
	containerPhysics->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, ui::scaleBySystemDPI(8)));

	Ref< ui::Static > staticPhysicsType = new ui::Static();
	staticPhysicsType->create(containerPhysics, i18n::Text(L"GAMEENTITY_WIZARD_PHYSICS_TYPE"));

	m_dropPhysicsType = new ui::DropDown();
	m_dropPhysicsType->create(containerPhysics);
	m_dropPhysicsType->add(i18n::Text(L"GAMEENTITY_WIZARD_PHYSICS_STATIC"));
	m_dropPhysicsType->add(i18n::Text(L"GAMEENTITY_WIZARD_PHYSICS_KINEMATIC"));
	m_dropPhysicsType->add(i18n::Text(L"GAMEENTITY_WIZARD_PHYSICS_DYNAMIC"));
	m_dropPhysicsType->select(0);

	Ref< ui::Static > staticCollisionGroup = new ui::Static();
	staticCollisionGroup->create(containerPhysics, i18n::Text(L"GAMEENTITY_WIZARD_COLLISION_GROUP"));

	m_editCollisionGroup = new ui::Edit();
	m_editCollisionGroup->create(containerPhysics, L"0x00000001");

	Ref< ui::Static > staticCollisionMask = new ui::Static();
	staticCollisionMask->create(containerPhysics, i18n::Text(L"GAMEENTITY_WIZARD_COLLISION_MASK"));

	m_editCollisionMask = new ui::Edit();
	m_editCollisionMask->create(containerPhysics, L"0x00000001");

	Ref< ui::Static > staticMaterial = new ui::Static();
	staticMaterial->create(containerPhysics, i18n::Text(L"GAMEENTITY_WIZARD_MATERIAL"));

	m_editMaterial = new ui::Edit();
	m_editMaterial->create(containerPhysics, L"0");

	Ref< ui::Static > staticFriction = new ui::Static();
	staticFriction->create(containerPhysics, i18n::Text(L"GAMEENTITY_WIZARD_FRICTION"));

	m_editFriction = new ui::Edit();
	m_editFriction->create(containerPhysics, L"0.75");

	// Script
	m_checkBoxCreateScript = new ui::CheckBox();
	m_checkBoxCreateScript->create(this, i18n::Text(L"GAMEENTITY_WIZARD_CREATE_SCRIPT"));

	addEventHandler< ui::ButtonClickEvent >(this, &GameEntityWizardDialog::eventDialogClick);
	return true;
}

void GameEntityWizardDialog::eventBrowseVisualMeshClick(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, i18n::Text(L"GAMEENTITY_WIZARD_FILE_TITLE"), L"All files;*.*"))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	std::wstring assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	FileSystem::getInstance().getRelativePath(
		FileSystem::getInstance().getAbsolutePath(fileName),
		FileSystem::getInstance().getAbsolutePath(assetPath),
		fileName
	);

	m_editVisualMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventBrowseCollisionMeshClick(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, i18n::Text(L"GAMEENTITY_WIZARD_FILE_TITLE"), L"All files;*.*"))
		return;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	std::wstring assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	FileSystem::getInstance().getRelativePath(
		FileSystem::getInstance().getAbsolutePath(fileName),
		FileSystem::getInstance().getAbsolutePath(assetPath),
		fileName
	);

	m_editCollisionMesh->setText(fileName.getPathName());
}

void GameEntityWizardDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if (event->getCommand() == ui::DrOk)
	{
		std::wstring name = m_editName->getText();
		std::wstring visualMesh = m_editVisualMesh->getText();
		std::wstring collisionMesh = m_editCollisionMesh->getText();
		int32_t physics = m_dropPhysicsType->getSelected();

		if (name.empty())
		{
			log::error << L"Game entity wizard failed; entity must have a name." << Endl;
			return;
		}

		Ref< world::ComponentEntityData > entityData = new world::ComponentEntityData();
		entityData->setName(name);

		if (!visualMesh.empty())
		{
			// Create visual mesh asset.
			Ref< mesh::MeshAsset > meshAsset = new mesh::MeshAsset();
			meshAsset->setFileName(visualMesh);
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

			entityData->setComponent(new mesh::MeshComponentData(
				resource::Id< mesh::IMesh >(meshAssetInstance->getGuid())
			));
		}

		if (!collisionMesh.empty())
		{
			// Create physics mesh asset.
			Ref< physics::MeshAsset > meshAsset = new physics::MeshAsset();
			meshAsset->setFileName(collisionMesh);
			meshAsset->setCalculateConvexHull(physics == 2);

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
			
			//m_collisionGroup.insert(resource::Id< CollisionSpecification >(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}")));
			//m_collisionMask.insert(resource::Id< CollisionSpecification >(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}")));
			//meshShapeDesc->setCollisionGroup(parseString< uint32_t >(m_editCollisionGroup->getText()));
			//meshShapeDesc->setCollisionMask(parseString< uint32_t >(m_editCollisionMask->getText()));

			meshShapeDesc->setMaterial(parseString< int32_t >(m_editMaterial->getText()));

			if (physics == 0 || physics == 1)
			{
				Ref< physics::StaticBodyDesc > bodyDesc = new physics::StaticBodyDesc();
				bodyDesc->setFriction(parseString< float >(m_editFriction->getText()));
				bodyDesc->setKinematic(physics == 1);
				bodyDesc->setShape(meshShapeDesc);

				Ref< physics::RigidBodyComponentData > componentData = new physics::RigidBodyComponentData();
				entityData->setComponent(new physics::RigidBodyComponentData(bodyDesc));
			}
			else
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
			ss << name << L" = " << name << L" or inherit(traktor.world.ComponentEntity)" << Endl;
			ss << name << L"_meta = " << name << L"_meta or { __index = " << name << L" }" << Endl;
			ss << Endl;
			ss << L"function " << name << L".new(o)" << Endl;
			ss << L"\to = o or {}" << Endl;
			ss << L"\tsetmetatable(o, " << name << L"_meta)" << Endl;
			ss << Endl;
			ss << L"\treturn o" << Endl;
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

		entityDataInstance->setObject(entityData);

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
