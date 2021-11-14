#pragma once

#include <functional>
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace world
	{

/*! Resolve external entities, ie flatten scene without external references. */
Ref< ISerializable > T_DLLCLASS resolveExternal(
	const std::function< Ref< const ISerializable >(const Guid& objectId) >& getObjectFn,
	const ISerializable* object,
	const Guid& seed,
	AlignedVector< Guid >* outExternalEntities
);

	}
}
