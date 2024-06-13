/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Scene/Editor/IModifier.h"

namespace traktor
{
    namespace scene
    {

class EntityAdapter;
class SceneEditorContext;

    }

	namespace shape
	{

/*! Primitive editor modifier.
 * \ingroup Shape
 */
class PrimitiveEditModifier : public scene::IModifier
{
	T_RTTI_CLASS;

public:
	explicit PrimitiveEditModifier(scene::SceneEditorContext* context);

	virtual bool activate() override final;

	virtual void deactivate() override final;

	virtual void selectionChanged() override final;

	virtual void buttonDown() override final;

	virtual CursorMovedResult cursorMoved(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual bool begin(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		int32_t mouseButton
	) override final;

	virtual void apply(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		bool snapOverrideEnable
	) override final;

	virtual void end(const scene::TransformChain& transformChain) override final;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const override final;

private:
	scene::SceneEditorContext* m_context;
	RefArray< scene::EntityAdapter > m_entityAdapters;
};

	}
}
