
#ifndef _FSYS_H
#define _FSYS_H
//****************************************************************************
//**
//**    flpydsk.h
//**
//****************************************************************************

//============================================================================
//    INTERFACE REQUIRED HEADERS
//============================================================================

#include <stdint.h>

//============================================================================
//    INTERFACE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================
//============================================================================
//    INTERFACE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    INTERFACE STRUCTURES / UTILITY CLASSES
//============================================================================

/**
*	File
*/
typedef struct _FILE
{

	char name[32];
	uint32_t flags;
	uint32_t fileLength;
	uint32_t id;
	uint32_t eof;
	uint32_t position;
	uint32_t currentCluster;
	uint32_t deviceID;

} FILE, *PFILE;

/**
*	Filesystem interface
*/
typedef struct _FILE_SYSTEM
{

	char Name[8];
	FILE(*Directory)
	(const char *DirectoryName);
	void (*Mount)();
	void (*Read)(PFILE file, unsigned char *Buffer, unsigned int Length);
	void (*Close)(PFILE);
	FILE(*Open)
	(const char *FileName);

} FILESYSTEM, *PFILESYSTEM;

/**
*	File flags
*/
#define FS_FILE 0
#define FS_DIRECTORY 1
#define FS_INVALID 2

//============================================================================
//    INTERFACE DATA DECLARATIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTION PROTOTYPES
//============================================================================

FILE volOpenFile(const char *fname);
void volReadFile(PFILE file, unsigned char *Buffer, unsigned int Length);
void volCloseFile(PFILE file);
void volRegisterFileSystem(PFILESYSTEM, unsigned int deviceID);
void volUnregisterFileSystem(PFILESYSTEM);
void volUnregisterFileSystemByID(unsigned int deviceID);

//============================================================================
//    INTERFACE OBJECT CLASS DEFINITIONS
//============================================================================
//============================================================================
//    INTERFACE TRAILING HEADERS
//============================================================================
//****************************************************************************
//**
//**    END [fsys.h]
//**
//****************************************************************************

#endif
