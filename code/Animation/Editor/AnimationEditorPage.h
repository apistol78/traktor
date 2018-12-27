/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_AnimationEditorPage_H
#define traktor_animation_AnimationEditorPage_H

#include "Core/Math/Color4ub.h"
#include "Core/Math/Vector4.h"
#include "Editor/IEditorPage.h"
#include "Ui/Point.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;

	}

	namespace db
	{

class Instance;

	}

	namespace ui
	{

class CursorMoveEvent;
class Menu;
class SequencerControl;
class ToolBar;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewContentChangeEvent;
class TreeViewItem;
class Widget;

	}

	namespace resource
	{

class ResourceManager;

	}

	namespace render
	{

class PrimitiveRenderer;

	}

	namespace animation
	{

class Animation;
class Skeleton;
class Pose;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS AnimationEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	AnimationEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< Animation > m_animation;
	Ref< Skeleton > m_skeleton;
	Ref< ui::Widget > m_renderWidgets[4];
	Ref< ui::Container > m_skeletonPanel;
	Ref< ui::TreeView > m_treeSkeleton;
	Ref< ui::ToolBar > m_toolBarPlay;
	Ref< ui::Container > m_sequencerPanel;
	Ref< ui::SequencerControl > m_sequencer;
	Ref< ui::Menu > m_menuPopup;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< resource::ResourceManager > m_resourceManager;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	Color4ub m_colorBone;
	Color4ub m_colorBoneSel;
	ui::Point m_lastMousePosition;
	int m_selectedBone;
	bool m_showGhostTrail;
	bool m_twistLock;
	bool m_ikEnabled;
	float m_relativeTwist;
	bool m_haveRelativeTwist;
	bool m_editMode;
	bool m_previewAnimation;
	Vector4 m_cameraOffsetScale;
	float m_cameraSizeScale;

	void setSkeleton(Skeleton* skeleton);

	bool getSelectedPoseId(int& outPoseId) const;

	void updateRenderWidgets();

	void updateSequencer();

	bool calculateRelativeTwist(int poseIndex, int boneIndex, float& outRelativeTwist) const;

	void drawSkeleton(float time, const Color4ub& defaultColor, const Color4ub& selectedColor, bool drawAxis) const;

	void updateSettings();

	void eventRenderButtonDown(ui::MouseButtonDownEvent* event);

	void eventRenderButtonUp(ui::MouseButtonUpEvent* event);

	void eventRenderMouseMove(ui::MouseMoveEvent* event);

	void eventRenderMouseWheel(ui::MouseWheelEvent* event);

	void eventRenderSize(ui::SizeEvent* event);

	void eventRenderPaint(ui::PaintEvent* event);

	void eventSequencerButtonDown(ui::MouseButtonDownEvent* event);

	void eventSequencerCursorMove(ui::CursorMoveEvent* event);

	void eventSequencerTimer(ui::TimerEvent* event);

	void eventToolClick(ui::ToolBarButtonClickEvent* event);
};

	}
}

#endif	// traktor_animation_AnimationEditorPage_H
