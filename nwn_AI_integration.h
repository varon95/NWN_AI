#include "NW_I0_GENERIC"
#include "our_constants"
//---------------------Common functions-----------------------------------------
//this is the new determine combat round
void T4_DetermineCombatAction()
{
    object oClosestEnemy = GetNearestEnemy();

    if (IsWizard())
    {
      ActionCastSpellAtObject(SPELL_MAGIC_MISSILE, oClosestEnemy);
      if(!CheckCurrentAction())
      {TalentSpellAttack(oClosestEnemy);}
    }
    else if (IsCleric())
    {
      TalentHeal();
    }
    else if (IsMaster())
    {
    TalentMeleeAttack(oClosestEnemy);
    }
    else if (IsFighterLeft())
    {
    TalentMeleeAttack(oClosestEnemy);
    }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//----------------------Random movement based AI--------------------------------
//------------------------------------------------------------------------------
//at the beginning choosing specific targets are more effective then random
string T4_GetSpecificTarget()
{
    object oWizardess = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_1");  //wizardess
    object oCleric = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_2");  //cleric
    object oFighter = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_3");  //fighter
    object oMaster = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_4");  //master
    object oAmazon = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_5");  //amazon
    object oPriestess = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_6");  //priestess
    object oWizard = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_7");  //wizard

    string sMyAltar1 = "WP_ALTAR_" + MyColor(OBJECT_SELF) + "_1";
    string sMyAltar2 = "WP_ALTAR_" + MyColor(OBJECT_SELF) + "_2";
    string sOpponentAltar1 = "WP_ALTAR_" + OpponentColor(OBJECT_SELF) + "_1";
    string sOpponentAltar2 = "WP_ALTAR_" + OpponentColor(OBJECT_SELF) + "_2";
    string sDoubler = "WP_DOUBLER";

    if (oWizardess == OBJECT_SELF){return sOpponentAltar1 ;}
    else if (oCleric == OBJECT_SELF){return sMyAltar1  ;}
    else if( oFighter == OBJECT_SELF){return sDoubler ;}
    else if (oMaster == OBJECT_SELF){return sOpponentAltar1 ;}
    else if (oAmazon == OBJECT_SELF){return sMyAltar1 ;}
    else if (oPriestess == OBJECT_SELF){return sMyAltar2 ;}
    else if (oWizard == OBJECT_SELF){return sDoubler ;}
    else {return sDoubler;}
}

//this is the heart beat. any changes to the heartbeat must be implemented here
//this runs every second thanks to a recursive delay command
void T4_OriginalHearthBeat()
{
    if (GetIsInCombat())
        return;

    string sTarget = GetLocalString( OBJECT_SELF, "TARGET" );
    if (sTarget == "")
        return;

    object oTarget = GetObjectByTag( sTarget );
    if (!GetIsObjectValid( oTarget ))
        return;

    // If there is a member of my own team close to the target and closer than me,
    // and no enemy is closer and this other member is not in combat and
    // has the same target, then choose a new target.
    float fToTarget = GetDistanceToObject( oTarget );
    int i = 1;
    int bNewTarget = FALSE;
    object oCreature = GetNearestObjectToLocation( OBJECT_TYPE_CREATURE, GetLocation( oTarget ), i );
    while (GetIsObjectValid( oCreature ))
    {
        if (GetLocation( oCreature ) == GetLocation( OBJECT_SELF ))
            break;
        if (GetDistanceBetween( oCreature, oTarget ) > fToTarget)
            break;
        if (GetDistanceBetween( oCreature, oTarget ) > 5.0)
            break;
        if (!SameTeam( oCreature ))
            break;
        if (GetIsInCombat( oCreature ))
            break;
        if (GetLocalString( oCreature, "TARGET" ) == sTarget)
        {
            bNewTarget = TRUE;
            break;
        }
        ++i;
        oCreature = GetNearestObjectToLocation( OBJECT_TYPE_CREATURE, GetLocation( oTarget ), i );
    }

    if (bNewTarget)
    {
    //at the beginning go to specific targets, all later targets are random
        if(!GetLocalInt(MyPortal(), "iFirstTarget"))
    {
            sTarget = T4_GetSpecificTarget();
            SetLocalInt(MyPortal(), "iFirstTarget", TRUE);
        }
        else{sTarget = GetRandomTarget();}
    //---------------------------------------------------------------------

        SetLocalString( OBJECT_SELF, "TARGET", sTarget );
        oTarget = GetObjectByTag( sTarget );
        if (!GetIsObjectValid( oTarget ))
            return;
        fToTarget = GetDistanceToObject( oTarget );
    }

    if (fToTarget > 0.5)
        ActionMoveToLocation( GetLocation( oTarget ), TRUE );

    return;
}
//------------------------------------------------------------------------------
//----------------------End of Random movement based AI-------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//-----------------------------Main functions-----------------------------------
//------------------------------------------------------------------------------
// Called every time that the AI needs to take a combat decision
void T4_DetermineCombatRound( object oIntruder = OBJECT_INVALID, int nAI_Difficulty = 10 )
{
    T4_DetermineCombatAction();
}

//-----------------Mini heartbeat -----------------
//creates a new evet, which runs every second
void T4_MiniHeartBeat()
{
    SetLocalInt(OBJECT_SELF, "IN_MINI_HEARTBEAT", TRUE);
    T4_OriginalHearthBeat();
    DelayCommand(1.0, T4_MiniHeartBeat());
}

// Called every heartbeat (i.e., every six seconds).
void T4_HeartBeat()
{
    if(!GetLocalInt(OBJECT_SELF, "IN_MINI_HEARTBEAT"))
    {
        T4_MiniHeartBeat();
    }
    return;
}
//------------------End of mini heartbeat --------------------------

// Called when the NPC is spawned.
void T4_Spawn()
{
    string sTarget = GetRandomTarget();
    SetLocalString( OBJECT_SELF, "TARGET", sTarget );
    ActionMoveToLocation( GetLocation( GetObjectByTag( sTarget ) ), TRUE );
}

// This function is called when certain events take place, after the standard
// NWN handling of these events has been performed.
void T4_UserDefined( int Event )
{
    switch (Event)
    {
        // The NPC has just been attacked.
        case EVENT_ATTACKED:
            break;

        // The NPC was damaged.
        case EVENT_DAMAGED:
            break;

        // At the end of one round of combat.
        case EVENT_END_COMBAT_ROUND:
            break;

        // Every heartbeat (i.e., every six seconds).
        case EVENT_HEARTBEAT:
            T4_HeartBeat();
            break;

        // Whenever the NPC perceives a new creature.
        case EVENT_PERCEIVE:
            break;

        // When a spell is cast at the NPC.
        case EVENT_SPELL_CAST_AT:
            break;

        // Whenever the NPC's inventory is disturbed.
        case EVENT_DISTURBED:
            break;

        // Whenever the NPC dies.
        case EVENT_DEATH:
            break;

        // When the NPC has just been spawned.
        case EVENT_SPAWN:
            T4_Spawn();
            break;
    }

    return;
}

// Called when the fight starts, just before the initial spawning.
void T4_Initialize( string sColor )
{
    SetTeamName( sColor, "KillerAi-" + GetStringLowerCase( sColor ) );
}