/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_i18n_Dictionary_H
#define traktor_i18n_Dictionary_H

#include <map>
#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace i18n
	{

/*! \brief Dictionary
 * \ingroup I18N
 */
class T_DLLCLASS Dictionary : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool has(const std::wstring& id) const;

	void set(const std::wstring& id, const std::wstring& text);

	void remove(const std::wstring& id);

	bool get(const std::wstring& id, std::wstring& outText) const;

	const std::map< std::wstring, std::wstring >& get() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, std::wstring > m_map;
};

	}
}

#endif	// traktor_i18n_Dictionary_H
