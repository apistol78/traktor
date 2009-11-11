#include "Editor/PipelineSettings.h"
#include "Core/Serialization/DeepHash.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineSettings", PipelineSettings, IPipelineSettings)

PipelineSettings::PipelineSettings(const Settings* settings)
:	m_settings(settings)
,	m_hash(0)
{
}

Ref< const PropertyValue > PipelineSettings::getProperty(const std::wstring& propertyName) const
{
	Ref< const PropertyValue > prop = m_settings->getProperty(propertyName);
	if (prop)
		m_hash += DeepHash(prop).get();
	return prop;
}

uint32_t PipelineSettings::getHash() const
{
	return m_hash;
}

	}
}
