#include "Spark/Action/Common/Classes/AsSystem.h"
#include "Spark/Action/Common/Classes/AsSecurity.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AsSystem", AsSystem, ActionObject)

AsSystem::AsSystem(ActionContext* context)
:	ActionObject(context)
{
	setMember("security", ActionValue(new AsSecurity(context)));
}

	}
}
