#pragma once

#include <string>
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

	RefArray< GlslResource > get(uint8_t stageMask) const;

	uint32_t count(const TypeInfo& resourceType, uint8_t stageMask = ~0) const;

	template < typename T >
	uint32_t count(uint8_t stageMask = ~0) const { return count(type_of< T >(), stageMask); }

private:
	RefArray< GlslResource > m_resources;
};

	}
}
