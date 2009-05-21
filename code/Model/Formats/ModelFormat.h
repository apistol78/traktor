#ifndef traktor_model_ModelFormat_H
#define traktor_model_ModelFormat_H

#include "Core/Object.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
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
	T_RTTI_CLASS(ModelFormat)

public:
	enum ImportFlags
	{
		IfMaterials = 1 << 0,
		IfMesh = 1 << 1,
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
	 * Most conventional way might be just to check file
	 * extension.
	 *
	 * \param filePath Path to model file.
	 * \return True if format is supported.
	 */
	virtual bool supportFormat(const Path& filePath) const = 0;

	/*! \brief Read model.
	 *
	 * \param filePath Path to model file.
	 * \param importFlags Import flags.
	 * \return Read model.
	 */
	virtual Model* read(const Path& filePath, uint32_t importFlags = IfAll) const = 0;

	/*! \brief Write model.
	 *
	 * \param filePath Path to new model file.
	 * \param model Output model.
	 * \return True if model written successfully.
	 */
	virtual bool write(const Path& filePath, const Model* model) const = 0;

	/*! \brief Automatically read model using appropriate format.
	 *
	 * \param filePath Path to model file.
	 * \param importFlags Import flags.
	 * \return Read model.
	 */
	static Model* readAny(const Path& filePath, uint32_t importFlags = IfAll);

	/*! \brief Automatically write model using format based on filename extension.
	 *
	 * \param filePath Path to new model file.
	 * \param model Output model.
	 * \return True if model written successfully.
	 */
	static bool writeAny(const Path& filePath, const Model* model);
};

	}
}

#endif	// traktor_model_ModelFormat_H
