#include "Core/Misc/SafeDestroy.h"
#include "I18N/Text.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeFacade", ScriptNodeFacade, INodeFacade)

ScriptNodeFacade::ScriptNodeFacade(ShaderGraphEditorPage* page)
:	m_page(page)
{
	m_nodeShape = new ui::DefaultNodeShape(ui::DefaultNodeShape::StScript);
}

Ref< Node > ScriptNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	Ref< Script > sc = new Script();
	sc->setName(L"Unnamed");
	sc->addInputPin(Guid::create(), L"Input");
	sc->addOutputPin(Guid::create(), L"Output", PtScalar);
	sc->setScript(L"$Output = $Input;\n");
	return sc;
}

Ref< ui::Node > ScriptNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);

	Ref< ui::Node > editorNode = graphControl->createNode(
		scriptNode->getName(),
		L"",
		ui::Point(
			ui::dpi96(scriptNode->getPosition().first),
			ui::dpi96(scriptNode->getPosition().second)
		),
		m_nodeShape
	);

	for (int j = 0; j < scriptNode->getInputPinCount(); ++j)
	{
		const InputPin* inputPin = scriptNode->getInputPin(j);
		editorNode->createInputPin(
			inputPin->getName(),
			!inputPin->isOptional()
		);
	}

	for (int j = 0; j < scriptNode->getOutputPinCount(); ++j)
	{
		const OutputPin* outputPin = scriptNode->getOutputPin(j);
		editorNode->createOutputPin(
			outputPin->getName()
		);
	}

	editorNode->setComment(scriptNode->getComment());
	editorNode->setInfo(scriptNode->getInformation());
	return editorNode;
}

void ScriptNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	m_page->editScript(
		mandatory_non_null_type_cast< Script* >(shaderNode)
	);
}

void ScriptNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::GraphControl* graphControl,
	ui::Node* editorNode,
	ShaderGraph* shaderGraph,
	Node* shaderNode
)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);
	editorNode->setTitle(scriptNode->getName());
	editorNode->setComment(scriptNode->getComment());
	editorNode->setInfo(scriptNode->getInformation());
}

void ScriptNodeFacade::setValidationIndicator(
	ui::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
