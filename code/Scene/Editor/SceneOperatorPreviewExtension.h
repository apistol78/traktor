/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Transform.h"
#include "Scene/Editor/ISceneEditorUIExtension.h"

namespace traktor::scene
{

class PostBuildEvent;
class PostFrameEvent;
class PostModifyEvent;
class SceneEditorContext;
class SceneOperatorChain;

/*! */
class SceneOperatorPreviewExtension : public ISceneEditorUIExtension
{
	T_RTTI_CLASS;

public:
	explicit SceneOperatorPreviewExtension(SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::ToolBar* toolBar) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	/*! Transforms the operators derived for one state of the scene. */
	struct CachedTransforms
	{
		uint32_t hash;
		SmallMap< Guid, Transform > transforms;
	};

	SceneEditorContext* m_context;
	Ref< scene::SceneOperatorChain > m_chain;
	SmallMap< Guid, Transform > m_transforms;
	AlignedVector< CachedTransforms > m_cache;
	bool m_pending = false;

	void apply();

	/*! Take the transforms derived for a state of the scene into use; false if there are none. */
	bool useCachedTransforms(uint32_t hash);

	/*! Keep the transforms derived for a state of the scene. */
	void cacheTransforms(uint32_t hash);

	void updateEntities();

	void eventPostBuild(PostBuildEvent* event);

	void eventPostFrame(PostFrameEvent* event);

	void eventPostModify(PostModifyEvent* event);
};

}
