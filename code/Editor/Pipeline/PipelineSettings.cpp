#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/IPropertyValue.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/Pipeline/PipelineSettings.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineSettings", PipelineSettings, IPipelineSettings)

PipelineSettings::PipelineSettings(const PropertyGroup* settings)
:	m_settings(settings)
,	m_hash(0)
{
}

Ref< const IPropertyValue > PipelineSettings::getProperty(const std::wstring& propertyName, bool includeInHash, const IPropertyValue* defaultValue) const
{
	Ref< const IPropertyValue > prop = m_settings->getProperty(propertyName);
	if (includeInHash)
	{
		if (prop)
			m_hash += DeepHash(prop).get();
		else if(defaultValue)
			m_hash += DeepHash(defaultValue).get();
	}
	return prop != nullptr ? prop : defaultValue;
}

uint32_t PipelineSettings::getHash() const
{
	return m_hash;
}

	}
}
