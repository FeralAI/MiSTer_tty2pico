#ifndef TTY2PICO_FSTS_H
#define TTY2PICO_FSTS_H

#include <stdint.h>
#include "MCU.h"
#include "FsLib/FsVolume.h"
#include "FsLib/FsFile.h"

class FsVolumeTS;

class FsFileTS
{
public:
	FsFileTS() { }
	FsFileTS(File32 file, MCU *mcu) : file32(file), mcu(mcu) { }
	FsFileTS(FsFile file, MCU *mcu) : fsFile(file), mcu(mcu) { }
	~FsFileTS()
	{
		mcu = nullptr;
		if (fsFile) fsFile.close();
		if (file32) file32.close();
	}

	explicit operator bool() const { return file32 || fsFile; }

	static void setActiveVolume(FsVolumeTS *volume) { activeVolume = volume; }

	bool available(void);
	bool close(void);
	uint8_t getError() const;
	size_t getName(char* name, size_t len);
	bool isDir(void);
	FsFileTS openNextFile(void);
	bool openNext(FsBaseFile* dir, oflag_t oflag);
	uint64_t position(void);
	int read(void* buf, size_t count);
	void rewindDirectory(void);
	bool seek(uint64_t position);
	uint64_t size(void);
	size_t write(const void* buf, size_t count);

private:
	static FsVolumeTS *activeVolume;

	MCU *mcu;
	File32 file32;
	FsFile fsFile;
};

class FsVolumeTS
{
public:
	FsVolumeTS() { }
	FsVolumeTS(FatVolume *vol, MCU *mcu) : flashVol(vol), mcu(mcu) { }
	FsVolumeTS(FsVolume *vol, MCU *mcu) : sdVol(vol), mcu(mcu) { }
	~FsVolumeTS()
	{
		mcu = nullptr;
		flashVol = nullptr;
		sdVol = nullptr;
	}

	explicit operator bool() const { return flashVol || sdVol; }

	bool exists(const char *path);
	FsFileTS open(const char *path, oflag_t oflag);

	FatVolume *getFlashVol(void) { return flashVol; }
	FsVolume *getSdVol(void) { return sdVol; }

private:
	MCU *mcu;
	FatVolume *flashVol;
	FsVolume *sdVol;
};

#endif
