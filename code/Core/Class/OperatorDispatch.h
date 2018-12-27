/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

/*! \brief Operator dispatch
 * \ingroup Core
 *
 * Handle multiple dispatches for same operator.
 */
class T_DLLCLASS OperatorDispatch : public IRuntimeDispatch
{
	T_RTTI_CLASS;

public:
	void add(const IRuntimeDispatch* dispatch);

	virtual void signature(OutputStream& ss) const override final;

	virtual Any invoke(ITypedObject* self, uint32_t argc, const Any* argv) const override final;

private:
	RefArray< const IRuntimeDispatch > m_dispatches;
};

}
