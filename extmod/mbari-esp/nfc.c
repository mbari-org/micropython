/*
 * nfc.c
 *
 *  Created on: Sep 16, 2022
 *      Author: sjensen
 */

#include "nfc.h"

int nfcHelp(char *optionline);

int writebuf(unsigned char *src, int n)
{
	if(u4txroom()<n) return -1;
	while (n>0)
	{
		u4txaddbyte(*src);
		src++;
		n--;
	}
	return n;
}

int buildpacket(unsigned char *buffer,unsigned char *command,int cmdlen)
{
	unsigned char checksum=0;
	int i;
	buffer[0]=0;
	buffer[1]=0;
	buffer[2]=0xff;
	buffer[3]=cmdlen;
	buffer[4]=(~cmdlen+1);
	for(i=0,checksum=0;i<cmdlen;i++)
	{
		buffer[i+5]=command[i];
            checksum+=buffer[i+5];
	}
	checksum=(~checksum)+1;
      buffer[i+5]=checksum;
	buffer[i+6]=0;
	return(i+7);

}//end of buildpacket

int sendACK(void)
{
	unsigned char tACK[]={0,0,0xff,0,0xff,0};
	writebuf(tACK,sizeof(tACK));
	return 0;
}//end of sendACK

int findPreamble(int timesToTry)
{
	int trys,state=0;
	int result;
	unsigned int fail,start,current;
	unsigned char buffer[3];

	start= HAL_GetTick();
	fail = start+timesToTry;

	while(1)
	{
		switch(state)
			{
		case 0:
			if(nin4rxbuf()>0)
			{
				result=u4rxgetbyte(&buffer[0]);
				if(buffer[0]==0) state=2;   //possible first 0 found
				else state=1;		//not a zero so try again
			}
			break;
		case 1: //looking for first 0
			if(nin4rxbuf()>0)
			{
				result=u4rxgetbyte(&buffer[0]);
				if(buffer[0]==0) state=2;   //possible first 0 found
			}
			break;
		case 2: //found a zero now looking for second 0
			if(nin4rxbuf()>0)
			{
				result=u4rxgetbyte(&buffer[1]);
				if(buffer[1]==0) state=3;   //possible first 0 found
				else state=1;		//not a zero so try again
			}
			break;
		case 3: //found second zero now looking for 0xff
			if(nin4rxbuf()>0)
			{
				result=u4rxgetbyte(&buffer[2]);
				if(buffer[2]==0xff)
					return 1;   //possible first 0 found
				else state=1;		//not a zero so try again
			}
			break;
		default: return EBADSTATE;
		}//end of switch
		current=HAL_GetTick();
		if (current>fail)
		{
			sendACK();
			return ETIMEOUT0;
		}
	}//end of while

return EBAD;
}//end of findPreamble



int readChars(int timeToTry,int numToRead, unsigned char *buffer)
{
//	int trys,
	int chars;
	int start,current,fail;

	start= HAL_GetTick();
	fail = start+timeToTry;

	current=HAL_GetTick();
	while (current<fail)
	{
		if(nin4rxbuf()>=numToRead)
		{
			for(chars=0;chars<numToRead;chars++)
			{
					u4rxgetbyte(&buffer[chars]);
			}
			return chars;
		}
		current=HAL_GetTick();
	}
	return ERCTIMEOUT;
}//end of readChars



int findACK(void)
{
	unsigned char buffer[3];
	if (findPreamble(3)<0) return EACKPFAIL;

	if (readChars(3,3,buffer)<0) return EACKNOREAD;
	if((buffer[0]==0)&&(buffer[1]==0xff)&&(buffer[2]==0))
		return 1;
	return ENOACK;
}//end of findACK

int readPacket(int maxbuf, unsigned char *buffer)
{
	unsigned char checksum;
      int i,nchars, length;

	if (findPreamble(100)<0) return EACKRPFAIL;
	if (readChars(3,2,buffer)<0) return EACKNOREADP;
	if((unsigned char)(buffer[0]+buffer[1])!=0) return EBADLCHKSUM;
	length=buffer[0]+2;
	if(length>100) return ETOLONG;
	nchars=readChars(10,length,buffer);
	if (nchars<0) return ERPDFAIL;
	if (nchars!=length) return ERPNOTENOUGH;
	if(buffer[length-1]!=0) return EBADTRAIL;
	for(i=0,checksum=0;i<length;checksum+=buffer[i++]);
	if ((checksum&0xff)!=0) return EBADDCHKSUM;
	return (length-1);
}//end of readPacket


