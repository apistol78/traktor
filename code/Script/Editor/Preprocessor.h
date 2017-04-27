/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_Preprocessor_H
#define traktor_script_Preprocessor_H

#include <map>
#include "Core/Object.h"

namespace traktor
{
	namespace script
	{

/*! \brief
 * \ingroup Script
 */
class Preprocessor : public Object
{
	T_RTTI_CLASS;

public:
	void setDefinition(const std::wstring& symbol, int32_t value = 0);

	void removeDefinition(const std::wstring& symbol);

	bool evaluate(const std::wstring& source, std::wstring& output, std::set< std::wstring >& usings) const;

private:
	std::map< std::wstring, int32_t > m_definitions;

	int32_t evaluateExpression(const std::wstring& expression) const;
};

	}
}

#endif	// traktor_script_Preprocessor_H
