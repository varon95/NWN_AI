#include "NW_I0_GENERIC"
#include "our_constants"


object GetPortal()
{
    return GetObjectByTag("PORTAL_" + MyColor(OBJECT_SELF) + "_4");
}

//creating a path of waypoints
//you should also set the number of waypoints in the path
void initializeMasterPath()
{
    //initialize steps fpr Master
    SetLocalInt(GetPortal(), "iTargetnum", 1);
    SetLocalInt(GetPortal(), "iNumberPathPoins", 6);

    SetLocalString(GetPortal(), "sPathStep_1","WP_ALTAR_" + OpponentColor() + "_1C" );
    SetLocalString(GetPortal(), "sPathStep_2","WP_ALTAR_" + OpponentColor() + "_1B" );
    SetLocalString(GetPortal(), "sPathStep_3","WP_CROSSROADS_" + OpponentColor() );
    SetLocalString(GetPortal(), "sPathStep_4","WP_ALTAR_" + OpponentColor() + "_2B" );
    SetLocalString(GetPortal(), "sPathStep_5","WP_ALTAR_" + OpponentColor() + "_2C" );
    SetLocalString(GetPortal(), "sPathStep_6","WP_CENTRE_" + OpponentColor() + "_2" );
    //SetLocalString(GetPortal(), "sPathStep_7","WP_CENTRE_" + MyColor() + "_2" );
    //SetLocalString(GetPortal(), "sPathStep_8","WP_ALTAR_" + MyColor() + "_2D" );

}

//calculates the next point in the path
string T5_NextStepInPath()
{
    if(!GetLocalInt(GetPortal(), "iTargetnum")){initializeMasterPath();}

    int iTargetnum = GetLocalInt(GetPortal(), "iTargetnum");
    int iNumberPathPoins = GetLocalInt(GetPortal(), "iNumberPathPoins");
    iTargetnum = (iTargetnum%iNumberPathPoins)+1;
    SetLocalInt(GetPortal(), "iTargetnum", iTargetnum);
    return GetLocalString(GetPortal(), "sPathStep_" + IntToString(iTargetnum));
}

//creates a new evet, which runs every three seconds
//it is used to get the master to the next destination
void miniHeartBeat()
{
SetLocalInt(OBJECT_SELF, "IN_MINI_HEARTBEAT", TRUE);
object oTarget = GetObjectByTag( GetLocalString( OBJECT_SELF, "TARGET" ) );
if(GetDistanceBetween( OBJECT_SELF, oTarget ) < 40.0)
     {
        SetLocalString( OBJECT_SELF, "TARGET", T5_NextStepInPath() );
        oTarget = GetObjectByTag( GetLocalString( OBJECT_SELF, "TARGET" ) );
     }
ActionMoveToLocation( GetLocation( oTarget ), TRUE );
DelayCommand(3.0,  miniHeartBeat());
}



// the least number of enemies
// the least hit points for enemies
int GetAltarBalanceCoefficient(string sAltar)
{
    object oPortal = GetPortal();
    int iFriends = GetLocalInt(oPortal, sAltar + "_ENEMIES_COUNT");
    int iEnemies = GetLocalInt(oPortal, sAltar + "_FRIENDS_COUNT");

    int iFriendsHPoints = GetLocalInt(oPortal, sAltar + "_ENEMIES_HPOINTS");
    int iEnemiesHPoints = GetLocalInt(oPortal, sAltar + "_FRIENDS_HPOINTS");
    int iCoeff = - iEnemies*10 - iEnemiesHPoints;

    return iCoeff;
}

string IsAltarEmpty (object oAltar) {

    object oCreature = GetNearestObjectToLocation( OBJECT_TYPE_CREATURE, GetLocation( oAltar ), 1 );
    if (GetDistanceBetween(oCreature, oAltar) < 5.0 && oCreature != OBJECT_SELF)
    {
        return "FALSE";
    } else {
        return "TRUE";
    }
}

string GetWeakestEnemyPoint()
{
    string sOpponentAltar1 = "WP_ALTAR_" + OpponentColor(OBJECT_SELF) + "_1";
    string sOpponentAltar2 = "WP_ALTAR_" + OpponentColor(OBJECT_SELF) + "_2";
    string sDoubler = "WP_DOUBLER";

    int iCoeffAltar1 = GetAltarBalanceCoefficient(sOpponentAltar1);
    int iCoeffAltar2 = GetAltarBalanceCoefficient(sOpponentAltar2);
    int iCoeffDoubler = GetAltarBalanceCoefficient(sDoubler);

    if (iCoeffAltar1 >= iCoeffAltar2 && iCoeffAltar1 > iCoeffDoubler)
    {
       return sOpponentAltar1;
    } else {
       return  sOpponentAltar2;
    }

    //if (iCoeffAltar1 >= iCoeffAltar2 && iCoeffAltar1 > iCoeffDoubler)
    //{
    //   return sOpponentAltar1;
    //} else if (iCoeffAltar2 >= iCoeffAltar1 && iCoeffAltar2 > iCoeffDoubler){
    //   return  sOpponentAltar2;
    //} {
    //    return sDoubler;
    //}


}

