#ifndef traktor_json_JsonObject_H
#define traktor_json_JsonObject_H

#include "Core/RefArray.h"
#include "Core/Class/Any.h"
#include "Json/JsonNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace json
	{

class JsonMember;

/*! \brief JSON object
 * \ingroup JSON
 */
class T_DLLCLASS JsonObject : public JsonNode
{
	T_RTTI_CLASS;

public:
	JsonObject();

	/*! \brief Push member onto object.
	 *
	 * \param member Member to push.
	 */
	void push(JsonMember* member);

	/*! \brief Check if object doesn't contain any member.
	 *
	 * \return True if object has no members.
	 */
	bool empty() const { return m_members.empty(); }

	/*! \brief Get first member.
	 *
	 * \return First member.
	 */
	JsonMember* front() { return m_members.front(); }

	/*! \brief Get last member.
	 *
	 * \return Last member.
	 */
	JsonMember* back() { return m_members.back(); }

	/*! \brief Get number of members.
	 *
	 * \return Number of members.
	 */
	uint32_t size() const { return uint32_t(m_members.size()); }

	/*! \brief Get named member.
	 *
	 * \param name Member name.
	 * \return Member node.
	 */
	JsonMember* getMember(const std::wstring& name) const;

	/*! \brief Set named member value.
	 * \note If no such member exist a new member is created.
	 *
	 * \param name Member name.
	 * \param value Value of member.
	 */
	void setMemberValue(const std::wstring& name, const Any& value);

	/*! \brief Get value of named member.
	 * \note If no such member exist undefined is returned.
	 *
	 * \param name Member name.
	 * \return Value of member.
	 */
	Any getMemberValue(const std::wstring& name) const;

	/*! \brief Get value of member path.
	 *
	 * \param path Member path.
	 * \return Value of member.
	 */
	Any getValue(const std::wstring& path) const;

	/*! \brief Get member by index.
	 *
	 * \param index Member index.
	 * \return Member node.
	 */
	JsonMember* get(uint32_t index) { return m_members[index]; }

	/*! \brief Get array of all members.
	 *
	 * \return All member nodes.
	 */
	const RefArray< JsonMember >& get() const { return m_members; }

	virtual bool write(OutputStream& os) const T_OVERRIDE;

private:
	RefArray< JsonMember > m_members;
};

	}
}

#endif	// traktor_json_JsonObject_H
