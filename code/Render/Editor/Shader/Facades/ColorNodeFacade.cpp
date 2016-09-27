#include "Render/Shader/Nodes.h"
#include "Render/Editor/Shader/Facades/ColorNodeFacade.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/InputNodeShape.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ColorNodeFacade", ColorNodeFacade, INodeFacade)

ColorNodeFacade::ColorNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = new ui::custom::InputNodeShape(graphControl);
}

Ref< Node > ColorNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Color();
}

Ref< ui::custom::Node > ColorNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		i18n::Text(L"SHADERGRAPH_NODE_COLOR"),
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);

	for (int j = 0; j < shaderNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = shaderNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < shaderNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = shaderNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(shaderNode->getComment());

	return editorNode;
}

void ColorNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	Node* shaderNode
)
{
	Ref< Color > colorNode = checked_type_cast< Color* >(shaderNode);

	ui::custom::ColorDialog colorDialog;
	colorDialog.create(
		graphControl,
		i18n::Text(L"COLOR_DIALOG_TEXT"),
		ui::custom::ColorDialog::WsDefaultFixed | ui::custom::ColorDialog::WsAlpha,
		colorNode->getColor()
	);
	if (colorDialog.showModal() == ui::DrOk)
		colorNode->setColor(colorDialog.getColor());
	colorDialog.destroy();
}

void ColorNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	Node* shaderNode
)
{
	editorNode->setComment(shaderNode->getComment());
	editorNode->setInfo(shaderNode->getInformation());
}

void ColorNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
