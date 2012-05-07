#ifndef traktor_scene_LayerEntityData_H
#define traktor_scene_LayerEntityData_H

#include "World/Entity/GroupEntityData.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Layer entity data.
 *
 * Used by scene editor to store editing layer information
 * in a scene asset.
 */
class LayerEntityData : public world::GroupEntityData
{
	T_RTTI_CLASS;

public:
	LayerEntityData();

	virtual bool serialize(ISerializer& s);

	void setVisible(bool visible) { m_visible = visible; }

	bool isVisible() const { return m_visible; }

	void setLocked(bool locked) { m_locked = locked; }

	bool isLocked() const { return m_locked; }

	void setInclude(bool include) { m_include = include; }

	bool isInclude() const { return m_include; }

private:
	bool m_visible;
	bool m_locked;
	bool m_include;
};

	}
}

#endif	// traktor_scene_LayerEntityData_H
