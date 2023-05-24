/*
 * command.c
 *
 *  Created on: Aug 9, 2022
 *      Author: sjensen
 */


#include "command.h"

//int scriptrunning=0;
int scriptindex=0;

int help(char *cmdline);

int printhello(char *cmdline)
{
    printf("Hello\r\n");
    fflush(stdout);
    return 1;
}

int delaycommand (char *cmdline)
{
	int msdelay;
	int result;

	result=sscanf(cmdline,"%*s %d",&msdelay);
	if (result<1)
	{
		printf("milliSeconds required\n\r");
		fflush(stdout);
		return -1;
	}
	if (result==1 )
	{
		if(msdelay<1)
		{
			printf("invalid time\n\r");
			fflush(stdout);
			return -2;
		}

		uint32_t tickstart = HAL_GetTick();
		uint32_t wait = msdelay;

		/* Add a period to guaranty minimum wait */
		if (wait < HAL_MAX_DELAY)
		{
			wait += (uint32_t)uwTickFreq;
		}

		while ((HAL_GetTick() - tickstart) < wait)
		{
			if(getBreak()) return -1;
		}
	}

	return 1;
}

int CmndNotFound(char *cmdline)
{
    printf("Unrecognized Command or Script\r\n");
    fflush(stdout);
    return 1;
}
struct
{
    char *cmd;
    int cmdlen;
    int (*cmdfunc)(char *cmdline);
}typedef commandstruct;

commandstruct commands[]=
{
    {"solenoid",sizeof("solenoid")-1,solenoidcommand},
    {"cylinder",sizeof("cylinder")-1,cylindercommand},
    {"slide",sizeof("slide")-1,slidecommand},
    {"clamp",sizeof("clamp")-1,clampcommand},
    {"flush",sizeof("flush")-1,flushcommand},
    {"rna",sizeof("rna")-1,rnacommand},
    {"finger",sizeof("finger")-1,fingercommand},
    {"vacuum",sizeof("vacuum")-1,vacuumcommand},
    {"nfc",sizeof("nfc")-1,nfccommand},
	{"script",sizeof("script")-1,scriptcommand},
	{"delay",sizeof("delay")-1,delaycommand},
    {"hello",sizeof("hello")-1,printhello},
    {"help",sizeof("help")-1,help},
    {"",0,CmndNotFound}
};


#define NUMCMDS ((sizeof(commands)/sizeof(commandstruct))-1)


int help(char *cmdline)
{
	int cmdindex;
	for(cmdindex=0;cmdindex<NUMCMDS;cmdindex++)
	{
		printf("%s\r\n",commands[cmdindex].cmd);
		fflush(stdout);
	}
    return 1;
}

void command(void)
{
//	#define CMDLINE_LENGTH 256

	char commandLine[CMDLINE_LENGTH];
	char cmd[CMDLINE_LENGTH];
	int cmdlen;
	int result;
	int cmdindex;
	int scriptindex;



	scriptindex=scriptRunningIndex();

	if (scriptindex<0)
	{
	result=getline(commandLine,CMDLINE_LENGTH);
	}
	else
	{
		result=getscriptline(commandLine,scriptindex,CMDLINE_LENGTH);
		if (result>0) printf("%s",commandLine);
		fflush(stdout);

		if(result<=0)
		{
			result=getline(commandLine,CMDLINE_LENGTH);
		}
	}

	if(result>=0)
	{
		result=sscanf(commandLine,"%s",cmd);
		if (result==1)
		{
			cmdlen=strlen(cmd);
			for(cmdindex=0;cmdindex<NUMCMDS;cmdindex++)
			{
				if(cmdlen==commands[cmdindex].cmdlen)
				{
					if(memcmp(cmd,commands[cmdindex].cmd,cmdlen)==0)
						{
							commands[cmdindex].cmdfunc(commandLine);
							break;
						}
				}
			}
			result=scriptFindName(commandLine);
			if(result>=0) scriptIndexToRun(result);  //found script to run
			else if(cmdindex==NUMCMDS) commands[cmdindex].cmdfunc(commandLine); //did not recognize
		}
		printf("\r\n>");
		fflush(stdout);
	}
	if(getBreak())
	{
		scriptStop();
		clrBreak();
	}
	return;
}


//		result=sscanf(commandLine,"%[!--/-~].%[!--/-~] %[ -~]",cmd,ext,args);
//		result=sscanf(commandLine,"%[!--/-~].%[!-~] %[ -~]",cmd,ext,args);
//		result=sscanf(commandLine,"%[^.].%[!-~] %[ -~]",cmd,ext,args);  //  this should return 3 from a command line like this:   group.operation arg,arg...
//		result=sscanf(commandLine,"%[^.].%[A-za-z0-9] %s",cmd,ext,args);
//		result=sscanf(commandLine,"%[A-Za-z0-9]%1[.]%[A-Za-z0-9] %s",cmd,delimiter,ext,args);
//		printf("Items: %d\r\n",result);
//		printf("[%s]\r\n",cmd);
//		printf("[%c]\r\n",*delimiter);
//		printf("[%s]\r\n",ext);
//		printf("[%s]\r\n",args);
//		printf("\r\n[%s]\r\n",commandLine);
//		fflush(stdout);
