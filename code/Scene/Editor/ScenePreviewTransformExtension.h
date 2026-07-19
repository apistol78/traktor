/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Scene/Editor/ISceneEditorUIExtension.h"

namespace traktor::scene
{

class PostBuildEvent;
class PostModifyEvent;
class SceneEditorContext;
class SceneTransformChain;

/*! */
class ScenePreviewTransformExtension : public ISceneEditorUIExtension
{
	T_RTTI_CLASS;

public:
	explicit ScenePreviewTransformExtension(SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::ToolBar* toolBar) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

private:
	SceneEditorContext* m_context;
	Ref< scene::SceneTransformChain > m_chain;

	void apply();

	void eventPostBuild(PostBuildEvent* event);

	void eventPostModify(PostModifyEvent* event);
};

}
