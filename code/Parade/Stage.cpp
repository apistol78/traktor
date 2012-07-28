#include "Amalgam/IEnvironment.h"
#include "Amalgam/IStateManager.h"
#include "Amalgam/IUpdateInfo.h"
#include "Parade/Layer.h"
#include "Parade/Stage.h"
#include "Parade/StageLoader.h"
#include "Parade/StageState.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Stage", Stage, Object)

Stage::Stage(
	amalgam::IEnvironment* environment,
	const std::map< std::wstring, Guid >& transitions,
	const Object* params
)
:	m_environment(environment)
,	m_transitions(transitions)
,	m_params(params)
,	m_running(true)
{
}

Stage::~Stage()
{
	destroy();
}

void Stage::destroy()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->destroy();
	m_layers.resize(0);
}

void Stage::addLayer(Layer* layer)
{
	m_layers.push_back(layer);
}

void Stage::removeLayer(Layer* layer)
{
	m_layers.remove(layer);
}

void Stage::removeAllLayers()
{
	m_layers.resize(0);
}

Layer* Stage::findLayer(const std::wstring& name) const
{
	for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

void Stage::terminate()
{
	m_running = false;
}

Ref< Stage > Stage::loadStage(const std::wstring& name, const Object* params)
{
	std::map< std::wstring, Guid >::const_iterator i = m_transitions.find(name);
	if (i == m_transitions.end())
		return 0;

	Ref< StageLoader > stageLoader = StageLoader::create(m_environment, i->second, params);
	if (stageLoader->failed())
		return 0;

	return stageLoader->get();
}

Ref< StageLoader > Stage::loadStageAsync(const std::wstring& name, const Object* params)
{
	std::map< std::wstring, Guid >::const_iterator i = m_transitions.find(name);
	if (i == m_transitions.end())
		return 0;

	return StageLoader::createAsync(m_environment, i->second, params);
}

bool Stage::gotoStage(Stage* stage)
{
	m_pendingStage = stage;
	return true;
}

bool Stage::update(amalgam::IStateManager* stateManager, const amalgam::IUpdateInfo& info)
{
	if (!m_running)
		return false;

	if (!m_pendingStage)
	{
		for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			(*i)->update(this, info);
	}
	else
	{
		stateManager->enter(new StageState(m_environment, m_pendingStage));
		m_pendingStage = 0;
	}

	return true;
}

bool Stage::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->build(this, info, frame);
	return true;
}

void Stage::render(render::EyeType eye, uint32_t frame)
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->render(this, eye, frame);
}

void Stage::leave()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->leave(this);
}

void Stage::reconfigured()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->reconfigured(this);
}

	}
}
