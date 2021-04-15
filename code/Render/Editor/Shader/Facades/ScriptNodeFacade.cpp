#include "Core/Misc/SafeDestroy.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/Script.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEditorPage.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"
#include "Ui/Application.h"
#include "Ui/Graph/DefaultNodeShape.h"
#include "Ui/Graph/GraphControl.h"
#include "Ui/Graph/Node.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class ScriptNodeInstanceData : public Object
{
	T_RTTI_CLASS;

public:
	ScriptNodeInstanceData(
		editor::IEditor* editor,
		ShaderGraphEditorPage* page,
		ui::GraphControl* graphControl,
		ShaderGraph* shaderGraph,
		Script* scriptNode
	)
	:	m_editor(editor)
	,	m_page(page)
	,	m_graphControl(graphControl)
	,	m_shaderGraph(shaderGraph)
	,	m_scriptNode(scriptNode)
	{
	}

	void edit()
	{
		if (m_dialog)
		{
			m_dialog->show();
			return;
		}

		m_dialog = new ScriptNodeDialog(m_editor, m_scriptNode);
		m_dialog->create(m_graphControl);
		m_dialog->show();

		m_dialog->addEventHandler< ui::ButtonClickEvent >(this, &ScriptNodeInstanceData::eventClick);
		m_dialog->addEventHandler< ui::CloseEvent >(this, &ScriptNodeInstanceData::eventCloseDialog);
	}

private:
	editor::IEditor* m_editor;
	ShaderGraphEditorPage* m_page;
	ui::GraphControl* m_graphControl;
	ShaderGraph* m_shaderGraph;
	Script* m_scriptNode;
	Ref< ScriptNodeDialog > m_dialog;

	void eventClick(ui::ButtonClickEvent* event)
	{
		const ui::Command& command = event->getCommand();
		if (command.getId() == ui::DrOk || command.getId() == ui::DrApply)
		{
			// Apply changes to script node.
			bool needCompleteRebuild = false;
			m_dialog->apply(m_shaderGraph, m_scriptNode, needCompleteRebuild);

			// Call back to page to ensure entire editor graph is rebuilt.
			if (needCompleteRebuild)
				m_page->createEditorGraph();
		}
		if (command.getId() == ui::DrOk || command.getId() == ui::DrCancel)
			safeDestroy(m_dialog);
	}

	void eventCloseDialog(ui::CloseEvent* event)
	{
		safeDestroy(m_dialog);
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ScriptNodeInstanceData", ScriptNodeInstanceData, Object)

		}

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
	sc->addInputPin(Guid::create(), L"Input", PtScalar);
	sc->addOutputPin(Guid::create(), L"Output", PtScalar);
	sc->setScript(
		L"ENTRY\n"
		L"{\n"
		L"\tOutput = Input;\n"
		L"}\n"
	);
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
	editorNode->setData(L"INSTANCE", new ScriptNodeInstanceData(editor, m_page, graphControl, shaderGraph, scriptNode));

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
	ScriptNodeInstanceData* instanceData = editorNode->getData< ScriptNodeInstanceData >(L"INSTANCE");
	T_ASSERT(instanceData);

	instanceData->edit();
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
