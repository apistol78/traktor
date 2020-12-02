#pragma once

#include <functional>
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Singleton/ISingleton.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;
class Object;
class PropertyGroup;

	namespace editor
	{

/*! Data access cache.
 *
 * This cache is useful for storing expensive operations, aka memoization,
 * from previous runs.
 */
class T_DLLCLASS DataAccessCache : public ISingleton
{
public:
	static DataAccessCache& getInstance();

	bool create(const PropertyGroup* settings);

	template< typename ObjectType >
	Ref< ObjectType > read(
		uint32_t key,
		const std::function< Ref< ObjectType > (IStream* stream) >& read,
		const std::function< bool (const ObjectType* object, IStream* stream) >& write,
		const std::function< Ref< ObjectType > () >& create
	)
	{
		return dynamic_type_cast< ObjectType* >(readObject(
			key,
			[&](IStream* stream) -> Ref< Object > { return read(stream); },
			[=](const Object* object, IStream* stream) -> bool { return write(mandatory_non_null_type_cast< const ObjectType* >(object), stream); },
			[&]() -> Ref< Object > { return mandatory_non_null_type_cast< ObjectType* >(create()); }
		));
	}

protected:
	virtual void destroy() override final;

private:
	typedef std::function< Ref< Object > (IStream* stream) > fn_readObject_t;
	typedef std::function< bool (const Object* object, IStream* stream) > fn_writeObject_t;
	typedef std::function< Ref< Object > () > fn_createObject_t;

	Path m_cachePath;

	Ref< Object > readObject(
		uint32_t key,
		const fn_readObject_t& read,
		const fn_writeObject_t& write,
		const fn_createObject_t& create
	);
};

	}
}