/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ShortcutCommand_H
#define traktor_ui_ShortcutCommand_H

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;

public:
	enum CommandFlags
	{
		CfNone = 0,
		CfId = 1,
		CfName = 2,
		CfData = 4
	};

	Command();

	explicit Command(uint32_t id);

	explicit Command(const std::wstring& name);

	explicit Command(uint32_t id, const std::wstring& name);

	explicit Command(const std::wstring& name, Object* data);

	explicit Command(uint32_t id, const std::wstring& name, Object* data);

	uint32_t getFlags() const;

	uint32_t getId() const;

	const std::wstring& getName() const;

	Ref< Object > getData() const;

	bool operator == (const Command& command) const;

	bool operator == (uint32_t id) const;

	bool operator == (const std::wstring& name) const;

private:
	uint32_t m_flags;
	uint32_t m_id;
	std::wstring m_name;
	Ref< Object > m_data;
};

	}
}

#endif	// traktor_ui_ShortcutCommand_H
