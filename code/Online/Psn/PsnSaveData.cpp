/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sysutil/sysutil_gamecontent.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Psn/PsnLogError.h"
#include "Online/Psn/PsnSaveData.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const uint32_t c_maxDirCount = 32;
const uint32_t c_maxFileCount = 32;
const char c_secureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE] =
{
	'C', 'L', 'E', 'A', 
	'R', 'H', 'E', 'A', 
	'D', 'G', 'A', 'M', 
	'E', 'S', ' ', ' ', 
};

char s_dirNamePrefix[] = "NPEB00401-";
char s_loadIndicatorDispMsg[CELL_SAVEDATA_INDICATORMSG_MAX] = "Loading...";
char s_saveIndicatorDispMsg[CELL_SAVEDATA_INDICATORMSG_MAX] = "Saving...";

CellSaveDataAutoIndicator s_loadIndicator = 
{
	dispPosition : CELL_SAVEDATA_INDICATORPOS_LOWER_RIGHT | CELL_SAVEDATA_INDICATORPOS_MSGALIGN_RIGHT,
	dispMode : CELL_SAVEDATA_INDICATORMODE_BLINK,
	dispMsg : s_loadIndicatorDispMsg,
	picBufSize : 0,
	picBuf : NULL,
	reserved : NULL
};

CellSaveDataAutoIndicator s_saveIndicator = 
{
	dispPosition : CELL_SAVEDATA_INDICATORPOS_LOWER_RIGHT | CELL_SAVEDATA_INDICATORPOS_MSGALIGN_RIGHT,
	dispMode : CELL_SAVEDATA_INDICATORMODE_BLINK,
	dispMsg : s_saveIndicatorDispMsg,
	picBufSize : 0,
	picBuf : NULL,
	reserved : NULL
};

struct EnumerateData
{
	PsnSaveData* m_this;
	std::set< std::wstring >& m_saveDataIds;

	EnumerateData(PsnSaveData* this_, std::set< std::wstring >& outSaveDataIds)
	:	m_this(this_)
	,	m_saveDataIds(outSaveDataIds)
	{
		m_saveDataIds.clear();
	}
};

struct LoadData
{
	PsnSaveData* m_this;
	std::vector< uint8_t > m_buffer;
	bool m_bufferPending;

	LoadData(PsnSaveData* this_)
	:	m_this(this_)
	,	m_bufferPending(false)
	{
	}
};

struct SaveDataHelper
{
	PsnSaveData* m_this;
	SaveDataDesc m_desc;
	const std::vector< uint8_t >& m_dataBuffer;
	const uint8_t* m_iconBuffer;
	int32_t m_iconSize;
	bool m_dataPending;
	bool m_iconPending;

	SaveDataHelper(PsnSaveData* this_, const SaveDataDesc& desc, const std::vector< uint8_t >& dataBuffer, const uint8_t* iconBuffer, int32_t iconSize)
	:	m_this(this_)
	,	m_desc(desc)
	,	m_dataBuffer(dataBuffer)
	,	m_iconBuffer(iconBuffer)
	,	m_iconSize(iconSize)
	,	m_dataPending(false)
	,	m_iconPending(false)
	{
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnSaveData", PsnSaveData, ISaveDataProvider)

PsnSaveData::PsnSaveData(int32_t excessSpaceNeededKB, const uint8_t* saveIconBuffer, int32_t saveIconSize)
:	m_excessSpaceNeededKB(excessSpaceNeededKB)
,	m_hddFreeSpaceKB(0)
,	m_spaceNeededKB(0)
,	m_threadDialog(0)
,	m_saveIconBuffer(saveIconBuffer)
,	m_saveIconSize(saveIconSize)
{
}

PsnSaveData::~PsnSaveData()
{
	if (m_threadDialog)
	{
		m_threadDialog->stop();
		ThreadManager::getInstance().destroy(m_threadDialog);
	}
}

bool PsnSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
{
	EnumerateData enumerateData(this, outSaveDataIds);

	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
	);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	CellSaveDataSetList list;
	std::memset(&list, 0, sizeof(list));
	list.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
	list.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT;
	list.dirNamePrefix = s_dirNamePrefix;

	m_currentSavedataSizeKB = 0;

	int32_t err = cellSaveDataListAutoLoad(
		CELL_SAVEDATA_VERSION_420,
		CELL_SAVEDATA_ERRDIALOG_NOREPEAT,
		&list,
		&buf,
		&callbackEnumFixed,
		&callbackEnumStat,
		&callbackEnumFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)&enumerateData
	);

