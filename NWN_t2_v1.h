#include "NW_I0_GENERIC"
#include "our_constants"

string GetSpecificTarget()
{
    // The next line moves to the spawn location of the similar opponent
    // ActionMoveToLocation( GetLocation( GetObjectByTag( "WP_" + OpponentColor( OBJECT_SELF ) + "_" + IntToString( GetLocalInt( OBJECT_SELF, "INDEX" ) ) ) ), TRUE );
    object oChar1 = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_1");  //wizardess
    object oChar2 = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_2");  //cleric
    object oChar3 = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_3");  //fighter
    object oChar4 = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_4");  //master
    object oChar5 = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_5");  //amazon
    object oChar6 = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_6");  //priestess
    object oChar7 = GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_7");  //wizard

    if (oChar1 == OBJECT_SELF)
    {
        return WpDoubler();
    }
    else if (oChar2 == OBJECT_SELF)
    {
        return ("WP" + MyColor(OBJECT_SELF) + "_4");
    }
    else if( oChar3 == OBJECT_SELF)
    {
        return ("WP" + MyColor(OBJECT_SELF) + "_4");
    }
    else if (oChar4 == OBJECT_SELF)
    {
        return ("WP" + MyColor(OBJECT_SELF) + "_4");
    }
    else if (oChar5 == OBJECT_SELF)
    {
        return ("WP" + MyColor(OBJECT_SELF) + "_4");
    }
    else if (oChar6 == OBJECT_SELF)
    {
        return ("WP" + MyColor(OBJECT_SELF) + "_4");
    }
    else if (oChar7 == OBJECT_SELF)
    {
        return  ("WP" + MyColor(OBJECT_SELF) + "_4");
    }
    else
    {
        //ActionSpeakString(GetTag(OBJECT_SELF), TALKVOLUME_SHOUT);
        return GetRandomTarget();
    }
}

// Called every time that the AI needs to take a combat decision. The default is
// a call to the NWN DetermineCombatRound.
void T2_DetermineCombatRound( object oIntruder = OBJECT_INVALID, int nAI_Difficulty = 10 )
{
    DetermineCombatRound( oIntruder, nAI_Difficulty );
}

// Called every heartbeat (i.e., every six seconds).
void T2_HeartBeat()
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
        sTarget = GetSpecificTarget();
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

// Called when the NPC is spawned.
void T2_Spawn()
{
    string sTarget = GetSpecificTarget();
    SetLocalString( OBJECT_SELF, "TARGET", sTarget );
    ActionMoveToLocation( GetLocation( GetObjectByTag( sTarget ) ), TRUE );
}

// This function is called when certain events take place, after the standard
// NWN handling of these events has been performed.
void T2_UserDefined( int Event )
{
    // In a word, fatal is certainly having only 1 HP left!
    int nHealth = GetCurrentHitPoints() - 1;

    // Declare the damage - visuals, however, are not automatic.
    effect eOuch = EffectDamage(nHealth, DAMAGE_TYPE_DIVINE);

    effect asdfgh = EffectParalyze();

    switch (Event)
    {
        // The NPC has just been attacked.
        case EVENT_ATTACKED:
             //ActionSpeakString("IM ATTACKED", TALKVOLUME_SHOUT);
             break;

        // The NPC was damaged.
        case EVENT_DAMAGED:
            //ActionSpeakString("IM DAMAGED", TALKVOLUME_SHOUT);
            //ApplyEffectToObject( DURATION_TYPE_TEMPORARY, asdfgh, OBJECT_SELF, 10.0 );
            break;

        // At the end of one round of combat.
        case EVENT_END_COMBAT_ROUND:
            //ActionSpeakString("END COMBAT ROUND", TALKVOLUME_SHOUT);
            break;

        // Every heartbeat (i.e., every six seconds).
        case EVENT_HEARTBEAT:
            T2_HeartBeat();

            // Apply the damage instantly
            //ApplyEffectToObject(DURATION_TYPE_INSTANT, eOuch, OBJECT_SELF);

            //ActionSpeakString("HEARTBEAT", TALKVOLUME_SHOUT);

            break;

        // Whenever the NPC perceives a new creature.
        case EVENT_PERCEIVE:
            //ActionSpeakString("PERCIEVE", TALKVOLUME_SHOUT);
            break;

        // When a spell is cast at the NPC.
        case EVENT_SPELL_CAST_AT:
           //ActionSpeakString("SPELL CAST", TALKVOLUME_SHOUT);
           break;

        // Whenever the NPC's inventory is disturbed.
        case EVENT_DISTURBED:
            //ActionSpeakString("INVENTORY DISTURBED", TALKVOLUME_SHOUT);
            break;

        // Whenever the NPC dies.
        case EVENT_DEATH:
            //ActionSpeakString("DIEDED", TALKVOLUME_SHOUT);
            ActionSpeakString(GetTag(OBJECT_SELF), TALKVOLUME_SHOUT);
            break;

        // When the NPC has just been spawned.
        case EVENT_SPAWN:
            T2_Spawn();
            ActionSpeakString("Spawn", TALKVOLUME_SHOUT);
            break;
    }

    return;
}

// Called when the fight starts, just before theiinital spawning.
void T2_Initialize( string sColor )
{
    SetTeamName( sColor, "Default-" + GetStringLowerCase( sColor ) );
}