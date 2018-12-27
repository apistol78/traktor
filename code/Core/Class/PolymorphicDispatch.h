/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_PolymorphicDispatch_H
#define traktor_PolymorphicDispatch_H

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

/*! \brief Polymorphic dispatch
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

#endif	// traktor_PolymorphicDispatch_H
