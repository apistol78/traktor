/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/SceneOperatorPreviewExtension.h"

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
#include "Scene/Editor/SceneOperatorChain.h"
#include "World/Editor/Traverser.h"
#include "World/Entity.h"
#include "World/EntityData.h"

namespace traktor::scene
{
namespace
{

const Scalar c_positionEpsilon(1e-4f);

/*! Editor-side transform context: reads source objects from the source database
 *  and provides a live (in-memory) ground sampler. */
class EditorTransformContext : public ISceneOperator::TransformContext
{
public:
	explicit EditorTransformContext(db::Database* database)
		: m_database(database)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneOperatorPreviewExtension", SceneOperatorPreviewExtension, ISceneEditorUIExtension)

SceneOperatorPreviewExtension::SceneOperatorPreviewExtension(SceneEditorContext* context)
	: m_context(context)
{
}

bool SceneOperatorPreviewExtension::create(ui::Widget* parent, ui::ToolBar* toolBar)
{
	// Instantiate the geometric operator chain used for live preview.
	editor::PipelineSettings settings(m_context->getEditor()->getSettings());
	m_chain = new SceneOperatorChain();
	if (!m_chain->create(&settings))
	{
		log::warning << L"Scene preview transform disabled; unable to create operator chain." << Endl;
		m_chain = nullptr;
	}

	m_context->addEventHandler< PostBuildEvent >(this, &SceneOperatorPreviewExtension::eventPostBuild);
	m_context->addEventHandler< PostFrameEvent >(this, &SceneOperatorPreviewExtension::eventPostFrame);
	m_context->addEventHandler< PostModifyEvent >(this, &SceneOperatorPreviewExtension::eventPostModify);
	return true;
}

bool SceneOperatorPreviewExtension::handleCommand(const ui::Command& command)
{
	return false;
}

void SceneOperatorPreviewExtension::apply()
{
	if (!m_chain || m_chain->empty())
		return;

	SceneAsset* authored = m_context->getSceneAsset();
	if (!authored || authored->getOperationData().empty())
		return;

	// Running the chain is expensive; while the user is modifying entities it
	// would be re-evaluated for each intermediate state, such as when entities
	// are cloned at the start of a duplicating drag. Defer until the
	// modification has finished so movement remains responsive.
	if (m_context->isModifyInProgress())
	{
		m_pending = true;
		return;
	}

	m_pending = false;

	EditorTransformContext context(m_context->getSourceDatabase());

	// Run geometric operators on a throwaway clone; always clone fresh from the
	// authored asset so operators start from canonical transforms (avoids e.g.
	// compounding an orientation alignment on repeated runs).
	Ref< SceneAsset > working = DeepClone(authored).create< SceneAsset >();
	if (!working)
		return;
	if (!m_chain->apply(working, context))
		return;

	// Collect the resulting transforms by entity id; kept so they can be
	// restored without running the chain again, such as when entities have been
	// recreated in the middle of a modification.
	m_transforms.clear();
	for (auto layer : working->getLayers())
		world::Traverser::visit(layer, [&](const world::EntityData* entityData) -> world::Traverser::Result {
			const Guid& id = entityData->getId();
			if (id.isNotNull())
				m_transforms[id] = entityData->getTransform();
			return world::Traverser::Result::Continue;
		});

	updateEntities();
}

void SceneOperatorPreviewExtension::updateEntities()
{
	// Copy transforms onto the live rendered entities only; the authored scene
	// asset and its EntityData are left untouched.
	bool anyChanged = false;
	for (auto adapter : m_context->getEntities(SceneEditorContext::GfDescendants))
	{
		world::Entity* entity = adapter->getEntity();
		if (!entity)
			continue;

		const auto it = m_transforms.find(adapter->getId());
		if (it == m_transforms.end())
			continue;

		const Transform& target = it->second;
		const Transform current = entity->getTransform();
		if (
			(current.translation() - target.translation()).length() > c_positionEpsilon ||
			current.rotation() != target.rotation()
		)
		{
			entity->setTransform(target);
			anyChanged = true;
		}
	}

	if (anyChanged)
		m_context->enqueueRedraw(nullptr);
}

void SceneOperatorPreviewExtension::eventPostBuild(PostBuildEvent* event)
{
	apply();

	// Entities have been recreated from the authored scene asset and thus lost
	// the transforms calculated by the operators; if the chain was deferred then
	// restore the last calculated transforms, otherwise the scene would snap
	// back to its authored placement until the modification has finished.
	if (m_pending)
		updateEntities();
}

void SceneOperatorPreviewExtension::eventPostFrame(PostFrameEvent* event)
{
	// Catch up with an apply which was deferred during a modification, in case
	// the modification ended without a post modify event being raised.
	if (m_pending)
		apply();
}

void SceneOperatorPreviewExtension::eventPostModify(PostModifyEvent* event)
{
	apply();
}

}
