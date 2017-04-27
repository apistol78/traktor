/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_SkeletonEditorPage_H
#define traktor_animation_SkeletonEditorPage_H

#include "Core/Math/Color4ub.h"
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
class Container;
class PopupMenu;
class TreeView;
class TreeViewItem;

		namespace custom
		{

class ToolBar;

		}
	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderView;
class PrimitiveRenderer;

	}

	namespace animation
	{

class Skeleton;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS SkeletonEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SkeletonEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void activate() T_OVERRIDE T_FINAL;

	virtual void deactivate() T_OVERRIDE T_FINAL;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< Skeleton > m_skeleton;
	Ref< ui::Widget > m_renderWidget;
	Ref< ui::PopupMenu > m_boneMenu;
	Ref< ui::Container > m_skeletonPanel;
	Ref< ui::TreeView > m_treeSkeleton;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< resource::IResourceManager > m_resourceManager;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	ui::Point m_lastMousePosition;
	int m_selectedJoint;
	float m_cameraHead;
	float m_cameraY;
	float m_cameraZ;
	float m_cameraMoveScaleY;
	float m_cameraMoveScaleZ;
	float m_cameraBoneScale;

	void updateSettings();

	void createSkeletonTreeNodes();

	void createSkeletonTreeNodes(ui::TreeViewItem* parentItem, int parentNodeIndex);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventTreeButtonDown(ui::MouseButtonDownEvent* event);

	void eventTreeSelect(ui::SelectionChangeEvent* event);

	void eventTreeEdited(ui::TreeViewContentChangeEvent* event);
};

	}
}

#endif	// traktor_animation_SkeletonEditorPage_H
