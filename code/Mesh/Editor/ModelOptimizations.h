#ifndef traktor_mesh_ModelOptimizations_H
#define traktor_mesh_ModelOptimizations_H

#include "Model/Model.h"

namespace traktor
{
	namespace mesh
	{

/*! \brief Z cover material sorting.
 *
 * Sorting materials by projecting triangles onto axis aligned planes and use it's projected area as sorting key.
 * Thus sorting material by greatest "view independent z cover" in order to minimize overdraw.
 */
void sortMaterialsByProjectedArea(model::Model& model);

	}
}

#endif	// traktor_mesh_ModelOptimizations_H
