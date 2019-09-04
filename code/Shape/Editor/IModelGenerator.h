#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class Guid;

    namespace editor
    {

class IPipelineBuilder;

    }

    namespace model
    {

class Model;

    }

    namespace shape
    {

/*! Generate model from source object, such as mesh, terrain, spline, solid etc.
 * \ingroup Shape
 */
class IModelGenerator : public Object
{
    T_RTTI_CLASS;

public:
    virtual TypeInfoSet getSupportedTypes() const = 0;

    virtual Ref< model::Model > createModel(
        editor::IPipelineBuilder* pipelineBuilder,
		const std::wstring& assetPath,
        const Object* source
    ) const = 0;

    /*! Prepare source object to reference lightmap.
     *
     * \param pipelineBuilder Pipeline builder.
     * \param source Source object, entity data.
     * \param lightmapId Lightmap which has been created onto model.
     * \param model Model which has been used in tracing, created from createModel method.
     * \return Replacement object if necessary, will be replaced in output scene.
     */
    virtual Ref< Object > modifyOutput(
        editor::IPipelineBuilder* pipelineBuilder,
		const std::wstring& assetPath,
        const Object* source,
        const Guid& lightmapId,
        const model::Model* model
    ) const = 0;
};

    }
}