#ifndef traktor_editor_PipelineSettings_H
#define traktor_editor_PipelineSettings_H

#include "Editor/IPipelineSettings.h"

namespace traktor
{

class PropertyGroup;

	namespace editor
	{

class PipelineSettings : public IPipelineSettings
{
	T_RTTI_CLASS;

public:
	PipelineSettings(const PropertyGroup* settings);

	virtual Ref< const IPropertyValue > getProperty(const std::wstring& propertyName) const;

	uint32_t getHash() const;

private:
	Ref< const PropertyGroup > m_settings;
	mutable uint32_t m_hash;
};

	}
}

#endif	// traktor_editor_PipelineSettings_H
