/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Timer/Timer.h"
#include "Render/Editor/RenderControl.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "World/WorldRenderView.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::mesh
{

class SkinnedMesh;

}

namespace traktor::render
{

class RenderContext;
class RenderGraph;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::scene
{

class Scene;

}

namespace traktor::world
{

class Entity;
class IWorldRenderer;

}

namespace traktor::physics
{

class Body;
class PhysicsManager;

}

namespace traktor::animation
{

class AnimatedMeshComponent;
class IPoseController;
class Skeleton;

/*! Animation 3d preview control.
 * \ingroup Animation
 */
class AnimationPreviewControl : public render::RenderControl
{
	T_RTTI_CLASS;

public:
	explicit AnimationPreviewControl(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	void setMesh(const resource::Id< mesh::SkinnedMesh >& mesh);

	void setSkeleton(const resource::Id< Skeleton >& skeleton);

	void setPoseController(IPoseController* poseController);

	void setParameterValue(const std::wstring& parameterName, bool value);

	void updateSettings();

	physics::PhysicsManager* getPhysicsManager() const { return m_physicsManager; }

	const Skeleton* getSkeleton() const;

private:
	editor::IEditor* m_editor = nullptr;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< physics::PhysicsManager > m_physicsManager;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderGraph > m_renderGraph;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	resource::Proxy< scene::Scene > m_sceneInstance;
	resource::Proxy< mesh::SkinnedMesh > m_mesh;
	resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	Ref< world::Entity > m_entity;
	Color4ub m_colorClear;
	Timer m_timer;

	void updatePreview();

	void updateWorldRenderer();

	virtual bool renderFrame() override final;

	void eventIdle(ui::IdleEvent* event);
};

}
