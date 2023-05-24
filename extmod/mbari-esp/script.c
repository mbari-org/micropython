/*
 * script.c
 *
 *  Created on: Sep 2, 2022
 *      Author: sjensen
 */

#include "script.h"

struct
{
	char name[SCRIPT_NAME_LENGTH];
	int index;
	char script[SCRIPT_STRUCT_SIZE-SCRIPT_NAME_LENGTH-sizeof(int)];
}typedef scriptliststruct;

scriptliststruct scriptlist[MAX_SCRIPTS];

int scriptStatusVariable=0;
int scriptRunningIndexVariable=-1;

char testscript[] = "cylinder move 100000\rfinger up\rdelay 5000\rsolenoid on 1\rfinger down\rdelay 5000\rfinger brake\rsolenoid off 1";


int scriptHelp(char *optionline);

int scriptStatus(void)
{
	return scriptStatusVariable;
}

int scriptRunningIndex(void)
{
	return scriptRunningIndexVariable;
}

int scriptFindName(char *nameToFind)
{
	int index;

	for(index=0;index<MAX_SCRIPTS;index++)
		{
			if(strcmp(nameToFind,scriptlist[index].name)==0)
			{
				return index;
			}
		}
	return-1;
}

int scriptIndexToRun(int index)
{
	if((index<0)||(index>=MAX_SCRIPTS)) return -1;
	scriptlist[index].index=0;
	scriptRunningIndexVariable=index;
	return 1;
}

int scriptStop(void)
{
	scriptRunningIndexVariable=-1;
	return 1;
}

int getscriptline(char *dest, int indexScriptRunning,  int length)
{
	char *src;
	int len;

	len=0;
	src=&(scriptlist[indexScriptRunning].script[scriptlist[indexScriptRunning].index]);
	while (len<(length-1))
	{
		if(src[len]=='\r')
		{
			dest[len]=0;
			len++;
			scriptlist[indexScriptRunning].index+=len;
			return len;
		}
		if(src[len]==0)
		{
			dest[len]=0;
			scriptlist[indexScriptRunning].index+=len;
			scriptRunningIndexVariable=-1;
			return len;
		}
		dest[len]=src[len];
		len++;
	}
	scriptlist[indexScriptRunning].index+=len;
	return len;
}


int scriptclearall(char *optionline)
{
	int index;

	for(index=0;index<MAX_SCRIPTS;index++)
	{
//		if(scriptlist[index]!=0) free(scriptlist[index]);
//		scriptlist[index]=NULL;
		scriptlist[index].name[0]=0;
	}
	return 1;
}

int scriptlistnames(char *optionline)
{
	int index;

	for(index=0;index<MAX_SCRIPTS;index++)
	{
		if(scriptlist[index].name[0]!=0) printf("\n\r%d {%s}\n\r",index,scriptlist[index].name);
	}
	return 1;
}

int scriptclearname(char *optionline)
{
	int index;
	int result;
	char scriptToClear[SCRIPT_NAME_LENGTH];

	result=sscanf(optionline,"%*s %*s %s",scriptToClear);

	if (result<1)
	{
		printf("Name required\n\r");
		fflush(stdout);
		return -1;
	}

	for(index=0;index<MAX_SCRIPTS;index++)
	{
		if(strcmp(scriptToClear,scriptlist[index].name)==0)
		{
			scriptlist[index].name[0]=0;
			scriptlist[index].script[0]=0;
			return 1;
		}
	}
	printf("Script Name not found {%s}\n\r",scriptToClear);

	return -2;
}

int scriptcommandlist(char *optionline)
{
	int index;
	int listindex;
	int result;
	char scriptToList[SCRIPT_NAME_LENGTH];

	result=sscanf(optionline,"%*s %*s %s",scriptToList);

	if(result<1)
	{
		printf("script NAME required");
		fflush(stdout);
		return -1;
	}

	for(index=0;index<MAX_SCRIPTS;index++)
	{
		if(strcmp(scriptToList,scriptlist[index].name)==0)
		{
			listindex=0;
			while(scriptlist[index].script[listindex]!=0)
			{
				printf("%c",scriptlist[index].script[listindex]);
				if(scriptlist[index].script[listindex]=='\r') printf("\n");
				fflush(stdout);
				listindex++;
			}

			return 1;
		}
	}
	printf("Script Name not found {%s}\n\r",scriptToList);

	return -1;
}

int scriptrun(char *optionline)
{
	int index;
	int result;
	char scriptToRun[SCRIPT_NAME_LENGTH];

	result=sscanf(optionline,"%*s %*s %s",scriptToRun);

	if (result<1)
	{
		printf("Name required\n\r");
		fflush(stdout);
		return -1;
	}

	for(index=0;index<MAX_SCRIPTS;index++)
	{
		if(strcmp(scriptToRun,scriptlist[index].name)==0)
		{
			scriptlist[index].index=0;
			scriptRunningIndexVariable=index;
			return 1;
		}
	}
	printf("Script Name not found {%s}\n\r",scriptToRun);
	return -2;
}

