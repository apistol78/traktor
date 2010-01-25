#ifndef traktor_video_VideoFactory_H
#define traktor_video_VideoFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace video
	{

/*! \brief Video resource factory.
 * \ingroup Video
 */
class T_DLLCLASS VideoFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	VideoFactory(db::Database* database);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_database;
};

	}
}

#endif	// traktor_video_VideoFactory_H
