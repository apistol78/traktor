#ifndef traktor_editor_PipelineSettings_H
#define traktor_editor_PipelineSettings_H

#include "Editor/IPipelineSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace editor
	{

class T_DLLCLASS PipelineSettings : public IPipelineSettings
{
	T_RTTI_CLASS;

public:
	PipelineSettings(const PropertyGroup* settings);

	virtual Ref< const IPropertyValue > getProperty(const std::wstring& propertyName) const T_OVERRIDE T_FINAL;

	uint32_t getHash() const;

private:
	Ref< const PropertyGroup > m_settings;
	mutable uint32_t m_hash;
};

	}
}

#endif	// traktor_editor_PipelineSettings_H
