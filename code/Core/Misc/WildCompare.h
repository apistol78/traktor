/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_WildCompare_H
#define traktor_WildCompare_H

#include <string>
#include <vector>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Compare string with wild-card matching.
 * \ingroup Core
 */
class T_DLLCLASS WildCompare
{
public:
	enum CompareMode
	{
		CmIgnoreCase,
		CmCaseSensitive
	};

	WildCompare(const std::wstring& mask);

	/*! \brief Match string with wild-card pattern. */
	bool match(const std::wstring& str, CompareMode mode = CmIgnoreCase, std::vector< std::wstring >* outPieces = 0) const;

	/*! \brief Insert string pieces at wild-card tokens */
	std::wstring merge(const std::vector< std::wstring >& pieces) const;

private:
	std::vector< std::wstring > m_nowild;
	bool m_beginWild;
	bool m_endWild;
};

}

#endif	// traktor_WildCompare_H
