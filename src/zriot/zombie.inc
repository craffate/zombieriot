/**
 * ====================
 *     Zombie Riot
 *   File: zombie.inc
 *   Author: Greyscale
 * ====================
 */

char skyname[32];
char hostname[256];

int gZombieID[MAXPLAYERS+1];

HookCommands()
{
    RegConsoleCmd("nightvision", Command_NightVision);
}

public Action Command_NightVision(client, argc)
{
    bool allow_disable = GetConVarBool(gCvars.CVAR_ZVISION_ALLOW_DISABLE);
    bool enabled = GetConVarBool(gCvars.CVAR_ENABLE);
    if (!allow_disable || !enabled)
    {
        return;
    }
    
    if (!IsPlayerZombie(client))
    {
        return;
    }
    
    bZVision[client] = !bZVision[client];
    
    if (bZVision[client])
    {
        StartZVision(client);
    }
    else
    {
        StopZVision(client);
        ClientCommand(client, "r_screenoverlay \"\"");
    }
}

FindMapSky()
{
    GetConVarString(FindConVar("sv_skyname"), skyname, sizeof(skyname));
}

FindHostname()
{
    GetConVarString(FindConVar("hostname"), hostname, sizeof(hostname));
}

ChangeLightStyle()
{
    bool dark = GetConVarBool(gCvars.CVAR_DARK);
    if (dark)
    {
        char darkness[2];
        char sky[32];
        
        GetConVarString(gCvars.CVAR_DARK_LEVEL, darkness, sizeof(darkness));
        GetConVarString(gCvars.CVAR_DARK_SKY, sky, sizeof(sky));
        
        SetLightStyle(0, darkness);
        SetConVarString(FindConVar("sv_skyname"), sky, true);
    }
    else
    {
        SetLightStyle(0, "n");
        SetConVarString(FindConVar("sv_skyname"), skyname, true);
    }
}

UpdateHostname()
{
    char hostname_prefixed[256];
    
    SetGlobalTransTarget(LANG_SERVER);
    Format(hostname_prefixed, sizeof(hostname_prefixed), "%t %s", "Hostname prefix", gDay + 1, dCount, hostname);
    
    SetHostname(hostname_prefixed);
}

Zombify(client, zombieid)
{
    gZombieID[client] = zombieid;
    
    RemoveAllPlayersWeapons(client);
    int knife = GivePlayerItem(client, "weapon_knife");
    
    if (knife != -1)
    {
        SetEntityRenderMode(knife, RENDER_TRANSALPHA);
        SetEntityRenderColor(knife, 255, 255, 255, 0);
    }
    
    ApplyZombieModel(client, zombieid);
    ApplyZombieHealth(client, zombieid);
    ApplyZombieSpeed(client, zombieid);
    ApplyZombieGravity(client, zombieid);
    ApplyZombieFOV(client, zombieid);
    
    if (bZVision[client])
    {
        StartZVision(client);
    }
}

ZombiesWin()
{
    bool regression = GetConVarBool(gCvars.CVAR_REGRESSION);
    
    if (gDay > 0 && regression)
    {
        gDay--;
    }
    
    bool fade = GetConVarBool(gCvars.CVAR_ROUNDFADE);
    if (fade)
        Fade(0, 2000, 2000, 2, 255, 0, 0, 255);
    
    bool overlays = GetConVarBool(gCvars.CVAR_OVERLAYS);
    if (overlays)
        ShowOverlays(5.0, gZombieTeam);
    
    FreezeZombies();
}

HumansWin()
{
    gDay++;
        
    if (gDay + 1 > dCount)
    {
        gDay = 0;
    
        ZRiot_PrintToChat(0, "Game won");
        
        GotoNextMap();
    }
    
    bool fade = GetConVarBool(gCvars.CVAR_ROUNDFADE);
    if (fade)
        Fade(0, 2000, 2000, 2, 0, 0, 255, 255);
    
    bool overlays = GetConVarBool(gCvars.CVAR_OVERLAYS);
    if (overlays)
        ShowOverlays(5.0, gHumanTeam);
    
    FreezeZombies();
}

