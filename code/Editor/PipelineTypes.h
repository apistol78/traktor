#pragma once

#include "Core/Date/DateTime.h"

namespace traktor
{
	namespace editor
	{

/*! Pipeline dependency flags.
 * \ingroup Editor
 */
enum PipelineDependencyFlags
{
	PdfBuild = 1,		//!< Build output from dependency.
	PdfUse = 2,			//!< Used by parent dependency; parent needs to be built if this dependency is modified.
	PdfResource = 4,	//!< Dependent as a external resource during runtime.
	PdfFailed = 8		//!< Failed dependency.
};

/*! Pipeline build reasons.
 * \ingroup Editor
 */
enum PipelineBuildReason
{
	PbrNone = 0,				/*!< No reason, buildOutput shouldn't even be called. */
	PbrDependencyModified = 1,	/*!< Dependent resource has been modified. */
	PbrSourceModified = 2,		/*!< Source has been modified. */
	PbrAssetModified = 4,		/*!< Asset has been modified; only valid for Asset classes and are always combined with PbrSourceModified. */
	PbrAdHoc = 8,				/*!< Ad-hoc build. */
	PbrForced = 16				/*!< Forced build issued, must build even if output is up-to-date. */
};

/*! Pipeline dependency hash.
 * \ingroup Editor
 */
struct PipelineDependencyHash
{
	uint32_t pipelineHash;		/*!< Hash of pipeline version and settings. */
	uint32_t sourceAssetHash;	/*!< Hash of source instances. */
	uint32_t sourceDataHash;	/*!< Hash of source instance data streams. */
	uint32_t filesHash;			/*!< Hash of external files. */

	PipelineDependencyHash()
	:	pipelineHash(0)
	,	sourceAssetHash(0)
	,	sourceDataHash(0)
	,	filesHash(0)
	{
	}
};

/*! Pipeline file hash.
 * \ingroup Editor
 *
 * A file hash is the hash of a file's content along
 * with it's last modified time.
 * This permits the pipeline to use already calculated
 * hashes of external files from the pipeline database.
 */
struct PipelineFileHash
{
	uint64_t size;			/*!< Size of file. */
	DateTime lastWriteTime;	/*!< Last write time of file. */
	uint32_t hash;			/*!< Hash of file's content. */

	PipelineFileHash()
	:	size(0)
	,	hash(0)
	{
	}
};

	}
}

