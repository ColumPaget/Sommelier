#include "desktopfiles.h"
#include "platforms.h"
#include "config.h"

#define DESKTOP_PATH "$(homedir)/.local//share/applications/"

static char *DesktopFileMakePath(char *RetStr, TAction *Act)
{
char *Tempstr=NULL;

Tempstr=SubstituteVarsInString(Tempstr, DESKTOP_PATH, Act->Vars, 0);
SetVar(Act->Vars, "desktop-path", Tempstr);
RetStr=SubstituteVarsInString(RetStr, "$(desktop-path)/$(name).desktop",Act->Vars, 0);

Destroy(Tempstr);

return(RetStr);
}


int DesktopFileDelete(TAction *Act)
{
char *Tempstr=NULL;
int result;

Tempstr=DesktopFileMakePath(Tempstr, Act);
result=unlink(Tempstr);

Destroy(Tempstr);
if (result==0) return(TRUE);
return(FALSE);
}

int DesktopFileRead(TAction *Act)
{
char *Tempstr=NULL, *Token=NULL, *Exec=NULL;
const char *ptr;
int result=FALSE;
STREAM *S;

Tempstr=DesktopFileMakePath(Tempstr, Act);
S=STREAMOpen(Tempstr, "r");
if (S)
{
	result=TRUE;
	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		ptr=GetToken(Tempstr, "=", &Token, 0);
		if (strcasecmp(Token,"SommelierExec")==0) 
		{
			Act->Exec=CopyStr(Act->Exec, ptr);	
			StripQuotes(Act->Exec);
		}
		else if (strcasecmp(Token,"Exec")==0) 
		{
			Exec=CopyStr(Exec, ptr);	
			StripQuotes(Exec);
		}
		else if (strcasecmp(Token,"Icon")==0) 
		{
			Token=CopyStr(Token, ptr);
			StripQuotes(Token);
			SetVar(Act->Vars, "icon", Token);
		}
		else if (strcasecmp(Token,"Path")==0) 
		{
			Token=CopyStr(Token, ptr);
			StripQuotes(Token);
			SetVar(Act->Vars, "working-dir", Token);
		}
		Tempstr=STREAMReadLine(Tempstr, S);
	}
	STREAMClose(S);
}
else fprintf(stderr, "ERROR: Failed to open .desktop file '%s' for application\n", Tempstr);

//if we didn't find a 'SommelierExec' entry then we must be dealing with
//a setup that doesn't use sommelier to run the app. Thus the 'Exec' entry
//is the thing that we run
if (! StrValid(Act->Exec)) Act->Exec=CopyStr(Act->Exec, Exec);

//now we rebuild the exec, extracting 'WINEPREFIX' from it if such exists
Tempstr=CopyStr(Tempstr, Act->Exec);
Act->Exec=CopyStr(Act->Exec, "");
ptr=GetToken(Tempstr,"\\S",&Token,GETTOKEN_HONOR_QUOTES);
while (ptr)
{
if (strncmp(Token,"WINEPREFIX=",11)==0) SetVar(Act->Vars, "prefix", Token+11);
else Act->Exec=MCatStr(Act->Exec, Token, " ",NULL);
ptr=GetToken(ptr,"\\S",&Token, GETTOKEN_HONOR_QUOTES);
}

DestroyString(Token);
DestroyString(Tempstr);

return(result);
}



