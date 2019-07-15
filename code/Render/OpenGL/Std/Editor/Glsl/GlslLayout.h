#pragma once

#include <functional>
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

	GlslResource* get(const std::function< bool(const GlslResource*) >& fn) const
	{
		for (auto resource : m_resources)
		{
			if (fn(resource))
				return resource;
		}
		return nullptr;
	}

	template < typename T >
	T* get(const std::function< bool(const T*) >& fn) const
	{
		for (auto resource : m_resources)
		{
			if (auto typed = dynamic_type_cast< T* >(resource))
			{
				if (fn(typed))
					return typed;
			}
		}
		return nullptr;
	}

	template < typename T >
	T* get(int32_t index) const
	{
		for (auto resource : m_resources)
		{
			if (auto typed = dynamic_type_cast< T* >(resource))
			{
				if (index-- <= 0)
					return typed;
			}
		}
		return nullptr;
	}

	template < typename T >
	RefArray< T > get() const
	{
		RefArray< T > result;
		for (auto resource : m_resources)
		{
			if (auto typed = dynamic_type_cast< T* >(resource))
				result.push_back(typed);
		}
		return result;
	}

	const RefArray< GlslResource >& get() const { return m_resources; }

	uint32_t count(const TypeInfo& resourceType) const;

	template < typename T >
	uint32_t count() const { return count(type_of< T >()); }

private:
	RefArray< GlslResource > m_resources;
};

	}
}
