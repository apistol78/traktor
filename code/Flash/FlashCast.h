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

Any T_DLLCLASS castActionToAny(const ActionValue& value);

ActionValue T_DLLCLASS castAnyToAction(const Any& value);

	}

template < >
struct CastAny < flash::ActionValue, false >
{
	static const wchar_t* const typeName() {
		return L"flash::ActionValue";
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

template < >
struct CastAny < const flash::ActionValue&, false >
{
	static const wchar_t* const typeName() {
		return L"const flash::ActionValue&";
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