void DesktopFileGenerate(TAction *Act)
{
STREAM *S;
char *Tempstr=NULL, *EmuInvoke=NULL, *Hash=NULL;
const char *ptr;


printf("Generating desktop File...");
printf("exec-path: %s\n", GetVar(Act->Vars, "exec"));
Tempstr=DesktopFileMakePath(Tempstr, Act);
MakeDirPath(Tempstr, 0744);
S=STREAMOpen(Tempstr, "w mode=0744");
if (S)
{
fchmod(S->out_fd, 0744);

switch (PlatformType(Act->Platform))
{
	case PLATFORM_WINDOWS:
		//for windows we must override the found exec-path to be in windows format
		Tempstr=SubstituteVarsInString(Tempstr, "C:\\$(exec-dir)\\$(exec)", Act->Vars, 0);
		strrep(Tempstr, '/', '\\');
		SetVar(Act->Vars,"exec-path", Tempstr);

		if (strcmp(Act->Platform, "win64")==0) Tempstr=SubstituteVarsInString(Tempstr, "WINEARCH=win64 WINEPREFIX=$(prefix) wine \"$(exec-path)\" $(exec-args)", Act->Vars, 0);
		else if (strcmp(Act->Platform, "win32")==0) Tempstr=SubstituteVarsInString(Tempstr, "WINEARCH=win32 WINEPREFIX=$(prefix) wine \"$(exec-path)\" $(exec-args)", Act->Vars, 0);
		else Tempstr=SubstituteVarsInString(Tempstr, "WINEPREFIX=$(prefix) wine '$(exec-path)' $(exec-args)", Act->Vars, 0);

		SetVar(Act->Vars, "invocation", Tempstr);
		SetVar(Act->Vars, "invoke-dir", GetVar(Act->Vars, "working-dir"));
	break;


	case PLATFORM_DOS:
	case PLATFORM_SCUMMVM:
	case PLATFORM_GOGSCUMMVM:
	case PLATFORM_GOGDOS:
	case PLATFORM_GOGWINDOS:
	case PLATFORM_DOOM:
	case PLATFORM_ZXSPECTRUM:
	case PLATFORM_LINUX32:
	case PLATFORM_LINUX64:
		EmuInvoke=PlatformFindEmulator(EmuInvoke, Act->Platform);
		Tempstr=SubstituteVarsInString(Tempstr, EmuInvoke, Act->Vars, 0);
		StripLeadingWhitespace(Tempstr);
		StripTrailingWhitespace(Tempstr);
		SetVar(Act->Vars, "invocation", Tempstr);
		SetVar(Act->Vars, "invoke-dir", GetVar(Act->Vars, "working-dir"));
	break;


	default:
		ptr=GetVar(Act->Vars, "exec");
		if (StrValid(ptr))
		{
		//Tempstr=SubstituteVarsInString(Tempstr, "$(exec-dir)/$(exec)", Act->Vars, 0);
		Tempstr=SubstituteVarsInString(Tempstr, "$(platform-vars) $(exec-vars) $(exec) $(exec-args)", Act->Vars, 0);
		StripLeadingWhitespace(Tempstr);
		StripTrailingWhitespace(Tempstr);

		SetVar(Act->Vars, "invocation", Tempstr);
		}

		ptr=GetVar(Act->Vars, "exec64");
		if (StrValid(ptr))
		{
		Tempstr=SubstituteVarsInString(Tempstr, "$(exec-dir)/$(exec64)", Act->Vars, 0);
		StripLeadingWhitespace(Tempstr);
		StripTrailingWhitespace(Tempstr);

		SetVar(Act->Vars, "invocation64", Tempstr);
		}
		SetVar(Act->Vars, "invoke-dir", GetVar(Act->Vars, "working-dir"));
	break;
}


HashFile(&Hash, "sha256", GetVar(Act->Vars, "exec"), ENCODE_HEX);
SetVar(Act->Vars, "exec-sha256", Hash);

//did we download or otherwise obtain an application icon? If not, then consider the 'icon' setting
//which may point to an icon for this app on local disk
ptr=GetVar(Act->Vars, "app-icon");
if (! StrValid(ptr))
{
	ptr=GetVar(Act->Vars, "icon");
	if (StrValid(ptr))
	{
		Tempstr=SubstituteVarsInString(Tempstr, ptr, Act->Vars, 0);
		if (StrValid(Tempstr) && (access(Tempstr, F_OK)==0)) SetVar(Act->Vars, "app-icon", Tempstr);
	}
}

Tempstr=SubstituteVarsInString(Tempstr, "[Desktop Entry]\nName=$(name)\nType=Application\nTerminal=false\nComment=$(comment)\nSHA256=$(exec-sha256)\nPath=$(invoke-dir)\nExec=sommelier run $(name)\nSommelierExec=$(invocation)\nIcon=$(app-icon)\nRunsWith=$(runswith)\n",Act->Vars, 0);
STREAMWriteLine(Tempstr, S);
Tempstr=SubstituteVarsInString(Tempstr, "Categories=$(category)\nCategory=$(category)\n",Act->Vars, 0);
STREAMWriteLine(Tempstr, S);
STREAMClose(S);

printf(" program invoke: %s\n", GetVar(Act->Vars, "invocation"));
}

DestroyString(Tempstr);
DestroyString(EmuInvoke);
DestroyString(Hash);
}
