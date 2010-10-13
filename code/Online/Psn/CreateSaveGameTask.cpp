#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Psn/CreateSaveGameTask.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const uint32_t c_maxDirCount = 32;
const uint32_t c_maxFileCount = 32;

char s_indicatorDispMsg[CELL_SAVEDATA_INDICATORMSG_MAX] = "Saving...";

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.CreateSaveGameTask", CreateSaveGameTask, ISaveGameQueueTask)

Ref< ISaveGameQueueTask > CreateSaveGameTask::create(const std::wstring& name, const ISerializable* attachment)
{
	Ref< CreateSaveGameTask > task = new CreateSaveGameTask();

	DynamicMemoryStream dms(task->m_saveBuffer, false, true);
	if (!BinarySerializer(&dms).writeObject(attachment))
		return 0;

	task->m_saveBufferPending = true;
	return task;
}

bool CreateSaveGameTask::execute()
{
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

	int32_t err = cellSaveDataAutoSave2(
		CELL_SAVEDATA_VERSION_CURRENT,
		"NPEB00401-PUZZLED-0001",
		CELL_SAVEDATA_ERRDIALOG_ALWAYS,
		&buf,
		&callbackSaveStat,
		&callbackSaveFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)this
	);
	if (err != CELL_SAVEDATA_RET_OK)
		return false;

	return true;
}

void CreateSaveGameTask::callbackSaveStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	CreateSaveGameTask* this_ = static_cast< CreateSaveGameTask* >(cbResult->userdata);
	T_ASSERT (this_);

	set->setParam = &get->getParam;
	set->reCreateMode = CELL_SAVEDATA_RECREATE_YES;
	set->indicator = &s_indicator;

	if (get->isNewData)
	{
		strncpy(set->setParam->title, "Puzzle Dimension", CELL_SAVEDATA_SYSP_TITLE_SIZE);
		strncpy(set->setParam->subTitle, "", CELL_SAVEDATA_SYSP_SUBTITLE_SIZE);
		strncpy(set->setParam->detail, "", CELL_SAVEDATA_SYSP_DETAIL_SIZE);
		strncpy(set->setParam->listParam, "", CELL_SAVEDATA_SYSP_LPARAM_SIZE);

		set->setParam->attribute = CELL_SAVEDATA_ATTR_NORMAL;

		std::memset(set->setParam->reserved, 0, sizeof(set->setParam->reserved));
		std::memset(set->setParam->reserved2, 0, sizeof(set->setParam->reserved2));	
	}

	this_->m_saveBufferPending = true;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void CreateSaveGameTask::callbackSaveFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	CreateSaveGameTask* this_ = static_cast< CreateSaveGameTask* >(cbResult->userdata);
	T_ASSERT (this_);

	if (this_->m_saveBufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
		set->fileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = this_->m_saveBuffer.size();
		set->fileBufSize = this_->m_saveBuffer.size();
		set->fileBuf = &this_->m_saveBuffer[0];
		set->reserved = 0;

		cbResult->progressBarInc = 50;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		this_->m_saveBufferPending = false;
	}
	else
	{
		cbResult->progressBarInc = 50;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}
}

	}
}
