#ifndef traktor_render_TextureFactory_H
#define traktor_render_TextureFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace render
	{

class IRenderSystem;

/*! \brief Texture resource factory.
 * \ingroup Render
 */
class T_DLLCLASS TextureFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS(TextureFactory)

public:
	TextureFactory(db::Database* db, IRenderSystem* renderSystem);

	virtual const TypeSet getResourceTypes() const;

	virtual bool isCacheable() const;

	virtual Object* create(resource::IResourceManager* resourceManager, const Type& resourceType, const Guid& guid);

private:
	Ref< db::Database > m_db;
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_TextureFactory_H
