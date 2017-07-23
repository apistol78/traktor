/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ddc_DfnAlias_H
#define ddc_DfnAlias_H

#include <Core/Ref.h>
#include "Ddc/DfnNode.h"

namespace ddc
{

class DfnAlias : public DfnNode
{
	T_RTTI_CLASS;

public:
	DfnAlias();

	DfnAlias(
		const std::wstring& language,
		DfnNode* type,
		const std::wstring& languageType,
		const std::wstring& languageArgType,
		const std::wstring& languageMember
	);

	virtual bool serialize(traktor::ISerializer& s);

	const std::wstring& getLanguage() const { return m_language; }

	const DfnNode* getType() const { return m_type; }

	const std::wstring& getLanguageType() const { return m_languageType; }

	const std::wstring& getLanguageArgType() const { return m_languageArgType; }

	const std::wstring& getLanguageMember() const { return m_languageMember; }

private:
	std::wstring m_language;
	traktor::Ref< DfnNode > m_type;
	std::wstring m_languageType;
	std::wstring m_languageArgType;
	std::wstring m_languageMember;
};

}

#endif	// ddc_DfnAlias_H
