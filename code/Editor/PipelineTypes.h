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
	PdfFailed = 8,		//!< Failed dependency.
	PdfForceAdd = 16	//!< Force add even if in exclusive set; used to add ad-hoc root.
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
	uint32_t pipelineHash = 0;		/*!< Hash of pipeline version and settings. */
	uint32_t sourceAssetHash = 0;	/*!< Hash of source instances. */
	uint32_t sourceDataHash = 0;	/*!< Hash of source instance data streams. */
	uint32_t filesHash = 0;			/*!< Hash of external files. */

	bool operator == (const PipelineDependencyHash& rh) const
	{
		return
			pipelineHash == rh.pipelineHash &&
			sourceAssetHash == rh.sourceAssetHash &&
			sourceDataHash == rh.sourceDataHash &&
			filesHash == rh.filesHash;
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
	uint64_t size = 0;			/*!< Size of file. */
	DateTime lastWriteTime;		/*!< Last write time of file. */
	uint32_t hash = 0;			/*!< Hash of file's content. */
};

	}
}

