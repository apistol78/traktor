/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Timer/Timer.h"
#include "Resource/Proxy.h"
#include "Ui/Dialog.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderView;
class ITexture;
class PrimitiveRenderer;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::ui
{

class GridView;
class Menu;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class TreeView;
class TreeViewItem;

}

namespace traktor::model
{

class IModelOperation;
class Model;
class ModelAdjacency;

class ModelToolDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	ModelToolDialog(
		editor::IEditor* editor,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem
	);

	bool create(ui::Widget* parent, const std::wstring& fileName, float scale);

	virtual void destroy() override final;

private:
	editor::IEditor* m_editor;
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	resource::Proxy< render::ITexture > m_textureDebug;
	Ref< render::ITexture > m_texturePreview;
	Ref< ui::ToolBarButton > m_toolShading;
	Ref< ui::ToolBarButton > m_toolSolid;
	Ref< ui::ToolBarButton > m_toolWire;
	Ref< ui::ToolBarButton > m_toolNormals;
	Ref< ui::ToolBarButton > m_toolVertices;
	Ref< ui::ToolBarButton > m_toolCull;
	Ref< ui::ToolBarButton > m_toolNonSharedEdges;
	Ref< ui::ToolBarButton > m_toolUV;
	Ref< ui::ToolBarDropDown > m_toolChannel;
	Ref< ui::ToolBarButton > m_toolWeight;
	Ref< ui::ToolBarButton > m_toolPose;
	Ref< ui::ToolBarButton > m_toolRest;
	Ref< ui::TreeView > m_modelTree;
	Ref< ui::GridView > m_materialGrid;
	Ref< ui::TreeView > m_skeletonTree;
	Ref< ui::GridView > m_statisticGrid;
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
	Timer m_timer;

	bool loadModel();

	bool loadTexture();

	bool reloadModel(ui::TreeViewItem* itemModel);

	bool saveModel(Model* model);

	void updateModel();

	void updateOperations(ui::TreeViewItem* itemModel);

	void addStatistic(const std::wstring& name, const std::wstring& value);

	void eventDialogClose(ui::CloseEvent* event);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventModelTreeButtonDown(ui::MouseButtonDownEvent* event);

	void eventModelTreeSelect(ui::SelectionChangeEvent* event);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventRenderSize(ui::SizeEvent* event);

	void eventRenderPaint(ui::PaintEvent* event);

	void eventMaterialSelect(ui::SelectionChangeEvent* event);

	void eventSkeletonSelect(ui::SelectionChangeEvent* event);
};

}
