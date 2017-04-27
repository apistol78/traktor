/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_String_H
#define traktor_flash_String_H

#include "Flash/Action/ActionObjectRelay.h"

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

/*! \brief ActionScript string wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS String : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	String();

	explicit String(char ch);

	explicit String(const std::string& str);

	void set(const std::string& str) { m_str = str; }

	const std::string& get() const { return m_str; }

private:
	std::string m_str;
};

	}
}

#endif	// traktor_flash_String_H
