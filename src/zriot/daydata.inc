/**
 * ====================
 *     Zombie Riot
 *   File: daydata.inc
 *   Author: Greyscale
 * ====================
 */

enum struct ZRiot_DayData
{
    char data_display[32];
    char data_zombieoverride[32*MAXZOMBIES];
    char data_storyline[192];
    int data_count;
    int data_healthboost;
    bool data_respawn;
    int data_deaths_before_zombie;
    float data_fademin;
    float data_fademax;
}

#define MAXDAYS 25

Handle kvDays = INVALID_HANDLE;

ZRiot_DayData arrayDays[MAXDAYS];
int dCount;

LoadDayData(bool defaultconfig)
{
    char path[PLATFORM_MAX_PATH];
    Format(path, sizeof(path), "%s/days.txt", gMapConfig);
    
    if (!defaultconfig && !FileExists(path))
    {
        return;
    }
    
    if (kvDays != INVALID_HANDLE)
    {
        CloseHandle(kvDays);
    }
    
    kvDays = CreateKeyValues("days");
    KvSetEscapeSequences(kvDays, true);
    
    if (!FileToKeyValues(kvDays, path))
    {
        SetFailState("\"%s\" failed to load", path);
    }
    
    KvRewind(kvDays);
    if (!KvGotoFirstSubKey(kvDays))
    {
        SetFailState("No day data defined in \"%s\"", path);
    }
    
    char display[32];
    char zombieoverride[32*MAXZOMBIES];
    char storyline[192];
    
    dCount = 0;
    do
    {
        KvGetSectionName(kvDays, display, sizeof(display));
        strcopy(arrayDays[dCount].data_display, 32, display);
        
        KvGetString(kvDays, "zombieoverride", zombieoverride, sizeof(zombieoverride));
        strcopy(arrayDays[dCount].data_zombieoverride, 32*MAXZOMBIES, zombieoverride);
        
        KvGetString(kvDays, "storyline", storyline, sizeof(storyline));
        strcopy(arrayDays[dCount].data_storyline, 192, storyline);
        
        arrayDays[dCount].data_count = KvGetNum(kvDays, "count", 25);
        arrayDays[dCount].data_healthboost = KvGetNum(kvDays, "healthboost");
        arrayDays[dCount].data_respawn = KvGetNum(kvDays, "respawn") ? true : false;
        arrayDays[dCount].data_deaths_before_zombie = KvGetNum(kvDays, "deaths_before_zombie");
        arrayDays[dCount].data_fademin = KvGetFloat(kvDays, "fademin");
        arrayDays[dCount].data_fademax = KvGetFloat(kvDays, "fademax");
        
        dCount++;
    } while (KvGotoNextKey(kvDays));
}

GetDayDisplay(day, char[] display, len)
{
    strcopy(display, len, arrayDays[day].data_display);
}

bool ExplodeZombieOverrides(day)
{
    if (adtZombies != INVALID_HANDLE)
    {
        CloseHandle(adtZombies);
        adtZombies = INVALID_HANDLE;
    }
    
    char zombieoverride[32*MAXZOMBIES];
    GetDayZombieOverride(day, zombieoverride, sizeof(zombieoverride));
    
    if (zombieoverride[0])
    {
        adtZombies = CreateArray();
        
        char sZombies[MAXZOMBIES][64];
        ExplodeString(zombieoverride, ",", sZombies, MAXZOMBIES, 64);
        
        for (int x = 0; x < MAXZOMBIES; x++)
        {
            if (!sZombies[x][0])
                continue;
            
            TrimString(sZombies[x]);
            int zombieid = FindZombieIDByName(sZombies[x]);
            
            if (zombieid == -1)
                continue;
            
            PushArrayCell(adtZombies, zombieid);
        }
        
        return true;
    }
    
    return false;
}

GetDayZombieOverride(day, char[] zombieoverride, len)
{
    strcopy(zombieoverride, len, arrayDays[day].data_zombieoverride);
}

GetDayStoryLine(day, char[] storyline, len)
{
    strcopy(storyline, len, arrayDays[day].data_storyline);
}

GetDayCount(day)
{
    return arrayDays[day].data_count;
}

GetDayHealthBoost(day)
{
    return arrayDays[day].data_healthboost;
}

bool GetDayRespawn(day)
{
    return arrayDays[day].data_respawn;
}

GetDayDeathsBeforeZombie(day)
{
    return arrayDays[day].data_deaths_before_zombie;
}

float GetDayMinFade(day)
{
    return arrayDays[day].data_fademin;
}

float GetDayMaxFade(day)
{
    return arrayDays[day].data_fademax;
}

BeginDay()
{
    gZombiesKilled = 0;
    
    int zombiecount = GetDayCount(gDay);
    int zombiemax = GetConVarInt(gCvars.CVAR_ZOMBIEMAX);

    int spawncount;
    bool respawn = GetDayRespawn(gDay);
    
    if (respawn)
    {
        spawncount = zombiemax;
    }
    else
    {
        spawncount = (zombiecount < zombiemax) ? zombiecount : zombiemax;
    }
    
    ServerCommand("bot_quota %d", spawncount);
    
    char display[32];
    GetDayDisplay(gDay, display, sizeof(display));
    
    bool override = ExplodeZombieOverrides(gDay);
    
    int maxplayers = MaxClients;
    for (int x = 1; x <= maxplayers; x++)
    {
        if (!IsClientInGame(x))
        {
            continue;
        }
        
        ChangeClientDeathCount(x, 0);
        
        if (!IsPlayerZombie(x))
        {
            continue;
        }
        
        if (override)
        {
            int size = GetArraySize(adtZombies);
            if (size)
            {
                int zombieid = GetRandomInt(0, size - 1);
                            
                Zombify(x, GetArrayCell(adtZombies, zombieid));
            }
        }
    }
}
