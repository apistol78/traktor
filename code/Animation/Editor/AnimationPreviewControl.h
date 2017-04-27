/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_AnimationPreviewControl_H
#define traktor_animation_AnimationPreviewControl_H

#include "Core/Timer/Timer.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Ui/Widget.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace mesh
	{

class SkinnedMesh;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class IWorldRenderer;

	}

	namespace animation
	{

class AnimatedMeshEntity;
class IPoseController;
class Skeleton;

/*! \brief
 * \ingroup Animation
 */
class AnimationPreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	AnimationPreviewControl(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

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
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	resource::Proxy< mesh::SkinnedMesh > m_mesh;
	resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	Ref< AnimatedMeshEntity > m_entity;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	Timer m_timer;
	Vector4 m_position;
	float m_angleHead;
	float m_anglePitch;
	ui::Point m_lastMousePosition;

	void updatePreview();

	void updateWorldRenderer();

	void updateWorldRenderView();

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

	}
}

#endif	// traktor_animation_AnimationPreviewControl_H
