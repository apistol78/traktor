#include "Render/Editor/Shader/InterpolatorNodeFacade.h"
#include "Render/Node.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/IpolNodeShape.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.InterpolatorNodeFacade", InterpolatorNodeFacade, NodeFacade)

InterpolatorNodeFacade::InterpolatorNodeFacade()
{
	m_nodeShape = gc_new< ui::custom::IpolNodeShape >();
}

Node* InterpolatorNodeFacade::createShaderNode(
	const Type* nodeType,
	editor::IEditor* editor
)
{
	return checked_type_cast< Node* >(nodeType->newInstance());
}

ui::custom::Node* InterpolatorNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	return gc_new< ui::custom::Node >(
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
