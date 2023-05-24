# MBARI ESI extensions to Micropython

```
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
```
```
clampstruct clampoptions[]=
{
    {"position",sizeof("position")-1,clampPosition,""},
    {"move",sizeof("move")-1,clampMove,"Distance Pace"},
    {"state",sizeof("state")-1,clampToState,"State Distance"},
    {"set",sizeof("set")-1,clampSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,clampGet,""},
    {"help",sizeof("help")-1,clampHelp,""},
    {"",0,clampOptionNotFound,""}
};
```

```
cylinderstruct cylinderoptions[]=
{
    {"position",sizeof("position")-1,cylinderPosition,""},
    {"move",sizeof("move")-1,cylinderMove,"Distance Pace"},
    {"tube",sizeof("tube")-1,cylinderTubeCommand,"Tube Pace"},
    {"set",sizeof("set")-1,cylinderSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,cylinderGet,""},
    {"traj",sizeof("traj")-1,cylinderTraj,"Distance Pace"},
    {"help",sizeof("help")-1,cylinderHelp,""},
    {"",0,cylinderOptionNotFound,""}
};
```

fingerstruct fingeroptions[]=
{
    {"up",sizeof("up")-1,fingerUp},
    {"down",sizeof("down")-1,fingerDown},
    {"sleep",sizeof("sleep")-1,fingerSleep},
    {"brake",sizeof("brake")-1,fingerBrake},
    {"position",sizeof("position")-1,fingerPosition},
    {"help",sizeof("help")-1,fingerHelp},
    {"",0,fingeroptionNotFound}
};


nfcstruct nfcoptions[]=
{
    {"read",sizeof("read")-1,nfcRead,""},
    {"release",sizeof("release")-1,nfcRelease,""},
    {"help",sizeof("help")-1,nfcHelp,""},
    {"",0,nfcOptionNotFound,""}
};


rnastruct rnaoptions[]=
{
//    {"position",sizeof("position")-1,rnaPosition,""},
    {"deliver",sizeof("deliver")-1,rnaDeliver,"microL Rate"},
    {"move",sizeof("move")-1,rnaMove,"Distance Pace"},
    {"set",sizeof("set")-1,rnaSet,"Acceleration Drift DriftPace"},
    {"get",sizeof("get")-1,rnaGet,""},
    {"help",sizeof("help")-1,rnaHelp,""},
    {"",0,rnaOptionNotFound,""}
};


char testscript[] = "cylinder move 100000\rfinger up\rdelay 5000\rsolenoid on 1\rfinger down\rdelay 5000\rfinger brake\rsolenoid off 1";


slidePosNamesdef slideNameToNumber[] =
{
		{"pick",sizeof("pick")-1,1,0x9,pickPos},
		{"filter",sizeof("filter")-1,2,0xe,filterPos},
		{"drop",sizeof("drop")-1,3,0x0,dropPos},
		{"flush",sizeof("flush")-1,4,0xf,flushPos}
};


solenoidstruct solenoidoptions[]=
{
    {"on",sizeof("on")-1,solenoidOn,"valve"},
    {"off",sizeof("off")-1,solenoidOff,"valve"},
    {"position",sizeof("position")-1,solenoidPosition,""},
    {"help",sizeof("help")-1,solenoidHelp,""},
    {"",0,solenoidOptionNotFound,""}
};

vacuumstruct vacuumoptions[]=
{
    {"on",sizeof("on")-1,vacuumOn},
    {"off",sizeof("off")-1,vacuumOff},
    {"help",sizeof("help")-1,vacuumHelp},
    {"",0,vacuumoptionNotFound}
};
