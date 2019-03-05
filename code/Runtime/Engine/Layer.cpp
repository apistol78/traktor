#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateControl.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Layer.h"
#include "Runtime/Engine/Stage.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.Layer", Layer, Object)

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
