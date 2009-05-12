#ifndef traktor_ui_ShortcutCommand_H
#define traktor_ui_ShortcutCommand_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Command
 * \ingroup UI
 */
class T_DLLCLASS Command : public Object
{
	T_RTTI_CLASS(Command)

public:
	enum CommandFlags
	{
		CfNone = 0,
		CfId = 1,
		CfName = 2
	};

	Command();

	explicit Command(uint32_t id);

	explicit Command(const std::wstring& name);

	explicit Command(uint32_t id, const std::wstring& name);

	uint32_t getFlags() const;

	uint32_t getId() const;

	const std::wstring& getName() const;

	bool operator == (const Command& command) const;

	bool operator == (uint32_t id) const;

	bool operator == (const std::wstring& name) const;

private:
	uint32_t m_flags;
	uint32_t m_id;
	std::wstring m_name;
};

	}
}

#endif	// traktor_ui_ShortcutCommand_H
