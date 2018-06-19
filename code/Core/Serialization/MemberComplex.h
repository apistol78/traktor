/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MemberComplex_H
#define traktor_MemberComplex_H

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

class Attribute;
class ISerializer;

/*! \brief Complex member base.
 * \ingroup Core
 */
class T_DLLCLASS MemberComplex
{
public:
	MemberComplex(const wchar_t* const name, bool compound);

	MemberComplex(const wchar_t* const name, bool compound, const Attribute& attributes);

	virtual ~MemberComplex();

	/*! \brief Get member name.
	 *
	 * \return Member name.
	 */
	const wchar_t* const getName() const { return m_name; }
	
	/*! \brief Get member attributes.
	 *
	 * Get member attributes if applicable.
	 *
	 * \return Member attributes.
	 */
	const Attribute* getAttributes() const { return m_attributes; }

	/*! \brief Check if member is compound. 
	 *
	 * \return True if member is compound.
	 */
	bool getCompound() const { return m_compound; }

	virtual void serialize(ISerializer& s) const = 0;
	
private:
	const wchar_t* const m_name;
	bool m_compound;
	const Attribute* m_attributes;
};
	
}

#endif	// traktor_MemberComplex_H
