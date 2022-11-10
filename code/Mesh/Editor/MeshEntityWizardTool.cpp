/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshEntityWizardTool.h"
#include "Ui/MessageBox.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityWizardTool", 0, MeshEntityWizardTool, editor::IWizardTool)

std::wstring MeshEntityWizardTool::getDescription() const
{
	return i18n::Text(L"MESHENTITY_WIZARDTOOL_DESCRIPTION");
}

uint32_t MeshEntityWizardTool::getFlags() const
{
	return WfInstance;
}

bool MeshEntityWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	if (!is_type_a< MeshAsset >(*instance->getPrimaryType()))
	{
		ui::MessageBox::show(parent, i18n::Text(L"MESHENTITY_WIZARDTOOL_ERROR_NOT_MESH_ASSET_MESSAGE"), i18n::Text(L"MESHENTITY_WIZARDTOOL_ERROR_NOT_MESH_ASSET_CAPTION"), ui::MbOk | ui::MbIconExclamation);
		return false;
	}

	// Create mesh entity data.
	Ref< world::EntityData > entityData = new world::EntityData();
	entityData->setId(Guid::create());
	entityData->setName(instance->getName());
	entityData->setComponent(new MeshComponentData(resource::Id< IMesh >(instance->getGuid())));

	// Create mesh entity data instance.
	Ref< db::Instance > entityDataInstance = group->createInstance(
		instance->getName() + L"-Entity",
		db::CifReplaceExisting | db::CifKeepExistingGuid
	);
	if (!entityDataInstance)
	{
		log::error << L"Failed to create instance." << Endl;
		return false;
	}

	entityDataInstance->setObject(entityData);

	if (!entityDataInstance->commit())
	{
		log::error << L"Failed to commit instance." << Endl;
		return false;
	}

	return true;
}

	}
}
