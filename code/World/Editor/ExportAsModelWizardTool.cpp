/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/ExportAsModelWizardTool.h"

#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/Pipeline/PipelineSettings.h"
#include "I18N/Text.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Ui/FileDialog.h"
#include "World/Editor/IEntityReplicator.h"
#include "World/Editor/Traverser.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"

#include <cstring>

namespace traktor::world
{
namespace
{

class FauxPipelineCommon : public editor::IPipelineCommon
{
public:
	explicit FauxPipelineCommon(db::Database* database)
		: m_database(database)
	{
	}

	virtual db::Database* getSourceDatabase() const override final
	{
		return m_database;
	}

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) override final
	{
		return m_database->getObjectReadOnly(instanceGuid);
	}

private:
	db::Database* m_database;
};

struct SourceModel
{
	Ref< const model::Model > model;
	Transform transform;

	SourceModel() = default;

	SourceModel(const model::Model* model_, const Transform& transform_)
		: model(model_)
		, transform(transform_)
	{
	}
};

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.ExportAsModelWizardTool", 0, ExportAsModelWizardTool, editor::IWizardTool)

bool ExportAsModelWizardTool::create(const PropertyGroup* settings)
{
	editor::PipelineSettings pipelineSettings(settings);
	for (const auto& entityReplicatorType : type_of< world::IEntityReplicator >().findAllOf(false))
	{
		Ref< world::IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< world::IEntityReplicator* >(entityReplicatorType->createInstance());
		if (!entityReplicator->create(&pipelineSettings))
			return false;

		auto supportedTypes = entityReplicator->getSupportedTypes();
		for (auto supportedType : supportedTypes)
			m_entityReplicators[supportedType] = entityReplicator;
	}
	return true;
}

std::wstring ExportAsModelWizardTool::getDescription() const
{
	return i18n::Text(L"EXPORT_AS_MODEL_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet ExportAsModelWizardTool::getSupportedTypes() const
{
	return makeTypeInfoSet< EntityData >();
}

uint32_t ExportAsModelWizardTool::getFlags() const
{
	return WfInstance;
}

bool ExportAsModelWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< EntityData > sourceData = instance->getObject< EntityData >();
	if (!sourceData)
		return false;

	Path fileName;

	ui::FileDialog saveAsDialog;
	saveAsDialog.create(parent, L"", L"Save model as...", L"glTF model files (*.gltf);*.gltf;All files (*.*);*.*", L"", true);
	if (saveAsDialog.showModalThenDestroy(fileName) != ui::DialogResult::Ok)
		return false;

	FauxPipelineCommon pipelineCommon(editor->getSourceDatabase());

	AlignedVector< SourceModel > sourceModels;

	// First include the selected entity data.
	{
		Ref< model::Model > model;
		for (auto componentData : sourceData->getComponents())
		{
			const world::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
			if (entityReplicator)
			{
				if ((model = entityReplicator->createModel(&pipelineCommon, sourceData, componentData, IEntityReplicator::Usage::Visual)) != nullptr)
					break;
			}
		}

		if (model)
			sourceModels.push_back(SourceModel(
				model,
				Transform::identity()));
	}

	// Traverse all child entity data.
	Traverser::visit(sourceData, [&](const EntityData* entityData) -> Traverser::Result {
		Ref< model::Model > model;
		for (auto componentData : entityData->getComponents())
		{
			const world::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
			if (entityReplicator)
			{
				if ((model = entityReplicator->createModel(&pipelineCommon, entityData, componentData, IEntityReplicator::Usage::Visual)) != nullptr)
					break;
			}
		}

		if (model)
			sourceModels.push_back(SourceModel(
				model,
				entityData->getTransform()));

		return Traverser::Result::Continue;
	});

	Ref< model::Model > mergedModel = new model::Model();
	for (auto sourceModel : sourceModels)
	{
		const model::MergeModel mrg(*sourceModel.model, sourceModel.transform, 0.0001f);
		mergedModel->apply(mrg);
	}

	if (!model::ModelFormat::writeAny(fileName, mergedModel))
	{
		log::error << L"Failed to save model \"" << fileName.getOriginal() << L"\"." << Endl;
		return false;
	}

	log::info << L"Model \"" << fileName.getOriginal() << L"\" exported successfully!" << Endl;
	return true;
}

}
