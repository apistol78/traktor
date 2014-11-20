#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IUpdateInfo.h"
#include "Amalgam/Engine/Layer.h"
#include "Amalgam/Engine/Stage.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Layer", Layer, Object)

Layer::Layer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition
)
:	m_stage(stage)
,	m_name(name)
,	m_permitTransition(permitTransition)
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
