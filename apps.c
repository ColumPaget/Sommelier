#include "apps.h"
#include "config.h"

ListNode *Apps=NULL;

ListNode *AppsGetList()
{
return(Apps);
}


int AppMatches(TAction *Template, TAction *App)
{
int result=FALSE;
char *Token=NULL;
const char *ptr;

	if (StrValid(Template->Platform) && (strcasecmp(App->Platform, Template->Platform)!=0) ) return(FALSE);

	/* doesn't yet exist!
	if (StrValid(Template->Category))
	{
		ptr=GetToken(App->Category, ";", &Token, 0)
		while (ptr)
		{
			if (strcasecmp(Token, Template->Category)==0) result=TRUE;
		ptr=GetToken(ptr, ";", &Token, 0)
		}
	}
	*/

	Destroy(Token);
	return(TRUE);
}

void LoadAppConfigToAct(TAction *Act, const char *Config)
{
char *Name=NULL, *Value=NULL, *Tempstr=NULL;
const char *ptr;

ptr=GetNameValuePair(Config," ", "=", &Name, &Value);
while (ptr)
{
StripQuotes(Name);
StripQuotes(Value);
if (strcmp(Name,"url")==0) 
{
	Act->URL=CopyStr(Act->URL, Value);
	Tempstr=URLBasename(Tempstr, Act->URL);
	//SetVar(Act->Vars, "exec", Tempstr);
}
else if (strcmp(Name,"install-path")==0) Act->InstallPath=CopyStr(Act->InstallPath, Value);
else if (strcmp(Name,"dlname")==0) Act->DownName=CopyStr(Act->DownName, Value);
else if (strcmp(Name,"platform")==0) Act->Platform=CopyStr(Act->Platform, Value);
else if (strcmp(Name,"install-type")==0) 
{
	if (strcasecmp(Value,"unpack")==0) Act->InstallType=INSTALL_UNPACK;
	if (strcasecmp(Value,"executable")==0) Act->InstallType=INSTALL_EXECUTABLE;
}
else if (strcmp(Name,"sha256")==0) 
{
	strlwr(Value);
	SetVar(Act->Vars, Name, Value);
}

else if (strcmp(Name,"bundled")==0)
{
	//app is bundled with another app! Set the bundled flag, and set a variable
	Act->Flags |= FLAG_BUNDLED;
	SetVar(Act->Vars, "bundled-with", Value);
}
else if (StrValid(Name)) SetVar(Act->Vars, Name, Value);

ptr=GetNameValuePair(ptr," ", "=", &Name, &Value);
}

Destroy(Tempstr);
Destroy(Value);
Destroy(Name);
}



void AppsLoadFromFile(const char *Path, ListNode *Apps)
{
STREAM *S;
ListNode *Node;
TAction *Act;
char *Tempstr=NULL, *Token=NULL;
char *GlobalSettings=NULL;
const char *ptr;

S=STREAMOpen(Path, "r");
if (S)
{
Tempstr=STREAMReadLine(Tempstr, S);
while (Tempstr)
{
StripTrailingWhitespace(Tempstr);
if ((StrLen(Tempstr) > 0) && (*Tempstr != '#'))
{
	if (StrValid(GlobalSettings)) Tempstr=MCatStr(Tempstr, " ", GlobalSettings, NULL);
	ptr=GetToken(Tempstr, "\\S", &Token, GETTOKEN_QUOTES);

	// '*' for the app name means this line applies to all apps
	
	if (strcmp(Token, "*")==0) GlobalSettings=CopyStr(GlobalSettings, ptr);
	else
	{
	Act=ActionCreate(ACT_NONE, Token);
	ListAddNamedItem(Apps, Token, Act);
	LoadAppConfigToAct(Act, ptr);
	}
	}
Tempstr=STREAMReadLine(Tempstr, S);
}

STREAMClose(S);
}

Destroy(GlobalSettings);
Destroy(Tempstr);
Destroy(Token);
}


//bundles are situations were more than one application is supplied in the same package
void AppAddToBundle(ListNode *Apps, TAction *App, const char *ParentName)
{
ListNode *Curr;
TAction *Parent;
char *Tempstr=NULL;

//find the parent app/install that this item is bundled with
Curr=ListFindNamedItem(Apps, ParentName);
while (Curr)
{
Parent=(TAction *) Curr->Item;
Tempstr=CopyStr(Tempstr, GetVar(Parent->Vars, "bundles"));
Tempstr=MCatStr(Tempstr, " ", App->Name, NULL);
SetVar(Parent->Vars, "bundles", Tempstr);
Curr=ListFindNamedItem(Curr, ParentName);
}

Destroy(Tempstr);
}

void AppsProcessBundles(ListNode *Apps)
{
ListNode *Curr;
TAction *App;
const char *ptr;

Curr=ListGetNext(Apps);
while (Curr)
{
	App=(TAction *) Curr->Item;
	if (App->Flags & FLAG_BUNDLED)
	{
		ptr=GetVar(App->Vars, "bundled-with");
		if (StrValid(ptr)) AppAddToBundle(Apps, App, ptr);
	}
	Curr=ListGetNext(Curr);
}

}

