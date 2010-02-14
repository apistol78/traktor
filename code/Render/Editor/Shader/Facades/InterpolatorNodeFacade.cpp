#include "Render/Shader/Node.h"
#include "Render/Editor/Shader/Facades/InterpolatorNodeFacade.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/IpolNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.InterpolatorNodeFacade", InterpolatorNodeFacade, NodeFacade)

InterpolatorNodeFacade::InterpolatorNodeFacade()
{
	m_nodeShape = new ui::custom::IpolNodeShape();
}

Ref< Node > InterpolatorNodeFacade::createShaderNode(
	const TypeInfo* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->createInstance());
}

Ref< ui::custom::Node > InterpolatorNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	return new ui::custom::Node(
		L"",
		L"",
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);
}

void InterpolatorNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
}

void InterpolatorNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? Color(255, 255, 255) : Color(255, 120, 120));
}

	}
}
