#include "Scene/Editor/ReferencesRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/DefaultNodeShape.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/Pin.h"
#include "Core/Heap/GcNew.h"

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
	m_container = gc_new< ui::Container >();
	if (!m_container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"100%", 0, 0)))
		return false;

	m_referenceGraph = gc_new< ui::custom::GraphControl >();
	if (!m_referenceGraph->create(m_container))
		return false;

	RefArray< EntityAdapter > entityAdapters;
	context->getEntities(entityAdapters, SceneEditorContext::GfDescendants);

	for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		Ref< ui::custom::Node > node = gc_new< ui::custom::Node >(
			cref((*i)->getName()),
			cref((*i)->getTypeName()),
			cref(ui::Point(0, 0)),
			gc_new< ui::custom::DefaultNodeShape >(m_referenceGraph)
		);
		node->addInputPin(gc_new< ui::custom::Pin >(node, L"Referee", ui::custom::Pin::DrInput, false));
		node->addOutputPin(gc_new< ui::custom::Pin >(node, L"References", ui::custom::Pin::DrOutput, false));
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

void ReferencesRenderControl::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
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
