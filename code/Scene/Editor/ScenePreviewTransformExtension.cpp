/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/ScenePreviewTransformExtension.h"

#include "Core/Containers/SmallMap.h"
#include "Core/Log/Log.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "Editor/Pipeline/PipelineSettings.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/Events/PostBuildEvent.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Scene/Editor/Events/PostModifyEvent.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/SceneTransformChain.h"
#include "Scene/Editor/Traverser.h"
#include "World/Entity.h"
#include "World/EntityData.h"

namespace traktor::scene
{
namespace
{

const Scalar c_positionEpsilon(1e-4f);

/*! Editor-side transform context: reads source objects from the source database
 *  and provides a live (in-memory) ground sampler. */
class EditorTransformContext : public IScenePipelineOperator::TransformContext
{
public:
	explicit EditorTransformContext(db::Database* database)
	:	m_database(database)
	{
	}

	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) const override final
	{
		return m_database ? m_database->getObjectReadOnly(instanceGuid) : nullptr;
	}

	virtual db::Database* getSourceDatabase() const override final
	{
		return m_database;
	}

private:
	db::Database* m_database;
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ScenePreviewTransformExtension", ScenePreviewTransformExtension, ISceneEditorUIExtension)

ScenePreviewTransformExtension::ScenePreviewTransformExtension(SceneEditorContext* context)
:	m_context(context)
{
}

bool ScenePreviewTransformExtension::create(ui::Widget* parent, ui::ToolBar* toolBar)
{
	// Instantiate the geometric operator chain used for live preview.
	editor::PipelineSettings settings(m_context->getEditor()->getSettings());
	m_chain = new SceneTransformChain();
	if (!m_chain->create(&settings))
	{
		log::warning << L"Scene preview transform disabled; unable to create operator chain." << Endl;
		m_chain = nullptr;
	}

	m_context->addEventHandler< PostBuildEvent >(this, &ScenePreviewTransformExtension::eventPostBuild);
	m_context->addEventHandler< PostModifyEvent >(this, &ScenePreviewTransformExtension::eventPostModify);
	return true;
}

bool ScenePreviewTransformExtension::handleCommand(const ui::Command& command)
{
	return false;
}

void ScenePreviewTransformExtension::apply()
{
	if (!m_chain || m_chain->empty())
		return;

	SceneAsset* authored = m_context->getSceneAsset();
	if (!authored || authored->getOperationData().empty())
		return;

	EditorTransformContext context(m_context->getSourceDatabase());

	// Run geometric operators on a throwaway clone; always clone fresh from the
	// authored asset so operators start from canonical transforms (avoids e.g.
	// compounding an orientation alignment on repeated runs).
	Ref< SceneAsset > working = DeepClone(authored).create< SceneAsset >();
	if (!working)
		return;
	if (!m_chain->apply(working, context))
		return;

	// Collect the resulting transforms by entity id.
	SmallMap< Guid, Transform > transforms;
	for (auto layer : working->getLayers())
	{
		if (!layer)
			continue;
		Traverser::visit(layer, [&](const world::EntityData* entityData) -> Traverser::Result {
			const Guid& id = entityData->getId();
			if (id.isNotNull())
				transforms[id] = entityData->getTransform();
			return Traverser::Result::Continue;
		});
	}

	// Copy transforms onto the live rendered entities only; the authored scene
	// asset and its EntityData are left untouched.
	bool anyChanged = false;
	for (auto adapter : m_context->getEntities(SceneEditorContext::GfDescendants))
	{
		world::Entity* entity = adapter->getEntity();
		if (!entity)
			continue;

		const auto it = transforms.find(adapter->getId());
		if (it == transforms.end())
			continue;

		const Transform& target = it->second;
		if ((entity->getTransform().translation() - target.translation()).length() > c_positionEpsilon)
		{
			entity->setTransform(target);
			anyChanged = true;
		}
	}

	if (anyChanged)
		m_context->enqueueRedraw(nullptr);
}

void ScenePreviewTransformExtension::eventPostBuild(PostBuildEvent* event)
{
	apply();
}

void ScenePreviewTransformExtension::eventPostModify(PostModifyEvent* event)
{
	apply();
}

}
