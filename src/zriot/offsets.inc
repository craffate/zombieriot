/**
 * ====================
 *     Zombie Riot
 *   File: offsets.inc
 *   Author: Greyscale
 * ====================
 */

int offsBaseVelocity;
int offsGetVelocity2;
int offsSpeed;
int offsCollision;
int offsMoney;
int offsFOV;
int offsBuyZone;
int offsFadeMin;
int offsFadeMax;

FindOffsets()
{
    offsBaseVelocity = FindSendPropInfo("CBasePlayer", "m_vecBaseVelocity");
    if (offsBaseVelocity == -1)
    {
        SetFailState("Couldn't find \"m_vecBaseVelocity\"!");
    }
    
    offsGetVelocity2 = FindSendPropInfo("CBasePlayer", "m_vecVelocity[2]");
    if (offsGetVelocity2 == -1)
    {
        SetFailState("Couldn't find \"m_vecVelocity[2]\"!");
    }
    
    offsSpeed = FindSendPropInfo("CCSPlayer", "m_flLaggedMovementValue");
    if (offsSpeed == -1)
    {
        SetFailState("Couldn't find \"m_flLaggedMovementValue\"!");
    }
    
    offsCollision = FindSendPropInfo("CBaseEntity", "m_CollisionGroup");
    if (offsCollision == -1)
    {
        SetFailState("Couldn't find \"m_CollisionGroup\"!");
    }
    
    offsMoney = FindSendPropInfo("CCSPlayer", "m_iAccount");
    if (offsMoney == -1)
    {
        SetFailState("Couldn't find \"m_iAccount\"!");
    }
    
    offsFOV = FindSendPropInfo("CBasePlayer", "m_iDefaultFOV");
    if (offsFOV == -1)
    {
        SetFailState("Couldn't find \"m_iDefaultFOV\"!");
    }
    
    offsBuyZone = FindSendPropInfo("CCSPlayer", "m_bInBuyZone");
    if (offsBuyZone == -1)
    {
        SetFailState("Couldn't find \"m_bInBuyZone\"!");
    }
    
    offsFadeMin = FindSendPropInfo("CCSPlayer", "m_fadeMinDist");
    if (offsFadeMin == -1)
    {
        SetFailState("Couldn't find \"m_fadeMinDist\"!");
    }
    
    offsFadeMax = FindSendPropInfo("CCSPlayer", "m_fadeMaxDist");
    if (offsFadeMax == -1)
    {
        SetFailState("Couldn't find \"m_fadeMaxDist\"!");
    }
}

SetPlayerVelocity(client, const float vec[3])
{
    SetEntDataVector(client, offsBaseVelocity, vec, true);
}

SetPlayerSpeed(client, float speed)
{
    float intspeed = speed / 300.0;
    SetEntDataFloat(client, offsSpeed, intspeed, true);
}

NoCollide(client, bool nocollide)
{
    if (nocollide)
    {
        SetEntData(client, offsCollision, 2, 1, true);
    }
    else
    {
        SetEntData(client, offsCollision, 5, 1, true);
    }
}

SetPlayerMoney(client, amount)
{
    SetEntData(client, offsMoney, amount, 4, true);
}

SetPlayerFOV(client, fov)
{
    SetEntData(client, offsFOV, fov, 1, true);
}

bool IsClientInBuyZone(client)
{
    return bool:GetEntData(client, offsBuyZone);
}

SetPlayerMinDist(client, float mindist)
{
    SetEntDataFloat(client, offsFadeMin, mindist);
}

SetPlayerMaxDist(client, float maxdist)
{
    SetEntDataFloat(client, offsFadeMax, maxdist);
}

/**
 * Remove all weapons.
 * 
 * @param client        The client index.
 * @param weapons       An array of boolean values for each weapon slot.  True means remove, false means ignore.
 */
stock RemoveAllPlayersWeapons(client)
{
    int weaponindex;
    for (int weaponslot = 0; weaponslot < 5; weaponslot++)
    {
        weaponindex = GetPlayerWeaponSlot(client, weaponslot);
        if (weaponindex != -1)
        {
            Util_RemovePlayerItem(client, weaponindex);
        }
    }
    
    // Remove left-over projectiles.
    WepLib_GrenadeStripAll(client);
}

/**
 * Used to explicitly remove projectiles from a client.
 * 
 * @param client    The client index.
 */
stock WepLib_GrenadeStripAll(client)
{
    // While GetPlayerWeaponSlot returns a valid projectile, remove it and then test again.
    int grenade = GetPlayerWeaponSlot(client, 3);
    while (grenade != -1)
    {
        Util_RemovePlayerItem(client, grenade);
        grenade = GetPlayerWeaponSlot(client, 3);
    }
}

/**
 * Fully remove a weapon from a client's inventory and the world.
 * 
 * @param client        The client whose weapon to remove.
 * @param weaponindex   The weapon index.
 */
stock Util_RemovePlayerItem(client, weaponindex)
{
    RemovePlayerItem(client, weaponindex);
    RemoveEdict(weaponindex);
}

TerminateRound(float delay, CSRoundEndReason reason)
{
    if (tHUD == INVALID_HANDLE)
        return;
    
    CS_TerminateRound(delay, reason);
}