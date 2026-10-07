/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"
#include "Scene/Editor/IComponentEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;

}

namespace traktor::scene
{

class MarkerEntityComponentData;
class SceneEditorContext;

/*! Draws marker icon as a camera facing quad.
 * \ingroup Scene
 */
class T_DLLCLASS MarkerEntityComponentEditor : public IComponentEditor
{
	T_RTTI_CLASS;

public:
	explicit MarkerEntityComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, MarkerEntityComponentData* markerComponentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;

private:
	EntityAdapter* m_entityAdapter;
	resource::Proxy< render::ITexture > m_texture;
	float m_offset;
	float m_size;
};

}
