#ifndef traktor_editor_PipelineSettings_H
#define traktor_editor_PipelineSettings_H

#include "Editor/IPipelineSettings.h"

namespace traktor
{
	namespace editor
	{

class PipelineSettings : public IPipelineSettings
{
	T_RTTI_CLASS(PipelineSettings)

public:
	PipelineSettings(const Settings* settings);

	virtual Ref< const PropertyValue > getProperty(const std::wstring& propertyName) const;

	uint32_t getHash() const;

private:
	Ref< const Settings > m_settings;
	mutable uint32_t m_hash;
};

	}
}

#endif	// traktor_editor_PipelineSettings_H