void UpdateAltarStatistics(string sAltar) {
  object oAltar = GetObjectByTag(sAltar);

  int iFriends = 0;
  int iEnemies = 0;
  int iFriendsHPoints = 0;
  int iEnemiesHPoints = 0;
  int iFriendsUnderAttack = 0;

  int i = 1;

  object oCreature = GetNearestObjectToLocation( OBJECT_TYPE_CREATURE, GetLocation( oAltar ), i );
  while ( i < 6 )
  {
    if (GetDistanceBetween(oCreature, oAltar) < 30.0)
    {
        if (SameTeam( oCreature ))
        {
            ++iFriends;
            iFriendsHPoints += GetCurrentHitPoints(oCreature);
            if (GetIsInCombat(oCreature)) {
                ++iFriendsUnderAttack;
            }
        } else {
            ++iEnemies;
            iEnemiesHPoints += GetCurrentHitPoints(oCreature);
        }
    }

    ++i;
    oCreature = GetNearestObjectToLocation( OBJECT_TYPE_CREATURE, GetLocation( oAltar ), i );
  }

  object oPortal = GetPortal();
  SetLocalInt(oPortal, sAltar + "_ENEMIES_COUNT", iEnemies);
  SetLocalInt(oPortal, sAltar + "_FRIENDS_COUNT", iFriends);

  SetLocalInt(oPortal, sAltar + "_ENEMIES_HPOINTS", iEnemiesHPoints);
  SetLocalInt(oPortal, sAltar + "_FRIENDS_HPOINTS", iFriendsHPoints);

  SetLocalInt(oPortal, sAltar + "_FRIENDS_UNDERATTACK", iFriendsUnderAttack);

}

string GetAltarState(string sAltar)
{
  object oPortal = GetPortal();
  int iFriends = GetLocalInt(oPortal, sAltar + "_FRIENDS_COUNT");
  int iEnemies = GetLocalInt(oPortal, sAltar + "ENEMIES_COUNT");
  int iFriendsUnderAttack = GetLocalInt(oPortal, sAltar + "_FRIENDS_UNDERATTACK");

  int iSum = iFriends + iEnemies;
  if ((iFriends > iEnemies && iFriendsUnderAttack == 0) || iSum == 0) {
        return "SAFE";
    //return "SAFE: FRIENDS: " + IntToString(iFriends) + "  ENEMIES: " + IntToString(iEnemies) + "  UNDER ATTACK: " + IntToString(iFriendsUnderAttack);
  } else {
    //return "BATTLE: FRIENDS: " + IntToString(iFriends) + "  ENEMIES: " + IntToString(iEnemies) + "  UNDER ATTACK: " + IntToString(iFriendsUnderAttack);
        return "BATTLE";
  }
}

string T5_GetSpecificTarget()
{
    if (IsMaster()){return T5_NextStepInPath();}
    else
    {
        // The next line moves to the spawn location of the similar opponent
        // ActionMoveToLocation( GetLocation( GetObjectByTag( "WP_" + OpponentColor( OBJECT_SELF ) + "_" + IntToString( GetLocalInt( OBJECT_SELF, "INDEX" ) ) ) ), TRUE );
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

        if (oWizardess == OBJECT_SELF)
        {
            if (IsAltarEmpty(GetObjectByTag("WP_DOUBLER")) == "TRUE" && GetAltarState(sMyAltar1) == "SAFE") {
                ActionSpeakString("Going for the doubler", TALKVOLUME_SHOUT);
                return "WP_DOUBLER";
            }

            return sMyAltar1 + "C";
        }
        else if (oCleric == OBJECT_SELF)
        {
            return sMyAltar1;
        }
        else if( oFighter == OBJECT_SELF)
        {
            if (GetAltarState(sMyAltar1) == "BATTLE") {
                return sMyAltar1 + "_E";
            } else {
                return GetWeakestEnemyPoint();
            }
        }
        else if (oMaster == OBJECT_SELF)
        {
            if (GetAltarState(sMyAltar1) == "BATTLE") {
                return sMyAltar1;
            }
            if (GetAltarState(sMyAltar2)== "BATTLE"){
                return sMyAltar2;
            } else {
                return GetWeakestEnemyPoint();
            }
        }
        else if (oAmazon == OBJECT_SELF)
        {
           if (IsAltarEmpty(GetObjectByTag("WP_DOUBLER")) == "TRUE" && GetAltarState(sMyAltar2) == "SAFE") {
                ActionSpeakString("Going for the doubler", TALKVOLUME_SHOUT);
                return "WP_DOUBLER";
           } else {
                return sMyAltar2 + "C";
           }
        }
        else if (oPriestess == OBJECT_SELF)
        {
            return sMyAltar2;
        }
        else if (oWizard == OBJECT_SELF)
        {

            if (GetAltarState(sMyAltar2) == "BATTLE") {
                return sMyAltar2;
            } else {
                return GetWeakestEnemyPoint();
            }
        }
        else
        {
            ActionSpeakString("Getting Random Target", TALKVOLUME_SHOUT);
            return GetRandomTarget();
        }
    }
}


