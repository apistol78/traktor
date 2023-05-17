/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/IComponentEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityComponentData;

}

namespace traktor::scene
{

class EntityAdapter;
class SceneEditorContext;

/*!
 * \ingroup Scene
 */
class T_DLLCLASS DefaultComponentEditor : public IComponentEditor
{
	T_RTTI_CLASS;

public:
	explicit DefaultComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void transformModified(EntityAdapter* entityAdapter, EntityAdapter* modifiedEntityAdapter) override;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override;

protected:
	SceneEditorContext* m_context;
	EntityAdapter* m_entityAdapter;
	world::IEntityComponentData* m_componentData;
};

}