ListNode *AppsLoad(const char *ConfigFiles)
{
char *Tempstr=NULL, *Token=NULL;
const char *ptr;
ListNode *Vars;
glob_t Glob;
int i;

if (! Apps) Apps=ListCreate();
Vars=ListCreate();
SetVar(Vars, "homedir", GetCurrUserHomeDir());

ptr=GetToken(ConfigFiles, ",", &Token, 0);
while (ptr)
{
	Tempstr=SubstituteVarsInString(Tempstr, Token, Vars, 0);
	glob(Tempstr, 0, 0, &Glob);
	for (i=0; i < Glob.gl_pathc; i++) AppsLoadFromFile(Glob.gl_pathv[i], Apps);
	globfree(&Glob);
	ptr=GetToken(ptr, ",", &Token, 0);
}

AppsProcessBundles(Apps);

ListDestroy(Vars, Destroy);
Destroy(Tempstr);
Destroy(Token);

return(Apps);
}



char *AppFormatPath(char *Path, TAction *Act)
{
char *Tempstr=NULL;

//first generate wine prefix
Tempstr=CopyStr(Tempstr, GetVar(Act->Vars, "sommelier_root_template"));
Path=SubstituteVarsInString(Path, Tempstr, Act->Vars, 0);
Path=SlashTerminateDirectoryPath(Path);
SetVar(Act->Vars, "sommelier_root",Path);

Tempstr=CopyStr(Tempstr, GetVar(Act->Vars, "prefix_template"));
Path=SubstituteVarsInString(Path, Tempstr, Act->Vars, 0);
Path=SlashTerminateDirectoryPath(Path);
SetVar(Act->Vars, "prefix",Path);

//for dos and golang/go path==prefix
//for wine path is more complex
if (PlatformType(Act->Platform)==PLATFORM_WINDOWS)
{
	Path=CatStr(Path,"drive_c/");
	SetVar(Act->Vars, "drive_c",Path);

  if (StrValid(Act->InstallPath)) Path=SubstituteVarsInString(Path, Act->InstallPath, Act->Vars, 0);
  else Path=SubstituteVarsInString(Path, "$(drive_c)/Program Files/$(name)", Act->Vars, 0);

//  Tempstr=SubstituteVarsInString(Tempstr, "/Program Files/$(name)", Act->Vars, 0);
//  SetVar(Act->Vars, "exec-dir", Tempstr);
}
else 
{
	SetVar(Act->Vars, "drive_c",Path);
  SetVar(Act->Vars, "exec-dir", Path);
}

Path=SlashTerminateDirectoryPath(Path);
SetVar(Act->Vars, "install-dir",Path);

DestroyString(Tempstr);
return(Path);
}



int AppFindConfig(TAction *App, const char *Platforms)
{
TAction *AppConfig;
ListNode *Curr;
int result=FALSE;


Curr=ListGetNext(Apps);
while (Curr) 
{
	if (StrValid(Curr->Tag) && (strcmp(App->Name, Curr->Tag)==0))
	{
	AppConfig=(TAction *) Curr->Item;

	//if no platform requested, or app platform matches requested
	//then we've found the right one

	if ( StrEnd(Platforms) || InList(AppConfig->Platform, Platforms) )
	{
		//flags can already have been set by the command-line, so 
		//we have to | these
		App->Flags |= AppConfig->Flags;
		App->Platform=CopyStr(App->Platform, AppConfig->Platform);
		if (! StrValid(App->URL)) App->URL=CopyStr(App->URL, AppConfig->URL);
		CopyVars(App->Vars, AppConfig->Vars);
		result=TRUE;
		break;
	}
	}

	Curr=ListGetNext(Curr);
}

return(result);
}



int AppLoadConfig(TAction *App)
{
int result=FALSE;
char *Platform=NULL;

if (StrValid(App->Platform)) return(AppFindConfig(App, App->Platform));

Platform=PlatformSelectForURL(Platform, App->URL);
if (StrValid(Platform)) printf("Selected Platforms: %s\n", Platform);

//if no platform specified this will use the first matching app config it finds for any platform
result=AppFindConfig(App, Platform);
if (! result) result=AppFindConfig(App, "");

Destroy(Platform);

return(result);
}


int AppsOutputList(TAction *Template)
{
ListNode *Curr;
int result=FALSE;
TAction *App;
char *p_dl;

Curr=ListGetNext(Apps);
while (Curr)
{
	App=(TAction *) Curr->Item;
	if (AppMatches(Template, App))
	{
	if (StrValid(App->URL)) p_dl="www";
	else p_dl="";
	printf("%- 25s  %- 12s  %- 12s %- 3s   ", App->Name, App->Platform, GetVar(App->Vars, "category"), p_dl);
	printf("%s\n",GetVar(App->Vars, "comment"));
	}

	Curr=ListGetNext(Curr);
}

return(result);
}



