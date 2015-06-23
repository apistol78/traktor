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

class ListBox;

		namespace custom
		{

class ToolBarButton;
class ToolBarButtonClickEvent;

		}
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

	bool create(ui::Widget* parent);

	void destroy();

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	resource::Proxy< render::ITexture > m_textureDebug;
	Ref< ui::custom::ToolBarButton > m_toolSolid;
	Ref< ui::custom::ToolBarButton > m_toolWire;
	Ref< ui::custom::ToolBarButton > m_toolNormals;
	Ref< ui::custom::ToolBarButton > m_toolVertices;
	Ref< ui::custom::ToolBarButton > m_toolCull;
	Ref< ui::custom::ToolBarButton > m_toolNonSharedEdges;
	Ref< ui::custom::ToolBarButton > m_toolUV;
	Ref< ui::ListBox > m_modelList;
	Ref< ui::Widget > m_renderWidget;
	Ref< Model > m_model;
	Ref< Model > m_modelTris;
	Ref< ModelAdjacency > m_modelAdjacency;
	std::wstring m_modelName;
	float m_cameraHead;
	float m_cameraPitch;
	float m_cameraZ;
	float m_normalScale;
	ui::Point m_lastMousePosition;

	bool loadModel();

	bool saveModel();

	bool removeModel();

	bool applyOperation(const IModelOperation* operation);

	void eventDialogClose(ui::CloseEvent* event);

	void eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventModelListSelect(ui::SelectionChangeEvent* event);

	void eventMouseDown(ui::MouseButtonDownEvent* event);

	void eventMouseUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventRenderSize(ui::SizeEvent* event);

	void eventRenderPaint(ui::PaintEvent* event);
};

	}
}

#endif	// traktor_model_ModelToolDialog_H