	int32_t neededGameDataSizeKb = (m_currentSavedataSizeKB > 0) ? 4 : 100;
	int32_t needSizeKb = m_hddFreeSpaceKB - m_excessSpaceNeededKB - neededGameDataSizeKb;
	if (needSizeKb < 0)
	{
		m_spaceNeededKB = -needSizeKb;
		m_threadDialog = ThreadManager::getInstance().create(makeFunctor(this, &PsnSaveData::dialogThread), L"Savedata dialog");
		m_threadDialog->start(Thread::Normal);
		log::error << L"Unable to create session manager; Not enough space on HDD to save trophies and save-games." << Endl;
		return false;
	}

	if (err != CELL_SAVEDATA_RET_OK)
	{
		log::error << PsnLogError::getSaveDataErrorString(err) << Endl;
		return false;
	}

	return true;
}

void PsnSaveData::dialogThread()
{
	cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_NOSPACE_EXIT, m_spaceNeededKB, NULL); 
}

bool PsnSaveData::get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	if (m_spaceNeededKB)
		return false;
	LoadData loadData(this); 

	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
	);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	m_currentSavedataSizeKB = 0;

	char dirName[CELL_SAVEDATA_DIRNAME_SIZE];
	std::strcpy(dirName, s_dirNamePrefix);
	std::strcat(dirName, wstombs(toUpper(saveDataId)).c_str());

	int32_t err = cellSaveDataAutoLoad2(
		CELL_SAVEDATA_VERSION_420,
		dirName,
		CELL_SAVEDATA_ERRDIALOG_NOREPEAT,
		&buf,
		&callbackLoadStat,
		&callbackLoadFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)&loadData
	);

	if (err != CELL_SAVEDATA_RET_OK)
	{
		log::error << PsnLogError::getSaveDataErrorString(err) << Endl;
		return false;
	}

	DynamicMemoryStream dms(loadData.m_buffer, true, false);
	outAttachment = BinarySerializer(&dms).readObject();
	return bool(outAttachment != 0);
}

bool PsnSaveData::set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace)
{
	if (m_spaceNeededKB)
		return false;
	DynamicMemoryStream dms(false, true);
	BinarySerializer(&dms).writeObject(attachment);

	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
	);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	char dirName[CELL_SAVEDATA_DIRNAME_SIZE];
	std::strcpy(dirName, s_dirNamePrefix);
	std::strcat(dirName, wstombs(toUpper(saveDataId)).c_str());

	SaveDataHelper saveDataHelper(this, saveDataDesc, dms.getBuffer(), m_saveIconBuffer, m_saveIconSize); 
	int32_t err = cellSaveDataAutoSave2(
		CELL_SAVEDATA_VERSION_420,
		dirName,
		CELL_SAVEDATA_ERRDIALOG_ALWAYS,
		&buf,
		&callbackSaveStat,
		&callbackSaveFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)&saveDataHelper
	);
/*
	if (err == CELL_SAVEDATA_ERROR_CBRESULT && m_spaceNeededKB)
	{
		cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_NOSPACE_EXIT, m_spaceNeededKB, NULL); 
		log::error << L"Not enough space to save" << Endl;
	}
*/
	if (err != CELL_SAVEDATA_RET_OK)
	{
		log::error << PsnLogError::getSaveDataErrorString(err) << Endl;
		return false;
	}

	return true;
}

bool PsnSaveData::remove(const std::wstring& saveDataId)
{
	return false;
}

void PsnSaveData::callbackEnumFixed(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set)
{
	EnumerateData* enumerateData = static_cast< EnumerateData* >(cbResult->userdata);
	T_ASSERT (enumerateData);

	int32_t dirCount = get->dirListNum;
	int32_t prefixLength = strlen(s_dirNamePrefix);

	enumerateData->m_saveDataIds.clear();
	for (int i = 0; i< dirCount; i++)
	{
		const char* saveDataId = &(get->dirList[i].dirName[prefixLength]);
		enumerateData->m_saveDataIds.insert(mbstows(saveDataId));
	}

	set->dirName = s_dirNamePrefix; // Dummy to force next step
	set->newIcon = 0;
	set->option = CELL_SAVEDATA_OPTION_NONE;
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void PsnSaveData::callbackEnumStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	EnumerateData* enumerateData = static_cast< EnumerateData* >(cbResult->userdata);
	T_ASSERT (enumerateData);
	PsnSaveData* this_ = enumerateData->m_this;
	T_ASSERT (this_);

	this_->m_hddFreeSpaceKB = get->hddFreeSizeKB;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

void PsnSaveData::callbackEnumFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

void PsnSaveData::callbackLoadStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	LoadData* loadData = static_cast< LoadData* >(cbResult->userdata);
	T_ASSERT (loadData);
	PsnSaveData* this_ = loadData->m_this;
	T_ASSERT (this_);

	this_->m_hddFreeSpaceKB = get->hddFreeSizeKB;

	if (get->bind != 0)
		log::debug << PsnLogError::getSaveDataBindErrorString(get->bind) << Endl;

	if (get->isNewData)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		return;
	}

	if (get->fileNum > get->fileListNum)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	int32_t i = 0;
	for (; i < get->fileListNum; ++i)
	{
		if (strcmp(get->fileList[i].fileName, "ATT.BIN") == 0)
		{
			loadData->m_buffer.resize(get->fileList[i].st_size);
			loadData->m_bufferPending = true;
			break;
		}
	}
	if (i >= get->fileListNum)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO_NOBROKEN;
	set->setParam = 0;
	set->indicator = &s_loadIndicator;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void PsnSaveData::callbackLoadFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	LoadData* loadData = static_cast< LoadData* >(cbResult->userdata);
	T_ASSERT (loadData);

	cbResult->progressBarInc = 50;

	if (loadData->m_bufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_READ;
		set->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = loadData->m_buffer.size();
		set->fileBufSize = loadData->m_buffer.size();
		set->fileBuf = &loadData->m_buffer[0];
		set->reserved = 0;
		std::memcpy(set->secureFileId, c_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);

		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		loadData->m_bufferPending = false;
	}
	else
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

