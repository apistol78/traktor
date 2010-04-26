#ifndef traktor_flash_ActionFunction2_H
#define traktor_flash_ActionFunction2_H

#include "Flash/Action/ActionFunction.h"

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

class ActionDictionary;

/*! \brief ActionScript script function.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunction2 : public ActionFunction
{
	T_RTTI_CLASS;

public:
	enum Flags
	{
		AffPreloadThis			= 0x01,
		AffSuppressThis			= 0x02,
		AffPreloadArguments		= 0x04,
		AffSuppressArguments	= 0x08,
		AffPreloadSuper			= 0x10,
		AffSuppressSuper		= 0x20,
		AffPreloadRoot			= 0x40,
		AffPreloadParent		= 0x80,
		AffPreloadGlobal		= 0x100
	};

	ActionFunction2(
		const std::wstring& name,
		const uint8_t* code,
		uint16_t codeSize,
		uint8_t registerCount,
		uint16_t flags,
		const std::vector< std::pair< std::wstring, uint8_t > >& argumentsIntoRegisters,
		ActionDictionary* dictionary
	);

	virtual ActionValue call(const IActionVM* vm, ActionContext* context, ActionObject* self, const ActionValueArray& args);

	virtual ActionValue call(const IActionVM* vm, ActionFrame* callerFrame, ActionObject* self);

private:
	const uint8_t* m_code;
	uint16_t m_codeSize;
	uint8_t m_registerCount;
	uint16_t m_flags;
	std::vector< std::pair< std::wstring, uint8_t > > m_argumentsIntoRegisters;
	Ref< ActionDictionary > m_dictionary;
};

	}
}

#endif	// traktor_flash_ActionFunction2_H
