/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashCast_H
#define traktor_flash_FlashCast_H

#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Flash/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Cast AS value to Any.
 * \ingroup Flash
 */
Any T_DLLCLASS castActionToAny(const ActionValue& value);

/*! \brief Cast Any value to AS.
 * \ingroup Flash
 */
ActionValue T_DLLCLASS castAnyToAction(const Any& value);

	}

/*! \brief AS value cast rule.
 * \ingroup Flash
 */
template < >
struct CastAny < flash::ActionValue, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"flash::ActionValue";
	}
	static Any set(const flash::ActionValue& value)
	{
		return flash::castActionToAny(value);
	}
	static flash::ActionValue get(const Any& value)
	{
		return flash::castAnyToAction(value);
	}
};

/*! \brief AS value cast rule.
 * \ingroup Flash
 */
template < >
struct CastAny < const flash::ActionValue&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const flash::ActionValue&";
	}
	static Any set(const flash::ActionValue& value)
	{
		return flash::castActionToAny(value);
	}
	static flash::ActionValue get(const Any& value)
	{
		return flash::castAnyToAction(value);
	}
};

}

#endif	// traktor_flash_FlashCast_H
