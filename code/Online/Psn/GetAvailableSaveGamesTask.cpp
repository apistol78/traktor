#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Psn/GetAvailableSaveGamesTask.h"
#include "Online/Psn/SaveGamePsn.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const uint32_t c_maxDirCount = 32;
const uint32_t c_maxFileCount = 32;

char s_indicatorDispMsg[CELL_SAVEDATA_INDICATORMSG_MAX] = "Loading...";

CellSaveDataAutoIndicator s_indicator = 
{
	dispPosition : CELL_SAVEDATA_INDICATORPOS_LOWER_RIGHT | CELL_SAVEDATA_INDICATORPOS_MSGALIGN_RIGHT,
	dispMode : CELL_SAVEDATA_INDICATORMODE_BLINK,
	dispMsg : s_indicatorDispMsg,
	picBufSize : 0,
	picBuf : NULL,
	reserved : NULL
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GetAvailableSaveGamesTask", GetAvailableSaveGamesTask, ISaveGameQueueTask)

Ref< ISaveGameQueueTask > GetAvailableSaveGamesTask::create(RefArray< ISaveGame >& outSaveGames)
{
	Ref< GetAvailableSaveGamesTask > task = new GetAvailableSaveGamesTask();
	task->m_outSaveGames = &outSaveGames;
	return task;
}

bool GetAvailableSaveGamesTask::execute()
{
	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
	);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetList list;
	std::memset(&list, 0, sizeof(list));
	list.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
	list.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT;
	list.dirNamePrefix = "NPEB00401-PUZZLED-";

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	int32_t err = cellSaveDataListAutoLoad(
		CELL_SAVEDATA_VERSION_CURRENT,
		CELL_SAVEDATA_ERRDIALOG_NOREPEAT,
		&list,
		&buf,
		&callbackLoadFixed,
		&callbackLoadStat,
		&callbackLoadFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)this
	);
	if (err != CELL_SAVEDATA_RET_OK)
		return false;

	return true;
}

void GetAvailableSaveGamesTask::callbackLoadFixed(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set)
{
	set->dirName = "NPEB00401-PUZZLED-0001";
	set->newIcon = 0;
	set->option = CELL_SAVEDATA_OPTION_NONE;

	int32_t i = 0;
	for (; i < get->dirListNum; ++i)
	{
		if (strcmp(get->dirList[i].dirName, set->dirName) == 0)
			break;
	}

	if (i < get->dirListNum)
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
	else
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
}

void GetAvailableSaveGamesTask::callbackLoadStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	GetAvailableSaveGamesTask* this_ = static_cast< GetAvailableSaveGamesTask* >(cbResult->userdata);
	T_ASSERT (this_);

	if (get->isNewData)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
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
			this_->m_loadBuffer.resize(get->fileList[i].st_size);
			this_->m_loadBufferPending = true;
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
	set->indicator = &s_indicator;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void GetAvailableSaveGamesTask::callbackLoadFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	GetAvailableSaveGamesTask* this_ = static_cast< GetAvailableSaveGamesTask* >(cbResult->userdata);
	T_ASSERT (this_);

	cbResult->progressBarInc = 50;

	if (this_->m_loadBufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_READ;
		set->fileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = this_->m_loadBuffer.size();
		set->fileBufSize = this_->m_loadBuffer.size();
		set->fileBuf = &this_->m_loadBuffer[0];
		set->reserved = 0;

		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		this_->m_loadBufferPending = false;
	}
	else
	{
		DynamicMemoryStream dms(this_->m_loadBuffer, true, false);
		Ref< ISerializable > attachment = BinarySerializer(&dms).readObject();
		if (attachment)
		{
			this_->m_outSaveGames->push_back(new SaveGamePsn(L"", attachment));
			cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		}
		else
		{
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		}
	}
}

	}
}
