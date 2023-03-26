/**
 * ====================
 *     Zombie Riot
 *   File: zombieriot.inc
 *   Author: Greyscale
 * ==================== 
 */

#define DXLEVEL_MIN 90

#define DEFAULT_FOV 90
#define DEFAULT_GRAVITY 1.0

bool market;

char gMapConfig[PLATFORM_MAX_PATH];

int gDay;

int gZombieTeam;
int gHumanTeam;

int gZombiesKilled;

int dxLevel[MAXPLAYERS+1];

bool bZombie[MAXPLAYERS+1];
Handle trieDeaths = INVALID_HANDLE;

int gRespawnTime[MAXPLAYERS+1];
Handle tRespawn[MAXPLAYERS+1];
Handle tZVision[MAXPLAYERS+1];

bool bZVision[MAXPLAYERS+1];

Handle tHUD = INVALID_HANDLE;
Handle tFreeze = INVALID_HANDLE;

QueryCookie mat_dxlevel;

FindClientDXLevel(client)
{
    if (IsFakeClient(client))
    {
        return;
    }
    
    mat_dxlevel = QueryClientConVar(client, "mat_dxlevel", DXLevelClientQuery);
}

public DXLevelClientQuery(QueryCookie cookie, client, ConVarQueryResult result, const char[] cvarName, const char[] cvarValue)
{
    if (cookie != mat_dxlevel)
    {
        return;
    }
    
    dxLevel[client] = 0;
    
    if (result != ConVarQuery_Okay)
    {
        return;
    }
    
    dxLevel[client] = StringToInt(cvarValue);
}

DisplayClientOverlay(client, const char[] overlay)
{
    if (!dxLevel[client])
    {
        FindClientDXLevel(client);
        
        return;
    }
    
    if (dxLevel[client] >= DXLEVEL_MIN)
    {
        ClientCommand(client, "r_screenoverlay \"%s\"", overlay);
    }
    else
    {
        ZRiot_PrintCenterText(client, "DX90 not supported", dxLevel[client], DXLEVEL_MIN);
    }
}

stock GotoNextMap()
{
    Handle timelimit = FindConVar("mp_timelimit");
    
    if (timelimit == INVALID_HANDLE)
    {
        return;
    }
    
    int flags = GetConVarFlags(timelimit) & FCVAR_NOTIFY;
    SetConVarFlags(timelimit, flags);
    
    SetConVarInt(timelimit, 1);
}