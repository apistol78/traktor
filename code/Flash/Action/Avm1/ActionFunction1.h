#ifndef traktor_flash_ActionFunction1_H
#define traktor_flash_ActionFunction1_H

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
class T_DLLCLASS ActionFunction1 : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionFunction1(
		const std::string& name,
		const uint8_t* code,
		uint16_t codeSize,
		uint16_t argumentCount,
		ActionDictionary* dictionary
	);

	virtual ActionValue call(ActionContext* context, ActionObject* self, const ActionValueArray& args);

	virtual ActionValue call(ActionFrame* callerFrame, ActionObject* self);

private:
	const uint8_t* m_code;
	uint16_t m_codeSize;
	uint16_t m_argumentCount;
	Ref< ActionDictionary > m_dictionary;
};

	}
}

#endif	// traktor_flash_ActionFunction1_H
