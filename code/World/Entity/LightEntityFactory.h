#ifndef traktor_world_LightEntityFactory_H
#define traktor_world_LightEntityFactory_H

#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Light entity factory.
 * \ingroup World
 */
class T_DLLCLASS LightEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;
	
public:
	virtual const TypeInfoSet getEntityTypes() const;

	virtual Ref< Entity > createEntity(IEntityBuilder* builder, const std::wstring& name, const EntityData& entityData, const Object* instanceData) const;
};
	
	}
}

#endif	// traktor_world_LightEntityFactory_H
