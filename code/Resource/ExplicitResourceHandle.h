#ifndef traktor_resource_ExplicitResourceHandle_H
#define traktor_resource_ExplicitResourceHandle_H

#include "Resource/IResourceHandle.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! \brief Explicit resource handle.
 * \ingroup Resource
 */
class T_DLLCLASS ExplicitResourceHandle : public IResourceHandle
{
	T_RTTI_CLASS;

public:
	ExplicitResourceHandle(Object* object);

	virtual void replace(Object* object);

	virtual Object* get() const;

	virtual void flush();

private:
	mutable Ref< Object > m_object;
};

	}
}

#endif	// traktor_resource_ExplicitResourceHandle_H
