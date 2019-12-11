#pragma once

#include "Core/RefArray.h"
#include "Core/Class/IRuntimeDispatch.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Polymorphic dispatch
 * \ingroup Core
 *
 * Switch between dispatches based on number of
 * arguments given.
 */
class T_DLLCLASS PolymorphicDispatch : public IRuntimeDispatch
{
	T_RTTI_CLASS;

public:
	void set(uint32_t argc, const IRuntimeDispatch* dispatch);

	virtual void signature(OutputStream& ss) const override final;

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final;

private:
	RefArray< const IRuntimeDispatch > m_dispatches;
};

}

