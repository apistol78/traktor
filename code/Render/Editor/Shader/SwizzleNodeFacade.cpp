#include "Render/Editor/Shader/SwizzleNodeFacade.h"
#include "Render/Node.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/InOutNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SwizzleNodeFacade", SwizzleNodeFacade, NodeFacade)

SwizzleNodeFacade::SwizzleNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = gc_new< ui::custom::InOutNodeShape >(graphControl);
}

Node* SwizzleNodeFacade::createShaderNode(
	const Type* nodeType,
	editor::Editor* editor
)
{
	return checked_type_cast< Node* >(nodeType->newInstance());
}

ui::custom::Node* SwizzleNodeFacade::createEditorNode(
	editor::Editor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Ref< ui::custom::Node > editorNode = gc_new< ui::custom::Node >(
		L"",
		shaderNode->getInformation(),
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);

	editorNode->setColor(Color(220, 255, 255));

	return editorNode;
}

void SwizzleNodeFacade::editShaderNode(
	editor::Editor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
}

void SwizzleNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? Color(220, 255, 255) : Color(255, 120, 120));
}

	}
}
