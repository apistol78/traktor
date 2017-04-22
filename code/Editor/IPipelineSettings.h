#ifndef traktor_editor_IPipelineSettings_H
#define traktor_editor_IPipelineSettings_H

#include "Core/Object.h"
#include "Core/Settings/IPropertyValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class T_DLLCLASS IPipelineSettings : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< const IPropertyValue > getProperty(const std::wstring& propertyName) const = 0;

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getProperty(const std::wstring& propertyName, typename PropertyTrait< ValueType >::default_value_type_t defaultValue) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return value ? PropertyTrait< ValueType >::property_type_t::get(value) : defaultValue;
	}

	template < typename ValueType >
	typename PropertyTrait< ValueType >::return_type_t getProperty(const std::wstring& propertyName) const
	{
		Ref< const IPropertyValue > value = getProperty(propertyName);
		return PropertyTrait< ValueType >::property_type_t::get(value);
	}
};

	}
}

#endif	// traktor_editor_IPipelineSettings_H
