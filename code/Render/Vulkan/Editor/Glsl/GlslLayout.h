#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace render
	{
	
class GlslResource;

class GlslLayout : public Object
{
	T_RTTI_CLASS;

public:
	void add(GlslResource* resource);

	GlslResource* get(int32_t binding);

	GlslResource* get(const std::wstring& name);

	template < typename T >
	T* get(int32_t binding) { return checked_type_cast< T* >(get(binding)); }

	const RefArray< GlslResource >& get() const { return m_resources; }

	uint32_t count(const TypeInfo& resourceType) const;

	template < typename T >
	uint32_t count() const { return count(type_of< T >()); }

private:
	RefArray< GlslResource > m_resources;
};

	}
}
