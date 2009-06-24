#include "Render/Editor/Shader/SwitchNodeFacade.h"
#include "Render/Editor/Shader/SwitchNodeDialog.h"
#include "Render/Nodes.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SwitchNodeFacade", SwitchNodeFacade, NodeFacade)

SwitchNodeFacade::SwitchNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = gc_new< ui::custom::DefaultNodeShape >(graphControl);
}

Node* SwitchNodeFacade::createShaderNode(
	const Type* nodeType,
	editor::IEditor* editor
)
{
	return gc_new< Switch >();
}

ui::custom::Node* SwitchNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	return gc_new< ui::custom::Node >(
		i18n::Text(L"SHADERGRAPH_NODE_SWITCH"),
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);
}

void SwitchNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	SwitchNodeDialog nodeDialog;
	if (!nodeDialog.create(graphControl, checked_type_cast< Switch* >(shaderNode)))
		return;
	if (nodeDialog.showModal() == ui::DrOk)
	{
	}
	nodeDialog.destroy();
}

void SwitchNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? traktor::Color(255, 255, 255) : traktor::Color(255, 120, 120));
}

	}
}