RoundDraw()
{
    Fade(0, 1000, 2000, 2, 0, 0, 0, 255);
}

FreezeZombies()
{
    int maxplayers = MaxClients;
    for (int x = 1; x <= maxplayers; x++)
    {
        if (!IsClientInGame(x) || !IsPlayerAlive(x) || !IsPlayerZombie(x))
        {
            continue;
        }
        
        SetEntProp(x, Prop_Data, "m_takedamage", 0, 1);
        SetEntProp(x, Prop_Data, "m_fFlags", FL_ATCONTROLS);
    }
}

public Action UnfreezeZombies(Handle timer)
{
    int maxplayers = MaxClients;
    for (int x = 1; x <= maxplayers; x++)
    {
        if (!IsClientInGame(x) || !IsPlayerAlive(x) || !IsPlayerZombie(x))
        {
            continue;
        }
        
        SetEntProp(x, Prop_Data, "m_takedamage", 2, 1);
        SetEntProp(x, Prop_Data, "m_fFlags", FL_ONGROUND);
    }
    
    tFreeze = INVALID_HANDLE;
}

RemoveObjectives()
{
    char classname[64];
    
    int maxentities = GetMaxEntities();
    for (int x = 0; x <= maxentities; x++)
    {
        if(!IsValidEdict(x))
        {
            continue;
        }
        
        GetEdictClassname(x, classname, sizeof(classname));
        if( StrEqual(classname, "func_bomb_target") ||
            StrEqual(classname, "func_hostage_rescue") ||
            StrEqual(classname, "c4") ||
            StrEqual(classname, "hostage_entity"))
            {
                RemoveEdict(x);
            }
    }
}

public Action JoinZombie(Handle timer, any index)
{
    if (!IsClientInGame(index))
    {
        return;
    }
    
    ZRiot_Zombie(index);
}

public Action ZombieRespawn(Handle timer, any index)
{
    if (!IsClientInGame(index))
    {
        return;
    }
    
    CS_RespawnPlayer(index);
}

StartRespawnTimer(client, bool firstspawn)
{
    int respawn;
    if (firstspawn)
    {
        respawn = GetConVarInt(gCvars.CVAR_FIRST_RESPAWN);
    }
    else
    {
        respawn = GetConVarInt(gCvars.CVAR_RESPAWN);
    }
    
    if (!respawn)
    {
        return;
    }
    
    if (tRespawn[client] != INVALID_HANDLE)
    {
        CloseHandle(tRespawn[client]);
    }
    
    ZRiot_PrintCenterText(client, "Respawn time", respawn);
    
    gRespawnTime[client] = respawn;
    tRespawn[client] = CreateTimer(1.0, HumanRespawn, client, TIMER_REPEAT);
}

public Action HumanRespawn(Handle timer, any index)
{
    if (!IsClientInGame(index))
    {
        tRespawn[index] = INVALID_HANDLE;
        
        return Plugin_Stop;
    }
    
    int team = GetClientTeam(index);
    if (team == CS_TEAM_T || team == CS_TEAM_CT)
    {
        gRespawnTime[index]--;
    }
    
    int timeremaining = (gRespawnTime[index] < 0) ? 0 : gRespawnTime[index];
    
    ZRiot_PrintCenterText(index, "Respawn time", timeremaining);
    
    if (gRespawnTime[index] > 0)
    {
        return Plugin_Continue;
    }
    
    tRespawn[index] = INVALID_HANDLE;
    
    CS_RespawnPlayer(index);
    
    return Plugin_Stop;
}

StartZVision(client)
{
    if (tZVision[client] != INVALID_HANDLE)
    {
        CloseHandle(tZVision[client]);
        tZVision[client] = INVALID_HANDLE;
    }
    
    bool zvision = ZVision(client);
    if (zvision)
    {
        float redisplay = GetConVarFloat(gCvars.CVAR_ZVISION_REDISPLAY);
        tZVision[client] = CreateTimer(redisplay, ZVisionTimer, client, TIMER_REPEAT);
    }
}

