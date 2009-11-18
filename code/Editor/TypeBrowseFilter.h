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
	TypeBrowseFilter(const TypeInfo& type);

	virtual bool acceptable(db::Instance* instance) const;

private:
	const TypeInfo& m_type;
};

	}
}

#endif	// traktor_editor_TypeBrowseFilter_H
