#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IUpdateInfo.h"
#include "Parade/Layer.h"
#include "Parade/Stage.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Layer", Layer, Object)

Layer::Layer(
	Stage* stage,
	const std::wstring& name
)
:	m_stage(stage)
,	m_name(name)
{
}

Layer::~Layer()
{
	destroy();
}

void Layer::destroy()
{
	m_stage = 0;
}

	}
}
