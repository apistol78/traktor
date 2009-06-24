#include "Render/Editor/Shader/ExternalNodeFacade.h"
#include "Render/External.h"
#include "Render/ShaderGraph.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/TypeBrowseFilter.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/PaintSettings.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ExternalNodeFacade", ExternalNodeFacade, NodeFacade)

ExternalNodeFacade::ExternalNodeFacade(ui::custom::GraphControl* graphControl)
{
	m_nodeShape = gc_new< ui::custom::DefaultNodeShape >(graphControl);
}

Node* ExternalNodeFacade::createShaderNode(
	const Type* nodeType,
	editor::IEditor* editor
)
{
	editor::TypeBrowseFilter filter(type_of< ShaderGraph >());
	Ref< db::Instance > fragmentInstance = editor->browseInstance(&filter);
	if (!fragmentInstance)
		return 0;

	Ref< ShaderGraph > fragmentGraph = fragmentInstance->getObject< ShaderGraph >();
	if (!fragmentGraph)
		return 0;

	return gc_new< External >(
		fragmentInstance->getGuid(),
		fragmentGraph
	);
}

ui::custom::Node* ExternalNodeFacade::createEditorNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	std::wstring title;

	Guid fragmentGuid = checked_type_cast< External* >(shaderNode)->getFragmentGuid();

	Ref< editor::IProject > project = editor->getProject();
	Ref< db::Instance > instance = project->getSourceDatabase()->getInstance(fragmentGuid);
	if (instance)
		title = instance->getName();
	else
		title = fragmentGuid.format();

	Ref< ui::custom::Node > editorNode = gc_new< ui::custom::Node >(
		title,
		L"",
		ui::Point(
			shaderNode->getPosition().first,
			shaderNode->getPosition().second
		),
		m_nodeShape
	);

	editorNode->setColor(Color(255, 255, 200));

	return editorNode;
}

void ExternalNodeFacade::editShaderNode(
	editor::IEditor* editor,
	ui::custom::GraphControl* graphControl,
	Node* shaderNode
)
{
	Ref< editor::IProject > project = editor->getProject();
	T_ASSERT (project);

	Ref< db::Instance > instance = project->getSourceDatabase()->getInstance(
		checked_type_cast< External* >(shaderNode)->getFragmentGuid()
	);
	if (instance)
		editor->openEditor(instance);
}

void ExternalNodeFacade::setValidationIndicator(
	ui::custom::Node* editorNode,
	bool validationSucceeded
)
{
	editorNode->setColor(validationSucceeded ? Color(255, 255, 200) : Color(255, 120, 120));
}

	}
}