int sendSAMConfiguration()
{
	unsigned char SAMConfiguration[]= {0xd4,0x14,0x01};
	unsigned char buffer[100];
	int i;

	i=buildpacket(buffer,SAMConfiguration,sizeof(SAMConfiguration));
	if(writebuf(buffer,i)<0)
		return ESSCFAILEDTOSEND;

	if(findACK()<0)
		return ESSCFAILEDACK;

	if (readPacket(100,buffer)<0)
		return ESSCFAILEDRESPONSE;
	if ((buffer[0]!=0xd5)||(buffer[1]!=0x15))
		return ESSCBADRESPONSE;

	sendACK();

	return 0;
}//end of sendSAMConfiguration


int nfcWakeUp(void)
{
	int result;
	if(u4txaddbyte(0x55)<0) return -1;
//	if(u4txaddbyte(0x55)<0) return -2;
	HAL_Delay(2);
	result=sendSAMConfiguration();
	return result;
}

int sendInListPassiveTarget(unsigned char *buffer, int bufsize)
{
	unsigned char InListPassiveTarget[]={0xd4,0x4a,2,0};
	int i;

 	i=buildpacket(buffer, InListPassiveTarget,sizeof(InListPassiveTarget));
	if(writebuf(buffer,i)<0) return ESILPTFAILEDTOSEND;

	if(findACK()<0) return ESILPTFAILEDACK;

	i=readPacket(bufsize,buffer);
 	if (i==EACKRPFAIL) return ENOTARGETS;
	if (i<0) return ESILPTFAILEDRESPONSE;
	if ((buffer[0]!=0xd5)||(buffer[1]!=0x4b)) return ESILPTBADRESPONSE;

	sendACK();

	return 0;
}//end of sendInListpassiveTarget


int getTargetID(unsigned int *ID)
{
	unsigned char buffer[100];
	int ecode;
      *ID=0;
	ecode = sendInListPassiveTarget(buffer,100);
	if (ecode == ENOTARGETS) return ENOTARGETS;
	if (ecode < 0) return EFAILEDGETTING;
	*ID=*(unsigned int*)(&buffer[8]);
	return 0;
}//end of getTargetID


int nfcRead(char *cmdline)
{
	unsigned char buffer[100];
	int ecode;
    unsigned int ID=0;
	ecode = sendInListPassiveTarget(buffer,100);
	if (ecode == ENOTARGETS) 
		{
			printf("No Puck Found\n\r");
			fflush(stdout);
			return ENOTARGETS;
		}
	if (ecode < 0)		{
			printf("No Puck Found\n\r");
			fflush(stdout);
			return EFAILEDGETTING;
		}
	ID=*(unsigned int*)(&buffer[8]);
	printf("Puck ID: %x\n\r",ID);
	return 0;
}

int nfcRelease(char *cmdline)
{
	unsigned char buffer[100];
	int ecode;
	int i;
	unsigned char InRelease[] = {0xd4,0x52,0x0}; //0x0 means release all

	i=buildpacket(buffer, InRelease,sizeof(InRelease));
	if(writebuf(buffer,i)<0) return ESILPTFAILEDTOSEND;

	if(findACK()<0) return ESILPTFAILEDACK;
printf("Foudn ACK\n\r");
fflush(stdout);
	i=readPacket(100,buffer);
	if (i<0) return ESILPTFAILEDRESPONSE;
	printf("Foudn packet\n\r");
	fflush(stdout);
	if ((buffer[0]!=0xd5)||(buffer[1]!=0x53)) return ESILPTBADRESPONSE;
	printf("response is: %x\n\r",buffer[2]);
	return 0;

}

int nfcOptionNotFound(char *cmdline)
{
    printf("Unrecognized nfc option\r\n");
    fflush(stdout);
    return 1;
}


struct
{
    char *option;
    int optionlen;
    int (*optionfunc)(char *optionline);
    char *list;
}typedef nfcstruct;

nfcstruct nfcoptions[]=
{
    {"read",sizeof("read")-1,nfcRead,""},
    {"release",sizeof("release")-1,nfcRelease,""},
    {"help",sizeof("help")-1,nfcHelp,""},
    {"",0,nfcOptionNotFound,""}
};

#define OPTIONNUM ((sizeof(nfcoptions)/sizeof(nfcstruct))-1)

int nfcHelp(char *optionline)
{
	int optionindex;
	for(optionindex=0;optionindex<OPTIONNUM;optionindex++)
	{
		printf("%s %s\r\n",nfcoptions[optionindex].option,nfcoptions[optionindex].list);
		fflush(stdout);
	}
    return 1;
}

int nfccommand(char *optionline)
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
				if(optlen==nfcoptions[optindex].optionlen)
				{
					if(memcmp(opt,nfcoptions[optindex].option,optlen)==0)
						{
						nfcoptions[optindex].optionfunc(optionline);
							break;
						}
				}
			}
			if(optindex==OPTIONNUM) nfcoptions[optindex].optionfunc(optionline);
		}
//		else nfcPosition("nfc position");
	return 0;
}

#undef OPTIONNUM

