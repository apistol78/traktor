#ifndef traktor_model_ModelFormat_H
#define traktor_model_ModelFormat_H

#include "Core/Object.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace model
	{

class Model;

/*! \brief Model importer format.
 * \ingroup Model
 *
 * ModelFormat is used to import geometry and material
 * information from proprietary 3rd-party formats.
 */
class T_DLLCLASS ModelFormat : public Object
{
	T_RTTI_CLASS;

public:
	enum ImportFlags
	{
		IfMaterials = 1,
		IfMeshPositions = 2,
		IfMeshVertices = 4,
		IfMeshPolygons = 8,
		IfMeshBlendTargets = 16,
		IfMeshBlendWeights = 32,
		IfMesh = (IfMeshPositions | IfMeshVertices | IfMeshPolygons | IfMeshBlendTargets | IfMeshBlendWeights),
		IfAll = (IfMaterials | IfMesh)
	};

	/*! \brief Get list of common extensions of implemented format.
	 *
	 * \param outDescription Human readable description of format.
	 * \param outExtensions List of file extensions commonly used for format.
	 */
	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const = 0;

	/*! \brief Determine if format support parsing file.
	 *
	 * \param extension Model extension.
	 * \return True if format is supported.
	 */
	virtual bool supportFormat(const std::wstring& extension) const = 0;

	/*! \brief Read model.
	 *
	 * \param stream Source stream.
	 * \param importFlags Import flags.
	 * \return Read model.
	 */
	virtual Ref< Model > read(IStream* stream, uint32_t importFlags = IfAll) const = 0;

	/*! \brief Write model.
	 *
	 * \param stream Output stream.
	 * \param model Output model.
	 * \return True if model written successfully.
	 */
	virtual bool write(IStream* stream, const Model* model) const = 0;

	/*! \brief Automatically read model using appropriate format.
	 *
	 * \param filePath Path to model file.
	 * \param importFlags Import flags.
	 * \return Read model.
	 */
	static Ref< Model > readAny(const Path& filePath, uint32_t importFlags = IfAll);

	/*! \brief Automatically read model using appropriate format.
	 *
	 * \param file Source stream.
	 * \param extension File format extension.
	 * \param importFlags Import flags.
	 * \return Read model.
	 */
	static Ref< Model > readAny(IStream* stream, const std::wstring& extension, uint32_t importFlags = IfAll);

	/*! \brief Automatically write model using format based on filename extension.
	 *
	 * \param filePath Path to new model file.
	 * \param model Output model.
	 * \return True if model written successfully.
	 */
	static bool writeAny(const Path& filePath, const Model* model);

	/*! \brief Automatically write model using format based on filename extension.
	 *
	 * \param filePath Path to new model file.
	 * \param model Output model.
	 * \return True if model written successfully.
	 */
	static bool writeAny(IStream* stream, const std::wstring& extension, const Model* model);
};

	}
}

#endif	// traktor_model_ModelFormat_H
