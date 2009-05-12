#ifndef traktor_render_ParameterMap_H
#define traktor_render_ParameterMap_H

#include "Render/Shader.h"

namespace traktor
{
	namespace render
	{

/*! \brief Parameter handle to local index map.
 * \ingroup DX9 Xbox360
 *
 * Each shader have a unique handle to index map.
 * This class exploits the fact that parameter handles
 * are allocated in a sequence thus it's possible to keep
 * a linear map from handle to local index using a simple array.
 * It's important to remember to keep parameter
 * count to a minimum otherwise this will cause
 * major memory overhead.
 */
class ParameterMap
{
public:
	void set(handle_t handle, uint32_t index)
	{
		while (handle >= m_indices.size())
			m_indices.push_back(~0U);
		m_indices[handle] = index;
	}

	uint32_t get(handle_t handle) const
	{
		return handle < m_indices.size() ? m_indices[handle] : ~0U;
	}

private:
	std::vector< uint32_t > m_indices;
};

	}
}

#endif	// traktor_render_ParameterMap_H
