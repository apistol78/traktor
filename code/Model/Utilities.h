#ifndef traktor_model_Utilities_H
#define traktor_model_Utilities_H

#include "Core/Config.h"
#include "Core/Math/Aabb3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace model
	{

class Model;

/*! \brief Calculate bounding box volume.
 * \ingroup Model
 *
 * \param model Source model.
 * \return Bounding box.
 */
Aabb3 T_DLLCLASS calculateModelBoundingBox(const Model& model);

/*! \brief Calculate tangent space bases.
 * \ingroup Model
 *
 * Calculate tangent space bases for each vertex.
 *
 * \param model Source model.
 * \param binormals Calculate binormals.
 */
void T_DLLCLASS calculateModelTangents(Model& model, bool binormals);

/*! \brief Triangulate model polygons.
 * \ingroup Model
 *
 * Convert >3 polygons to triangles.
 *
 * \param model Source model.
 */
void T_DLLCLASS triangulateModel(Model& model);

/*! \brief Calculate convex hull from model.
 * \ingroup Model
 *
 * \param model Source model.
 */
void T_DLLCLASS calculateConvexHull(Model& model);

/*! \brief Sort polygons with respect to vertex caches.
 * \ingroup Model
 *
 * \note
 * Assume model has been triangulated.
 *
 * \param model Source model.
 */
void T_DLLCLASS sortPolygonsCacheCoherent(Model& model);

/*! \brief Clean model from duplicated items etc.
 * \ingroup Model
 *
 * \param model Source model.
 */
void T_DLLCLASS cleanDuplicates(Model& model);

/*! \brief Create reversed polygons instead of double sided materials.
 * \ingroup Model
 *
 * \param model Source model.
 */
void T_DLLCLASS flattenDoubleSided(Model& model);

/*! \brief Bake vertex occlusion.
 * \ingroup Model
 *
 * \param model Source model.
 */
void T_DLLCLASS bakeVertexOcclusion(Model& model);

/*! \brief Bake pixel occlusion.
 * \ingroup Model
 *
 * \param model Source model.
 */
Ref< drawing::Image > T_DLLCLASS bakePixelOcclusion(Model& model, int32_t width, int32_t height);

/*! \brief Cull distant faces.
 * \ingroup Model.
 *
 * \param model Source model.
 */
void T_DLLCLASS cullDistantFaces(Model& model);

	}
}

#endif	// traktor_model_Utilities_H
