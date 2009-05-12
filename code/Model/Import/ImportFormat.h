#ifndef traktor_model_ImportFormat_H
#define traktor_model_ImportFormat_H

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
 * ImportFormat is used to import geometry and material
 * information from proprietary 3rd-party formats.
 */
class T_DLLCLASS ImportFormat : public Object
{
	T_RTTI_CLASS(ImportFormat)

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

	/*! \brief Import model.
	 *
	 * \param filePath Path to model file.
	 * \param importFlags Import flags.
	 * \return Imported model.
	 */
	virtual model::Model* import(const Path& filePath, uint32_t importFlags = IfAll) const = 0;

	/*! \brief Automatically import model using appropriate format.
	 *
	 * \param filePath Path to model file.
	 * \param importFlags Import flags.
	 * \return Imported model.
	 */
	static model::Model* importAny(const Path& filePath, uint32_t importFlags = IfAll);
};

	}
}

#endif	// traktor_model_ImportFormat_H
