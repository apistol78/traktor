#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace model
	{

class Model;

/*!
 * \ingroup Model
 */
class T_DLLCLASS ModelCache : public Object
{
	T_RTTI_CLASS;

public:
	explicit ModelCache(const Path& cachePath);

	/*! Get model. */
	Ref< Model > get(const Path& fileName, const std::wstring& filter);

private:
	Semaphore m_lock;
	Path m_cachePath;
};

	}
}