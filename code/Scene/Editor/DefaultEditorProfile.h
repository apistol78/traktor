/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/ISceneEditorProfile.h"

namespace traktor::scene
{

/*! Default scene editor profile. */
class DefaultEditorProfile : public ISceneEditorProfile
{
	T_RTTI_CLASS;

public:
	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const override final;

	virtual void getGuideDrawIds(
		std::set< std::wstring >& outIds
	) const override final;

	virtual void createEditorPlugins(
		SceneEditorContext* context,
		RefArray< ISceneEditorPlugin >& outEditorPlugins
	) const override final;

	virtual void createResourceFactories(
		SceneEditorContext* context,
		RefArray< const resource::IResourceFactory >& outResourceFactories
	) const override final;

	virtual void createEntityFactories(
		SceneEditorContext* context,
		RefArray< const world::IEntityFactory >& outEntityFactories
	) const override final;

	virtual void createEntityRenderers(
		SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		const TypeInfo& worldRendererType,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const override final;

	virtual void createControllerEditorFactories(
		SceneEditorContext* context,
		RefArray< const IWorldComponentEditorFactory >& outControllerEditorFactories
	) const override final;

	virtual void createEntityEditorFactories(
		SceneEditorContext* context,
		RefArray< const IEntityEditorFactory >& outEntityEditorFactories
	) const override final;

	virtual void createComponentEditorFactories(
		SceneEditorContext* context,
		RefArray< const IComponentEditorFactory >& outComponentEditorFactories
	) const override final;

	virtual Ref< world::EntityData > createEntityData(
		SceneEditorContext* context,
		db::Instance* instance
	) const override final;
};

}
