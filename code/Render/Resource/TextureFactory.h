#ifndef traktor_render_TextureFactory_H
#define traktor_render_TextureFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
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

	namespace render
	{

class IRenderSystem;

/*! \brief Texture resource factory.
 * \ingroup Render
 */
class T_DLLCLASS TextureFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	TextureFactory(db::Database* db, IRenderSystem* renderSystem, int32_t skipMips);

	void setSkipMips(int32_t skipMips);

	int32_t getSkipMips() const;

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const;

private:
	Ref< db::Database > m_db;
	Ref< IRenderSystem > m_renderSystem;
	int32_t m_skipMips;
};

	}
}

#endif	// traktor_render_TextureFactory_H
