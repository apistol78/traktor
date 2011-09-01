#ifndef traktor_flash_Array_H
#define traktor_flash_Array_H

#include <algorithm>
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionValueArray.h"

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
class T_DLLCLASS Array : public ActionObject
{
	T_RTTI_CLASS;

public:
	Array();

	Ref< Array > concat() const;

	Ref< Array > concat(const ActionValueArray& values) const;

	std::string join(const std::string& delimiter) const;

	void push(const ActionValue& value);

	ActionValue pop();

	void reverse();

	ActionValue shift();

	uint32_t unshift(const ActionValueArray& values);

	Ref< Array > slice(int32_t startIndex, int32_t endIndex) const;

	Ref< Array > splice(int32_t startIndex, uint32_t deleteCount, const ActionValueArray& values, int32_t offset);

	uint32_t length() const;

	template < typename PredicateType >
	void sort(const PredicateType& predicate)
	{
		std::sort(m_values.begin(), m_values.end(), predicate);
	}

	virtual void setMember(const ActionValue& memberName, const ActionValue& memberValue);

	virtual bool getMember(ActionContext* context, const ActionValue& memberName, ActionValue& outMemberValue);

	virtual ActionValue toString() const;

	const std::vector< ActionValue >& getValues() const { return m_values; }

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	std::vector< ActionValue > m_values;
};

	}
}

#endif	// traktor_flash_Array_H
