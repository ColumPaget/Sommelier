
#ifndef SOMMELIER_COMMON_H
#define SOMMELIER_COMMON_H

#include "libUseful/libUseful.h"
#include <wait.h>
#include <glob.h>


#define VERSION "3.1"


#define INSTALL_RUN 0
#define INSTALL_UNPACK 1
#define INSTALL_EXECUTABLE 2

#define ACT_NONE 0
#define ACT_INSTALL 1
#define ACT_UNINSTALL 2
#define ACT_RUN 3
#define ACT_LIST 5
#define ACT_LIST 5
#define ACT_SET 8
#define ACT_REBUILD 13
#define ACT_REBUILD_HASHES 14

#define FLAG_DEBUG 1
#define FLAG_FORCE 2
#define FLAG_DEPENDANCY 4
#define FLAG_SANDBOX 64
#define FLAG_NET 128

typedef enum {FILETYPE_UNKNOWN, FILETYPE_MZ, FILETYPE_PE, FILETYPE_ZIP, FILETYPE_MSI, FILETYPE_RAR} TEnumFileTypes;

typedef struct
{
int Type;
int Flags;
int InstallType;
char *ConfigPath;
char *Name;
char *URL;
char *Root;
char *DownName;
char *InstallPath;
char *Exec;
char *Args;
char *Platform;
char *OSVersion;
ListNode *Vars;
} TAction;

char *URLBasename(char *RetStr, const char *URL);
TAction *ActionCreate(int Type, const char *Name);
void ActionDestroy(TAction *Act);
int IdentifyFileType(const char *Path);
int CompareSha256(TAction *Act);

//Some installers fork into background, perhaps calling 'setsid', which means we
//will no longer consider them child processes and will no longer wait for them.
//Holding open a pipe for their output seems to overcome this, and also allows us
//to suppress a lot of crap that they might print out.
void RunProgramAndConsumeOutput(const char *Cmd, int Flags);


#endif