void PsnSaveData::callbackSaveStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	SaveDataHelper* saveDataHelper = static_cast< SaveDataHelper* >(cbResult->userdata);
	T_ASSERT (saveDataHelper);
	PsnSaveData* this_ = saveDataHelper->m_this;
	T_ASSERT (this_);

	set->setParam = &get->getParam;
	set->reCreateMode = CELL_SAVEDATA_RECREATE_YES;
	set->indicator = &s_saveIndicator;

	if (get->isNewData)
	{
		std::strncpy(set->setParam->title, wstombs(saveDataHelper->m_desc.title).c_str(), CELL_SAVEDATA_SYSP_TITLE_SIZE);
		std::strncpy(set->setParam->subTitle, wstombs(saveDataHelper->m_desc.description).c_str(), CELL_SAVEDATA_SYSP_SUBTITLE_SIZE);
		std::strncpy(set->setParam->detail, "", CELL_SAVEDATA_SYSP_DETAIL_SIZE);
		std::strncpy(set->setParam->listParam, "", CELL_SAVEDATA_SYSP_LPARAM_SIZE);

		set->setParam->attribute = CELL_SAVEDATA_ATTR_NORMAL;

		std::memset(set->setParam->reserved, 0, sizeof(set->setParam->reserved));
		std::memset(set->setParam->reserved2, 0, sizeof(set->setParam->reserved2));	

		const int32_t CONTENT_SIZEKB = (saveDataHelper->m_dataBuffer.size() + 1023) / 1024 + (saveDataHelper->m_iconSize + 1023) / 1024; 
		const int32_t NEW_SIZEKB = CONTENT_SIZEKB + get->sysSizeKB;
		const int32_t NEED_SIZEKB = get->hddFreeSizeKB - NEW_SIZEKB;
		if (NEED_SIZEKB < 0)
		{
			this_->m_spaceNeededKB = -NEED_SIZEKB;
			cbResult->errNeedSizeKB = -NEED_SIZEKB;
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
			return;
		}
	}
	saveDataHelper->m_dataPending = true;
	saveDataHelper->m_iconPending = true;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void PsnSaveData::callbackSaveFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	SaveDataHelper* saveDataHelper = static_cast< SaveDataHelper* >(cbResult->userdata);
	T_ASSERT (saveDataHelper );

	if (saveDataHelper->m_dataPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
		set->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = saveDataHelper->m_dataBuffer.size();
		set->fileBufSize = saveDataHelper->m_dataBuffer.size();
		set->fileBuf = (void*) &saveDataHelper->m_dataBuffer[0];
		std::memcpy(set->secureFileId, c_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);
		set->reserved = 0;

		cbResult->progressBarInc = 33;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		saveDataHelper->m_dataPending = false;
	}
	else if (saveDataHelper->m_iconPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
		set->fileType = CELL_SAVEDATA_FILETYPE_CONTENT_ICON0;
		set->fileOffset = 0;
		set->fileSize = saveDataHelper->m_iconSize;
		set->fileBufSize = saveDataHelper->m_iconSize;
		set->fileBuf = (void*) saveDataHelper->m_iconBuffer;
		std::memcpy(set->secureFileId, c_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);
		set->reserved = 0;

		cbResult->progressBarInc = 33;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		saveDataHelper->m_iconPending = false;
	}
	else
	{
		cbResult->progressBarInc = 34;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}
}

	}
}