// Called every time that the AI needs to take a combat decision. The default is
// a call to the NWN DetermineCombatRound.
void T5_DetermineCombatRound( object oIntruder = OBJECT_INVALID, int nAI_Difficulty = 10 )
{   if(IsMaster())
    {
        if(GetHealth()>3){TalentHealingSelf();}
    }
    else{ DetermineCombatRound( oIntruder, nAI_Difficulty ); }
}


void T5_ObtainDoubler () {
    if (OBJECT_SELF != GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_5"))
    {
        return;
    }

    if (IsAltarEmpty(GetObjectByTag("WP_DOUBLER")) == "TRUE" && GetAltarState("WP_ALTAR_" + MyColor(OBJECT_SELF) + "_2") == "SAFE") {
        ActionSpeakString("Going for the doubler", TALKVOLUME_SHOUT);
        ActionMoveToLocation( GetLocation( GetObjectByTag("WP_DOUBLER") ), TRUE );
     }

}
// Called every heartbeat (i.e., every six seconds).
void T5_HeartBeat()
{
    //SpeakString(GetLocalString( OBJECT_SELF, "TARGET" ));/////////////////////////////////////////////////////////////////////////

    string sMyAltar1 = "WP_ALTAR_" + MyColor(OBJECT_SELF) + "_1";
    string sMyAltar2 = "WP_ALTAR_" + MyColor(OBJECT_SELF) + "_2";
    string sOpponentAltar1 = "WP_ALTAR_" + OpponentColor(OBJECT_SELF) + "_1";
    string sOpponentAltar2 = "WP_ALTAR_" + OpponentColor(OBJECT_SELF) + "_2";

    UpdateAltarStatistics(sMyAltar1);
    UpdateAltarStatistics(sMyAltar2);
    UpdateAltarStatistics(sOpponentAltar1);
    UpdateAltarStatistics(sOpponentAltar2);

    string sAltar1State = GetAltarState(sMyAltar1);
    string sAltar2State = GetAltarState(sMyAltar2);

    //ActionSpeakString("ALTAR 1 State: " + sAltar1State, TALKVOLUME_SHOUT);
    //ActionSpeakString("ALTAR 2 State: " + sAltar2State, TALKVOLUME_SHOUT);


    if (GetIsInCombat())
        return;

    //T5_ObtainDoubler();
    string sTarget = GetLocalString( OBJECT_SELF, "TARGET" );
    if (sTarget == "")
        return;

    object oTarget = GetObjectByTag( sTarget );
    if (!GetIsObjectValid( oTarget ))
        return;

    // If there is a member of my own team close to the target and closer than me,
    // and no enemy is closer and this other member is not in combat and
    // has the same target, then choose a new target. }

    if (IsMaster())
    {
        if(!GetLocalInt(OBJECT_SELF, "IN_MINI_HEARTBEAT"))
        {
          miniHeartBeat();
        }
        return;
    }

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
        ActionSpeakString("Looking for new target", TALKVOLUME_SHOUT);
        sTarget = T5_GetSpecificTarget();
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

void T5_HandleAttack()
{
    string sTarget = GetLocalString( OBJECT_SELF, "TARGET" );
    if (
      GetObjectByTag("NPC_" + MyColor(OBJECT_SELF) + "_4") != OBJECT_SELF &&
      sTarget == "WP_ALTAR_" + MyColor(OBJECT_SELF) + "_1" || sTarget == "WP_ALTAR_" + MyColor(OBJECT_SELF) + "_2")
    {
      ActionMoveToLocation( GetLocation( GetObjectByTag(sTarget) ), TRUE );
    }
}

// Called when the NPC is spawned.
void T5_Spawn()
{
    string sTarget = T5_GetSpecificTarget();
    SetLocalString( OBJECT_SELF, "TARGET", sTarget );
    ActionMoveToLocation( GetLocation( GetObjectByTag( sTarget ) ), TRUE );
}

// This function is called when certain events take place, after the standard
// NWN handling of these events has been performed.
void T5_UserDefined( int Event )
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
             T5_HandleAttack();
             break;

        // The NPC was damaged.
        case EVENT_DAMAGED:
            //ActionSpeakString("IM DAMAGED", TALKVOLUME_SHOUT);
            //ApplyEffectToObject( DURATION_TYPE_TEMPORARY, asdfgh, OBJECT_SELF, 10.0 );
            break;

        // At the end of one round of combat.
        case EVENT_END_COMBAT_ROUND:
            break;

        // Every heartbeat (i.e., every six seconds).
        case EVENT_HEARTBEAT:
            T5_HeartBeat();

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
            //ActionSpeakString(GetTag(OBJECT_SELF), TALKVOLUME_SHOUT);
            break;

        // When the NPC has just been spawned.
        case EVENT_SPAWN:
            T5_Spawn();
            //ActionSpeakString("Spawn", TALKVOLUME_SHOUT);
            break;
    }

    return;
}

// Called when the fight starts, just before theiinital spawning.
void T5_Initialize( string sColor )
{
    SetTeamName( sColor, "Default-" + GetStringLowerCase( sColor ) );
}