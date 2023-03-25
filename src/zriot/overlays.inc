/**
 * ====================
 *     Zombie Riot
 *   File: overlays.inc
 *   Author: Greyscale
 * ==================== 
 */

ShowOverlays(float time, winner)
{
    char overlay[64];
    if (winner == gZombieTeam)
    {
        GetConVarString(gCvars.CVAR_OVERLAYS_ZOMBIE, overlay, sizeof(overlay));
    }
    else if (winner == gHumanTeam)
    {
        GetConVarString(gCvars.CVAR_OVERLAYS_HUMAN, overlay, sizeof(overlay));
    }
        
    int maxplayers = MaxClients;
    for (int x = 1; x <= maxplayers; x++)
    {
        if (!IsClientInGame(x))
            continue;
        
        DisplayClientOverlay(x, overlay);
    }
    
    CreateTimer(time, KillOverlays);
}

public Action KillOverlays(Handle timer)
{
    int maxplayers = MaxClients;
    for (int x = 1; x <= maxplayers; x++)
    {
        if (!IsClientInGame(x))
            continue;
        
        ClientCommand(x, "r_screenoverlay \"\"");
    }
}