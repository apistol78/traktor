/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_ModelToolDialog_H
#define traktor_model_ModelToolDialog_H

#include "Resource/Proxy.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;
class ITexture;
class PrimitiveRenderer;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace ui
	{

class GridView;
class Menu;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class TreeView;
class TreeViewItem;

	}

	namespace model
	{

class IModelOperation;
class Model;
class ModelAdjacency;

class ModelToolDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	ModelToolDialog(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	bool create(ui::Widget* parent, const std::wstring& fileName);

	void destroy();

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	resource::Proxy< render::ITexture > m_textureDebug;
	Ref< ui::ToolBarButton > m_toolSolid;
	Ref< ui::ToolBarButton > m_toolWire;
	Ref< ui::ToolBarButton > m_toolNormals;
	Ref< ui::ToolBarButton > m_toolVertices;
	Ref< ui::ToolBarButton > m_toolCull;
	Ref< ui::ToolBarButton > m_toolNonSharedEdges;
	Ref< ui::ToolBarButton > m_toolUV;
	Ref< ui::ToolBarDropDown > m_toolChannel;
	Ref< ui::ToolBarButton > m_toolWeight;
	Ref< ui::ToolBarDropDown > m_toolJoint;
	Ref< ui::TreeView > m_modelTree;
	Ref< ui::GridView > m_materialGrid;
	Ref< ui::Menu > m_modelRootPopup;
	Ref< ui::Menu > m_modelChildPopup;
	Ref< ui::Widget > m_renderWidget;
	Ref< Model > m_model;
	Ref< Model > m_modelTris;
	Ref< ModelAdjacency > m_modelAdjacency;
	float m_cameraHead;
	float m_cameraPitch;
	float m_cameraZ;
	float m_normalScale;
	ui::Point m_lastMousePosition;

	bool loadModel();

	bool saveModel(Model* model);

	void bakeOcclusion(Model* model);

	void updateOperations(ui::TreeViewItem* itemModel);

	void eventDialogClose(ui::CloseEvent* event);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventModelTreeButtonDown(ui::MouseButtonDownEvent* event);

	void eventModelTreeSelect(ui::SelectionChangeEvent* event);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventRenderSize(ui::SizeEvent* event);

	void eventRenderPaint(ui::PaintEvent* event);
};

	}
}

#endif	// traktor_model_ModelToolDialog_H
