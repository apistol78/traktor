#ifndef traktor_flash_ActionFunction1_H
#define traktor_flash_ActionFunction1_H

#include "Flash/Action/Avm1/ActionFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionDictionary;

/*! \brief ActionScript script function.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunction1 : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionFunction1(
		const std::wstring& name,
		const std::vector< std::wstring >& arguments,
		const uint8_t* code,
		uint16_t codeSize,
		ActionDictionary* dictionary
	);

	virtual ActionValue call(ActionVM* vm, ActionFrame* callerFrame, ActionObject* self);

private:
	std::vector< std::wstring > m_arguments;
	const uint8_t* m_code;
	uint16_t m_codeSize;
	Ref< ActionDictionary > m_dictionary;
};

	}
}

#endif	// traktor_flash_ActionFunction1_H
