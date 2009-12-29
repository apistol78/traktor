#ifndef traktor_flash_ActionArray_H
#define traktor_flash_ActionArray_H

#include "Flash/Action/ActionObject.h"

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

/*! \brief Array of ActionValue objects.
 * \ingroup Flash
 */
class T_DLLCLASS ActionArray : public ActionObject
{
	T_RTTI_CLASS;

public:
	ActionArray();

	Ref< ActionArray > concat() const;

	Ref< ActionArray > concat(const std::vector< ActionValue >& values) const;

	std::wstring join(const std::wstring& delimiter) const;

	void push(const ActionValue& value);

	ActionValue pop();

	uint32_t length() const;

	virtual void setMember(const std::wstring& memberName, const ActionValue& memberValue);

	virtual bool getMember(const std::wstring& memberName, ActionValue& outMemberValue) const;

	virtual std::wstring toString() const;

private:
	std::vector< ActionValue > m_values;
};

	}
}

#endif	// traktor_flash_ActionArray_H
