#ifndef traktor_world_EntityDataTraverser_H
#define traktor_world_EntityDataTraverser_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;

/*! \brief EntityData traverser.
 *
 * Class enabling traversal of entity data; using
 * serialization pattern to extract and replace
 * entity data without explicit methods on the
 * entity data classes themselves.
 */
class T_DLLCLASS EntityDataTraverser : public Object
{
	T_RTTI_CLASS;

public:
	enum VisitorResult
	{
		VrDefault,	/*!< Traverse entity data. */
		VrSkip,		/*!< Skip traversal of entity data. */
		VrReplace	/*!< Replace entity data with substitute. */
	};

	struct IVisitor
	{
		virtual VisitorResult enter (const std::wstring& memberName, EntityData* entityData, Ref< EntityData >& outEntityDataSubst) = 0;

		virtual VisitorResult leave (const std::wstring& memberName, EntityData* entityData, Ref< EntityData >& outEntityDataSubst) = 0;
	};

	EntityDataTraverser(const EntityData* entityData);

	void visit(IVisitor& visitor);

private:
	Ref< const EntityData > m_entityData;
};

	}
}

#endif	// traktor_world_EntityDataTraverser_H
