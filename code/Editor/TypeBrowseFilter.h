#ifndef traktor_editor_TypeBrowseFilter_H
#define traktor_editor_TypeBrowseFilter_H

#include "Core/Rtti/Type.h"
#include "Editor/BrowseFilter.h"

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
class T_DLLCLASS TypeBrowseFilter : public BrowseFilter
{
public:
	TypeBrowseFilter(const Type& type);

	virtual bool acceptable(db::Instance* instance) const;

private:
	const Type& m_type;
};

	}
}

#endif	// traktor_editor_TypeBrowseFilter_H
