#ifndef traktor_animation_SkeletonEditorPage_H
#define traktor_animation_SkeletonEditorPage_H

#include "Core/Math/Color4ub.h"
#include "Editor/IEditorPage.h"
#include "Ui/Point.h"

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
class Event;

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

class T_DLLCLASS SkeletonEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SkeletonEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

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

	void eventMouseDown(ui::Event* event);

	void eventMouseUp(ui::Event* event);

	void eventMouseMove(ui::Event* event);

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);

	void eventTreeButtonDown(ui::Event* event);

	void eventTreeSelect(ui::Event* event);

	void eventTreeEdited(ui::Event* event);
};

	}
}

#endif	// traktor_animation_SkeletonEditorPage_H
