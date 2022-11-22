/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

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
class IComponentEditor;
class SceneEditorContext;

/*!
 * \ingroup Scene
 */
class T_DLLCLASS IComponentEditorFactory : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get supported set of components.
	 *
	 * \return Type Set of IEntityComponentData types.
	 */
	virtual const TypeInfoSet getComponentDataTypes() const = 0;

	/*! Create component editor.
	 *
	 * \param context Scene editor context.
	 * \param entityAdapter Owner entity adapter.
	 * \param componentData Component data.
	 * \return Component editor instance.
	 */
	virtual Ref< IComponentEditor > createComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const = 0;
};

}
