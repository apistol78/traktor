/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Vector4.h"
#include "Core/Ref.h"
#include "Scene/Editor/IComponentPanelEditor.h"

namespace traktor::ai
{

class NavMeshChunk;
class NavMeshComponent;

/*! Navigation mesh component editor.
 * \ingroup AI
 */
class NavMeshComponentEditor : public scene::IComponentPanelEditor
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getTitle() const override final;

	virtual bool create(scene::SceneEditorContext* context, ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual void entityRemoved(scene::EntityAdapter* entityAdapter) override final;

	virtual void propertiesChanged() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void update() override final;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) override final;

private:
	Ref< scene::SceneEditorContext > m_context;
	Ref< NavMeshComponent > m_navMeshComponent;	//!< Navigation mesh component chunks have been added to.
	SmallMap< int32_t, Ref< const NavMeshChunk > > m_chunks;	//!< Added chunks by index.

	void updateChunks(NavMeshComponent* navMeshComponent, const Vector4& eye);

	void removeChunks();
};

}
