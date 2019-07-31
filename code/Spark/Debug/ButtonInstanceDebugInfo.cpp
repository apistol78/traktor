#include "Spark/ButtonInstance.h"
#include "Spark/Debug/ButtonInstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ButtonInstanceDebugInfo", 0, ButtonInstanceDebugInfo, InstanceDebugInfo)

ButtonInstanceDebugInfo::ButtonInstanceDebugInfo()
{
}

ButtonInstanceDebugInfo::ButtonInstanceDebugInfo(const ButtonInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getLocalBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
}

void ButtonInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);
}

	}
}