StopZVision(client)
{
    if (tZVision[client] != INVALID_HANDLE)
    {
        CloseHandle(tZVision[client]);
        tZVision[client] = INVALID_HANDLE;
    }
}

bool ZVision(client)
{
    if (IsFakeClient(client))
    {
        return false;
    }
    
    char zvision[256];
    GetZombieZVision(gZombieID[client], zvision, sizeof(zvision));
    
    if (zvision[0])
    {
        DisplayClientOverlay(client, zvision);
        
        return true;
    }
    
    return false;
}

public Action ZVisionTimer(Handle timer, any index)
{
    if (!IsClientInGame(index) || !IsPlayerZombie(index))
    {
        tZVision[index] = INVALID_HANDLE;
        
        return Plugin_Stop;
    }
    
    ZVision(index);
        
    return Plugin_Continue;
}

public Action RemoveRagdoll(Handle timer, any ragdoll)
{
    if (ragdoll == -1 || !IsValidEdict(ragdoll))
    {
        return;
    }
    
    char classname[64];
    GetEdictClassname(ragdoll, classname, sizeof(classname));
    
    if (!StrEqual(classname, "cs_ragdoll"))
    {
        return;
    }
        
    RemoveEdict(ragdoll);
}

Fade(client, speed, hold, type, red, green, blue, alpha)
{
    Handle hFade = INVALID_HANDLE;
    
    if (client)
    {
	   hFade = StartMessageOne("Fade", client);
	}
	else
	{
	   hFade = StartMessageAll("Fade");
	}
	
    if (hFade != INVALID_HANDLE)
	{
        // type 1 = IN
        // type 2 = OUT
        
    	BfWriteShort(hFade, speed);
    	BfWriteShort(hFade, hold);
    	BfWriteShort(hFade, type);
    	BfWriteByte(hFade, red);
    	BfWriteByte(hFade, green);
    	BfWriteByte(hFade, blue);	
    	BfWriteByte(hFade, alpha);
    	EndMessage();
    }
}

InitClientDeathCount(client)
{
    if (IsFakeClient(client))
        return;
    
    char steamid[64];
    GetClientAuthId(client, AuthId_Steam2, steamid, sizeof(steamid));
    
    StrCat(steamid, sizeof(steamid), "_iDeaths");
    
    SetTrieValue(trieDeaths, steamid, 0, false);
}

ChangeClientDeathCount(client, value)
{
    if (IsFakeClient(client))
        return;
    
    char steamid[64];
    GetClientAuthId(client, AuthId_Steam2, steamid, sizeof(steamid));
    
    StrCat(steamid, sizeof(steamid), "_iDeaths");
    
    int newvalue = value != 0 ? GetClientDeathCount(client) + value : 0;
    SetTrieValue(trieDeaths, steamid, newvalue, true);
}

GetClientDeathCount(client)
{
    if (IsFakeClient(client))
        return 0;
    
    char steamid[64];
    GetClientAuthId(client, AuthId_Steam2, steamid, sizeof(steamid));
    
    StrCat(steamid, sizeof(steamid), "_iDeaths");
    
    int value;
    GetTrieValue(trieDeaths, steamid, value);
    
    return value;
}

SetHostname(const char[] name)
{
    bool update_hostname = GetConVarBool(gCvars.CVAR_HOSTNAME_UPDATE);
    if (!update_hostname)
    {
        return;
    }
    
    ServerCommand("hostname \"%s\"", name);
}

ZRiot_Zombie(client)
{
    if (bZombie[client])
    {
        return;
    }
    
    bZombie[client] = true;
    AssignTeamClient(client, true);
    
    Call_StartForward(hOnClientZombie);
    Call_PushCell(client);
    Call_Finish();
}

ZRiot_Human(client)
{
    if (!bZombie[client])
    {
        return;
    }
    
    bZombie[client] = false;
    AssignTeamClient(client, true);
    
    Call_StartForward(hOnClientHuman);
    Call_PushCell(client);
    Call_Finish();
}

bool IsPlayerZombie(client)
{
    return bZombie[client];
}

bool IsPlayerHuman(client)
{
    return !bZombie[client];
}