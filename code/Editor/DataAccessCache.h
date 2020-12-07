#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ChunkMemory;
class IStream;
class Object;

	namespace editor
	{

/*! Data access cache.
 *
 * This cache is useful for storing expensive operations, aka memoization,
 * from previous runs.
 */
class T_DLLCLASS DataAccessCache : public Object
{
public:
	bool create(const Path& cachePath);

	void destroy();

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

private:
	typedef std::function< Ref< Object > (IStream* stream) > fn_readObject_t;
	typedef std::function< bool (const Object* object, IStream* stream) > fn_writeObject_t;
	typedef std::function< Ref< Object > () > fn_createObject_t;

	Path m_cachePath;
	SmallMap< uint32_t, Ref< ChunkMemory > > m_objectPool;
	Semaphore m_lock;

	Ref< Object > readObject(
		uint32_t key,
		const fn_readObject_t& read,
		const fn_writeObject_t& write,
		const fn_createObject_t& create
	);
};

	}
}