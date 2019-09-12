#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace cmft
{

struct ClContext;

}

namespace traktor
{
	namespace render
	{

class CubeMap;

/*! Probe filtering processor.
 * \ingroup Render
 */
class T_DLLCLASS ProbeProcessor : public Object
{
	T_RTTI_CLASS;

public:
	ProbeProcessor();

	virtual ~ProbeProcessor();

	bool create();

	void destroy();

	/*! Filter input cube IBL into radiance cube maps.
	 *
	 * \param input Input IBL cube map.
	 * \param solidAngle Solid angle in radians from center of filtering cone.
	 * \param outRadiance Mip chain of radiance cubes.
	 */
	bool radiance(const CubeMap* input, float solidAngle, RefArray< CubeMap >& outRadiance) const;

	/*! Filter input cube IBL into irradiance cube maps.
	 *
	 * \param input Input IBL cube maps.
	 * \param outIrradiance Mip chain of irradiance cubes.
	 */
	bool irradiance(const CubeMap* input, RefArray< CubeMap >& outIrradiance) const;

private:
	cmft::ClContext* m_clContext;
};

	}
}
