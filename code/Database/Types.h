#ifndef traktor_db_Types_H
#define traktor_db_Types_H

namespace traktor
{
	namespace db
	{

/*! \ingroup Database */
//@{

enum CreateInstanceFlags
{
	CifDefault = 0,			//< Default flag.
	CifReplaceExisting = 1,	//< Replace existing instance.
	CifKeepExistingGuid = 2	//< Keep existing instance's guid.	
};

enum CommitInstanceFlags
{
	CfDefault = 0,
	CfKeepCheckedOut = 1
};

//@}

	}
}

#endif	// traktor_db_Types_H
