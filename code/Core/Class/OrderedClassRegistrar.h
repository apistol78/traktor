#ifndef traktor_OrderedClassRegistrar_H
#define traktor_OrderedClassRegistrar_H

#include "Core/RefArray.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Helper to ensure class hierarchy is preserved.
 * \ingroup Core
 */
class T_DLLCLASS OrderedClassRegistrar : public IRuntimeClassRegistrar
{
public:
	virtual void registerClass(IRuntimeClass* runtimeClass) T_OVERRIDE;

	void registerClassesInOrder(IRuntimeClassRegistrar* registrar);

private:
	RefArray< IRuntimeClass > m_runtimeClasses;
};

}

#endif	// traktor_OrderedClassRegistrar_H
