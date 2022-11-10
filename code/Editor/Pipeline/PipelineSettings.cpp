#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/IPropertyValue.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/Pipeline/PipelineSettings.h"

namespace traktor::editor
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
		m_log << L"\"" << propertyName << L"\"";
		if (prop)
		{
			uint32_t hash = DeepHash(prop).get();
			m_log << L" " << str(L"0x%08x", hash);
			m_hash += hash;
		}
		else if(defaultValue)
		{
			uint32_t hash = DeepHash(defaultValue).get();
			m_log << L" " << str(L"0x%08x", hash) << L" (default)";
			m_hash += hash;
		}
		else
			m_log << L" 0x00000000 (missing)";
		m_log << Endl;
	}
	if (prop)
		return prop;
	else
		return defaultValue;
}

uint32_t PipelineSettings::getHash() const
{
	return m_hash;
}

std::wstring PipelineSettings::getLog() const
{
	return m_log.str();
}

}
