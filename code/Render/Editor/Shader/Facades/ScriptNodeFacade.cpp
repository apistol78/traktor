#include "Core/Misc/SafeDestroy.h"
#include "I18N/Text.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/Script.h"
#include "Render/Editor/Shader/Facades/ScriptNodeDialog.h"
#include "Render/Editor/Shader/Facades/ScriptNodeFacade.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"

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
		ui::custom::GraphControl* graphControl,
		Script* scriptNode
	)
	:	m_editor(editor)
	,	m_graphControl(graphControl)
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
	ui::custom::GraphControl* m_graphControl;
	Script* m_scriptNode;
	Ref< ScriptNodeDialog > m_dialog;

	void eventClick(ui::ButtonClickEvent* event)
	{
		const ui::Command& command = event->getCommand();
		if (command.getId() == ui::DrOk || command.getId() == ui::DrApply)
		{
			// Update script in shader node.
			std::wstring script = m_dialog->getText();
			m_scriptNode->setScript(script);

			// Ensure pins are updated also.
			m_scriptNode->removeAllInputPins();
			m_scriptNode->removeAllOutputPins();

			int32_t inputPinCount = m_dialog->getInputPinCount();
			for (int32_t i = 0; i < inputPinCount; ++i)
			{
				std::wstring pinName = m_dialog->getInputPinName(i);
				ParameterType pinType = m_dialog->getInputPinType(i);
				m_scriptNode->addInputPin(pinName, pinType);
			}

			int32_t outputPinCount = m_dialog->getOutputPinCount();
			for (int32_t i = 0; i < outputPinCount; ++i)
			{
				std::wstring pinName = m_dialog->getOutputPinName(i);
				ParameterType pinType = m_dialog->getOutputPinType(i);
				m_scriptNode->addOutputPin(pinName, pinType);
			}
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

ScriptNodeFacade::ScriptNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = new ui::custom::DefaultNodeShape(graphControl);
}

Ref< Node > ScriptNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return new Script();
}

Ref< ui::custom::Node > ScriptNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);

	Ref< ui::custom::Node > editorNode = new ui::custom::Node(
		scriptNode->getName(),
		L"",
		ui::Point(
			scriptNode->getPosition().first,
			scriptNode->getPosition().second
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
	editorNode->setData(L"INSTANCE", new ScriptNodeInstanceData(editor, graphControl, scriptNode));

	return editorNode;
}

void ScriptNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	Node* shaderNode
)
{
	ScriptNodeInstanceData* instanceData = editorNode->getData< ScriptNodeInstanceData >(L"INSTANCE");
	T_ASSERT (instanceData);

	instanceData->edit();
}

void ScriptNodeFacade::refreshEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	ui::custom::Node* editorNode,
	Node* shaderNode
)
{
	Script* scriptNode = checked_type_cast< Script*, false >(shaderNode);
	editorNode->setTitle(scriptNode->getName());
	editorNode->setComment(scriptNode->getComment());
}

void ScriptNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setState(validationSucceeded ? 0 : 1);
}

	}
}
