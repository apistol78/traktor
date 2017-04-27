/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_TypeBrowseFilter_H
#define traktor_editor_TypeBrowseFilter_H

#include "Core/Rtti/TypeInfo.h"
#include "Editor/IBrowseFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Filter instances by type.
 * \ingroup Editor
 */
class T_DLLCLASS TypeBrowseFilter : public IBrowseFilter
{
public:
	TypeBrowseFilter(const TypeInfoSet& typeSet);

	TypeBrowseFilter(const TypeInfo& typeInfo);

	virtual bool acceptable(db::Instance* instance) const T_OVERRIDE T_FINAL;

private:
	TypeInfoSet m_typeSet;
};

	}
}

#endif	// traktor_editor_TypeBrowseFilter_H
