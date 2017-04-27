/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_SearchEvent_H
#define traktor_script_SearchEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace script
	{

class SearchEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	SearchEvent(
		ui::EventSubject* sender,
		const std::wstring& search,
		bool caseSensitive,
		bool wholeWord,
		bool wildCard,
		bool preview
	);

	const std::wstring& getSearch() const { return m_search; }

	bool getCaseSensitive() const { return m_caseSensitive; }

	bool getWholeWord() const { return m_wholeWord; }

	bool getWildCard() const { return m_wildCard; }

	bool isPreview() const { return m_preview; }

private:
	std::wstring m_search;
	bool m_caseSensitive;
	bool m_wholeWord;
	bool m_wildCard;
	bool m_preview;
};

	}
}

#endif	// traktor_script_SearchEvent_H