int scriptnew(char *optionline)
{
	int result;
	int index;
	int newindex;

	char newScriptName[SCRIPT_NAME_LENGTH];
	char scriptCommandLine[CMDLINE_LENGTH];

	result=sscanf(optionline,"%*s %*s %32s",newScriptName);
	if (result<1)
	{
		printf("Name required\n\r");
		fflush(stdout);
		return -1;
	}

	for(newindex=0;newindex<MAX_SCRIPTS;newindex++)
	{
		if(strcmp(newScriptName,scriptlist[newindex].name)==0)
		{
			printf("Name already exists");
			return -2;
		}
	}

	for(newindex=0;newindex<MAX_SCRIPTS;newindex++)
	{
		if(scriptlist[newindex].name[0]==0)break;
		}

	if(newindex==MAX_SCRIPTS)
	{
		printf("No room left\n\r");
		fflush(stdout);
		return -3;
	}


	/*scriptlist[newindex] = malloc(SCRIPT_SIZE);

	if(scriptlist[newindex]==NULL)
	{
		printf("Failed to malloc\n\r");
		return -1;
	}
	*/
	//Found an open spot
	strcpy(scriptlist[newindex].name,newScriptName);
//	printf("From new memory, name is: %s\n\r",scriptlist[newindex].name);
	scriptlist[newindex].index=0;
	do
	{
		result=getline(scriptCommandLine,CMDLINE_LENGTH);
		if(result>=0)
		{
			if(memcmp("end",scriptCommandLine,sizeof("end"))==0) break;
			for(index=0;((scriptlist[newindex].index<(SCRIPT_SIZE-1))&&(scriptCommandLine[index]!=0));scriptlist[newindex].index++,index++)
			{
				scriptlist[newindex].script[scriptlist[newindex].index]=scriptCommandLine[index];
			}
			scriptlist[newindex].script[scriptlist[newindex].index++]='\r';
		}
	} while (scriptlist[newindex].index<SCRIPT_SIZE);

	if (scriptlist[newindex].index>=SCRIPT_SIZE)
	{
		printf("Script to long");
		fflush(stdout);
		scriptlist[newindex].name[0]=0;
		scriptlist[newindex].index=0;
		return -1;
	}

	scriptlist[newindex].script[scriptlist[newindex].index]=0;

/*
	strcpy(scriptlist[newindex].script,testscript);
	printf("From new memory, script is: \n\r%s\n\r",scriptlist[newindex].script);
	fflush(stdout);
	index=0;
	while(scriptlist[newindex].script[scriptlist[newindex].index]!=0)
	{
		result=getscriptline(newScriptName,newindex,32);
		index+=result;
		printf("\n\r%d %d {%s}\n\r",scriptlist[newindex].index,result,newScriptName);
		fflush(stdout);
	}
	*/
	return 1;
}

int scriptOptionNotFound(char *cmdline)
{
    printf("Unrecognized script option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef scriptstruct;

scriptstruct scriptoptions[]=
{
    {"clearall",sizeof("clearall")-1,scriptclearall,"script clearall"},
    {"clear",sizeof("clear")-1,scriptclearname,"script clear name"},
    {"listnames",sizeof("listnames")-1,scriptlistnames,"script listnames"},
    {"list",sizeof("list")-1,scriptcommandlist,"script list name"},
    {"run",sizeof("run")-1,scriptrun,"script run ScriptNameToRun"},
    {"new",sizeof("new")-1,scriptnew,"script new newScriptName"},
    {"help",sizeof("help")-1,scriptHelp,""},
    {"",0,scriptOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(scriptoptions)/sizeof(scriptstruct))-1)

int scriptHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",scriptoptions[optionindex].option,scriptoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int scriptcommand(char *optionline)
{
	#define OPTIONLINE_LENGTH 256

	char opt[OPTIONLINE_LENGTH];
	int optlen;
	int result;
	int optindex;

		result=sscanf(optionline,"%*s %s",opt);
		if (result==1)
		{
			optlen=strlen(opt);
			for(optindex=0;optindex<OPTIONNUM;optindex++)
			{
				if(optlen==scriptoptions[optindex].optionlen)
				{
					if(memcmp(opt,scriptoptions[optindex].option,optlen)==0)
						{
						scriptoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) scriptoptions[optindex].optionfunc(optionline);
		}
//		else scriptPosition("script position");
	return 0;
}

#undef OPTIONNUM
