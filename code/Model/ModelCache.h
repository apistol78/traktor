#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Thread/Mutex.h"

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

class IModelOperation;

/*!
 * \ingroup Model
 */
class T_DLLCLASS ModelCache : public Object
{
    T_RTTI_CLASS;

public:
    ModelCache(
		const Path& cachePath,
		const std::function< Ref< File >(const Path&) >& getFile,
		const std::function< Ref< IStream >(const Path&) >& openStream
	);

    /*! Get model without any operations. */
    Ref< Model > get(const Path& fileName, const std::wstring& filter);

    /*! Get model with applied operations. */
    Ref< Model > get(const Path& fileName, const std::wstring& filter, const RefArray< const IModelOperation >& operations);

    /*! Get model with user key. */
    Ref< Model > get(const Path& fileName, const std::wstring& filter, uint32_t user);

    /*! Put model with user key. */
    void put(const Path& fileName, const std::wstring& filter, const Model* model, uint32_t user);

private:
    Path m_cachePath;
	std::function< Ref< File >(const Path&) > m_getFile;
    std::function< Ref< IStream >(const Path&) > m_openStream;
    Mutex m_lock;
};

    }
}