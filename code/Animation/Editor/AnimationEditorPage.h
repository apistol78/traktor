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

class Widget;
class TreeView;
class TreeViewItem;
class PopupMenu;

		namespace custom
		{

class CursorMoveEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class SequencerControl;

		}
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

class T_DLLCLASS AnimationEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	AnimationEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent);

	virtual void destroy();

	virtual void activate();

	virtual void deactivate();

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position);

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< Animation > m_animation;
	Ref< Skeleton > m_skeleton;
	Ref< ui::Widget > m_renderWidgets[4];
	Ref< ui::custom::ToolBar > m_toolBarPlay;
	Ref< ui::Container > m_sequencerPanel;
	Ref< ui::custom::SequencerControl > m_sequencer;
	Ref< ui::PopupMenu > m_menuPopup;
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

	void eventSequencerCursorMove(ui::custom::CursorMoveEvent* event);

	void eventSequencerTimer(ui::TimerEvent* event);

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);
};

	}
}

#endif	// traktor_animation_AnimationEditorPage_H
