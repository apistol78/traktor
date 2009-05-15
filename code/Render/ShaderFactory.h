#ifndef traktor_render_ShaderFactory_H
#define traktor_render_ShaderFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

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

class RenderSystem;

/*! \brief Shader resource factory.
 * \ingroup Render
 */
class T_DLLCLASS ShaderFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(ShaderFactory)

public:
	ShaderFactory(
		db::Database* db,
		RenderSystem* renderSystem
	);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid, bool& outCacheable);

private:
	Ref< db::Database > m_db;
	Ref< RenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_ShaderFactory_H
