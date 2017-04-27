/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_json_JsonDocument_H
#define traktor_json_JsonDocument_H

#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Json/JsonArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace json
	{

/*! \brief JSON document.
 * \ingroup JSON
 */
class T_DLLCLASS JsonDocument : public JsonArray
{
	T_RTTI_CLASS;

public:
	/*! \brief Load JSON document from file.
	 *
	 * \param fileName Path to file.
	 * \return True if successfully loaded.
	 */
	bool loadFromFile(const Path& fileName);
	
	/*! \brief Load JSON document from stream.
	 *
	 * \param stream Stream to load from.
	 * \return True if successfully loaded.
	 */
	bool loadFromStream(IStream* stream);
	
	/*! \brief Load JSON document from text string.
	 *
	 * \param text Text string.
	 * \return True if successfully loaded.
	 */
	bool loadFromText(const std::wstring& text);

	/*! \brief Save JSON document to file.
	 *
	 * \param fileName Path to file.
	 * \return True if successfully saved.
	 */
	bool saveToFile(const Path& fileName);

	/*! \brief Save JSON document to stream.
	 *
	 * \param stream Stream to save into.
	 * \return True if successfully saved.
	 */
	bool saveToStream(IStream* stream);
};

	}
}

#endif	// traktor_json_JsonDocument_H
