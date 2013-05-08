#ifndef traktor_world_LayerEntityData_H
#define traktor_world_LayerEntityData_H

#include "World/Entity/GroupEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Layer entity data.
 * \ingroup World
 *
 * Used by editor to store editing layer information
 * in a scene asset.
 */
class T_DLLCLASS LayerEntityData : public world::GroupEntityData
{
	T_RTTI_CLASS;

public:
	LayerEntityData();

	virtual void serialize(ISerializer& s);

	void setVisible(bool visible) { m_visible = visible; }

	bool isVisible() const { return m_visible; }

	void setLocked(bool locked) { m_locked = locked; }

	bool isLocked() const { return m_locked; }

	void setInclude(bool include) { m_include = include; }

	bool isInclude() const { return m_include; }

	void setDynamic(bool dynamic) { m_dynamic = dynamic; }

	bool isDynamic() const { return m_dynamic; }

private:
	bool m_visible;
	bool m_locked;
	bool m_include;
	bool m_dynamic;
};

	}
}

#endif	// traktor_world_LayerEntityData_H
