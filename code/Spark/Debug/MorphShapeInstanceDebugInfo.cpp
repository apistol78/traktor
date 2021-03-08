#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/MorphShapeInstance.h"
#include "Spark/Debug/MorphShapeInstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.MorphShapeInstanceDebugInfo", 0, MorphShapeInstanceDebugInfo, InstanceDebugInfo)

MorphShapeInstanceDebugInfo::MorphShapeInstanceDebugInfo(const MorphShapeInstance* instance, bool mask, bool clipped)
{
	m_name = instance->getName();
	m_bounds = instance->getBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_mask = mask;
	m_clipped = clipped;
}

void MorphShapeInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> Member< bool >(L"mask", m_mask);
	s >> Member< bool >(L"clipped", m_clipped);
}

	}
}
