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
#include "Physics/Editor/PhysicsRenderer.h"
#include "Render/Editor/RenderControl.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderControlEvent;

}

namespace traktor::animation
{

class RagDollBone;
class RagDollSkeletonAsset;
class Skeleton;

/*! Interactive 3D preview of a rag doll skeleton setup.
 * \ingroup Animation
 *
 * Inherits the shared camera navigation and render loop from
 * render::RenderControl and draws the rag doll setup on top.
 */
class T_DLLCLASS RagDollPreviewControl : public render::RenderControl
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, render::IRenderSystem* renderSystem, db::Database* database);

	/*! Set rag doll asset to preview; the setup is re-evaluated each frame. */
	void setAsset(const RagDollSkeletonAsset* asset);

	/*! Set optional animation skeleton to overlay. */
	void setSkeleton(const Skeleton* skeleton);

	/*! Set bone to highlight, matching the current tree selection (null to clear). */
	void setSelectedBone(const RagDollBone* bone);

private:
	physics::PhysicsRenderer m_physicsRenderer;
	Ref< const RagDollSkeletonAsset > m_asset;
	Ref< const Skeleton > m_skeleton;
	Ref< const RagDollBone > m_selectedBone;

	void eventRender(render::RenderControlEvent* event);
};

}
