#include "Scene/Editor/ReferencesRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Pin.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ReferencesRenderControl", ReferencesRenderControl, ISceneRenderControl)

ReferencesRenderControl::ReferencesRenderControl()
{
}

bool ReferencesRenderControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_referenceGraph = new ui::custom::GraphControl();
	if (!m_referenceGraph->create(m_container))
		return false;

	RefArray< EntityAdapter > entityAdapters;
	context->getEntities(entityAdapters, SceneEditorContext::GfDescendants);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		Ref< ui::custom::Node > node = new ui::custom::Node(
			(*i)->getName(),
			(*i)->getTypeName(),
			ui::Point(0, 0),
			new ui::custom::DefaultNodeShape(m_referenceGraph)
		);
		node->createInputPin(L"Referee", false);
		node->createOutputPin(L"References");
		m_referenceGraph->addNode(node);
	}

	return true;
}

void ReferencesRenderControl::destroy()
{
	if (m_container)
	{
		m_container->destroy();
		m_container = 0;
	}
}

void ReferencesRenderControl::updateWorldRenderer()
{
}

bool ReferencesRenderControl::handleCommand(const ui::Command& command)
{
	return false;
}

void ReferencesRenderControl::update()
{
}

	}
}
