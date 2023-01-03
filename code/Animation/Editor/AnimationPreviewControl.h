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
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Ui/Widget.h"
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

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;
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

namespace traktor::animation
{

class AnimatedMeshComponent;
class IPoseController;
class Skeleton;

/*! Animation 3d preview control.
 * \ingroup Animation
 */
class AnimationPreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	explicit AnimationPreviewControl(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	void setMesh(const resource::Id< mesh::SkinnedMesh >& mesh);

	void setSkeleton(const resource::Id< Skeleton >& skeleton);

	void setPoseController(IPoseController* poseController);

	void updateSettings();

	resource::IResourceManager* getResourceManager() const { return m_resourceManager; }

private:
	editor::IEditor* m_editor;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderGraph > m_renderGraph;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	resource::Proxy< scene::Scene > m_sceneInstance;
	resource::Proxy< mesh::SkinnedMesh > m_mesh;
	resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	Ref< world::Entity > m_entity;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	Timer m_timer;
	Vector4 m_position;
	float m_angleHead;
	float m_anglePitch;
	ui::Point m_lastMousePosition;
	ui::Size m_dirtySize = ui::Size(0, 0);

	void updatePreview();

	void updateWorldRenderer();

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

}
