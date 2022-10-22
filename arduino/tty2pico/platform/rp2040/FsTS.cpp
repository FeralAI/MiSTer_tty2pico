#include "FsTS.h"
#include "MCU.h"
#include "FsLib/FsVolume.h"
#include "FsLib/FsFile.h"

/*******************************************************************************
 * Custom file access implementation
 *******************************************************************************/

FsVolumeTS *FsFileTS::activeVolume = nullptr;

bool FsVolumeTS::exists(const char *path)
{
	if (sdVol)
	{
		mcu->pauseBackground();
		bool exists = sdVol && sdVol->exists(path);
		mcu->resumeBackground();
		return exists;
	}
	else return flashVol->exists(path);
}

FsFileTS FsVolumeTS::open(const char *path, oflag_t oflag)
{
	if (sdVol)
	{
		FsFile tmpFile;
		mcu->pauseBackground();
		bool opened = sdVol && tmpFile.open(sdVol, path, oflag);
		mcu->resumeBackground();
		return opened ? FsFileTS(tmpFile, mcu) : FsFileTS();
	}
	else return FsFileTS(flashVol->open(path, oflag), mcu);
}

bool FsFileTS::available(void)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		int available = fsFile ? fsFile.available() : 0;
		mcu->resumeBackground();
		return available;
	}
	else return file32.available();
}

bool FsFileTS::close(void)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		bool closed = fsFile ? fsFile.close() : true;
		mcu->resumeBackground();
		return closed;
	}
	else return file32.close();
}

uint8_t FsFileTS::getError() const
{
	if (fsFile)
	{
		mcu->pauseBackground();
		uint8_t error = fsFile ? fsFile.getError() : 0;
		mcu->resumeBackground();
		return error;
	}
	else return file32.getError();
}

size_t FsFileTS::getName(char* name, size_t len)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		size_t size = fsFile ? fsFile.getName(name, len) : 0;
		mcu->resumeBackground();
		return size;
	}
	else return file32.getName(name, len);
}

bool FsFileTS::isDir(void)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		bool isDir = fsFile && fsFile.isDir();
		mcu->resumeBackground();
		return isDir;
	}
	else return file32.isDir();
}

FsFileTS FsFileTS::openNextFile(void)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		FsFileTS nextFile = fsFile ? FsFileTS(fsFile.openNextFile(), mcu) : FsFileTS();
		mcu->resumeBackground();
		return nextFile;
	}
	else return FsFileTS(file32.openNextFile(), mcu);
}

bool FsFileTS::openNext(FsBaseFile* dir, oflag_t oflag)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		bool opened = fsFile && fsFile.openNext(dir, oflag);
		mcu->resumeBackground();
		return opened;
	}
	else
	{
		char dirName[255];
		size_t dirNameLength = dir->getName(dirName, 255);
		FatFile tmpFile = activeVolume->getFlashVol()->open(dirName, oflag);
		return file32.openNext(&tmpFile, oflag);
	}
}

uint64_t FsFileTS::position(void)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		uint64_t pos = fsFile ? fsFile.position() : 0;
		mcu->resumeBackground();
		return pos;
	}
	else return file32.position();
}

int FsFileTS::read(void* buf, size_t count)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		int byteCount = fsFile ? fsFile.read(buf, count) : 0;
		mcu->resumeBackground();
		return byteCount;
	}
	else return file32.read(buf, count);
}

void FsFileTS::rewindDirectory(void)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		if (fsFile) fsFile.rewindDirectory();
		mcu->resumeBackground();
	}
	else file32.rewindDirectory();
}

bool FsFileTS::seek(uint64_t position)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		bool success = fsFile && fsFile.seek(position);
		mcu->resumeBackground();
		return success;
	}
	else return file32.seek(position);
}

uint64_t FsFileTS::size(void)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		uint64_t fileSize = fsFile ? fsFile.size() : 0;
		mcu->resumeBackground();
		return fileSize;
	}
	else return file32.size();
}

size_t FsFileTS::write(const void* buf, size_t count)
{
	if (fsFile)
	{
		mcu->pauseBackground();
		size_t byteCount = fsFile ? fsFile.write(buf, count) : 0;
		mcu->resumeBackground();
		return byteCount;
	}
	else return file32.write(buf, count);
}
