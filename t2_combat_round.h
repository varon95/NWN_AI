#include "nw_i0_generic"

//::///////////////////////////////////////////////
//:: chooseTactics
//:: Copyright (c) 2001 Bioware Corp.
//:://////////////////////////////////////////////
/*
    Separated this function out from DetermineCombatRound
    for readibility
*/
//:://////////////////////////////////////////////
//:: Created By: Brent
//:: Created On: September 2002
//:://////////////////////////////////////////////

int newChooseTactics(object oIntruder)
{

    // SELF PRESERVATION: Always attempt to heal self first
    if(TalentHealingSelf() == TRUE) return 99; //Use spells and potions

    // Next, try the special tactics routines
    // specific to XP1
    if (SpecialTactics(oIntruder)) return 99;

    // * These constants in ChooseTactics routine
    // * remember previous rounds choices

    //moved to top of script, made into real constants
    //int      MEMORY_OFFENSE_MELEE    = 0;
    //int      MEMORY_DEFENSE_OTHERS   = 1;
    //int      MEMORY_DEFENSE_SELF     = 2;
    //int      MEMORY_OFFENSE_SPELL    = 3;

    // * If defensive last round, try to be offensive this round
    // * this is to prevent wasting time on multiple protections
    int nPreviousMemory = GetLocalInt(OBJECT_SELF, "NW_L_MEMORY");

    int nClass = DetermineClassToUse();

    //This does not seem to be used, so no point declaring it...
    //int nCrazy  =  0;

    // * Defaulted high so unspecified classes will not be cowards
    int nOffense = 50;

    int nCompassion = 25;

    // * Defaulted this high because non standard creatures
    // * with spells should try and use them.
    int nMagic = 55;

    // * setup base BEHAVIOR
    switch (nClass)
    {
        case CLASS_TYPE_COMMONER:
            // Commoners should run away from fights
            //SpawnScriptDebugger();
            nOffense = 0; nCompassion = 0; nMagic = 0;  break;
        case CLASS_TYPE_PALEMASTER:
        case  CLASS_TYPE_WIZARD:
        case  CLASS_TYPE_SORCERER:
                  //  SpawnScriptDebugger();
            nOffense = 40; nCompassion = 40; nMagic = 100; break;
        case CLASS_TYPE_BARD:
        case CLASS_TYPE_HARPER:
        case CLASS_TYPE_DRAGONDISCIPLE:
        {
            if(TalentBardSong() == TRUE) return 99;
            nOffense = 40; nCompassion = 42; nMagic = 43; break;
        }
        case CLASS_TYPE_CLERIC:
        case CLASS_TYPE_DRUID:
        case CLASS_TYPE_SHIFTER:
        {
            nOffense = 40;
            nCompassion = 45;
            nMagic = 44;
            // * Clerics shouldn't constantly cast spells
            if (nPreviousMemory != chooseTactics_MEMORY_OFFENSE_MELEE)
                nMagic = Random(50) + 1;
            break;
        }
        case CLASS_TYPE_PALADIN :
        case CLASS_TYPE_RANGER :
            nOffense = 40; nCompassion = 25; nMagic = Random(50) + 1; break;
        case CLASS_TYPE_BARBARIAN:
        {
           // SpawnScriptDebugger();
            // * GetHasFeat(...) does not work correctly with no-leveled up
            // * characters. So for now, only Xanos gets to do this.
            string sTag = GetTag(OBJECT_SELF);
            if (sTag == "x0_hen_xan" || sTag == "x2_hen_daelan")
            {
                if (GetHasFeatEffect(FEAT_BARBARIAN_RAGE) == FALSE)
                {

                    if (GetHasFeat(FEAT_BARBARIAN_RAGE) == TRUE)
                    {
                        ActionUseFeat(FEAT_BARBARIAN_RAGE, OBJECT_SELF);
                        return 99;
                    }
                }
            }
            nOffense = 50; nCompassion = 25; nMagic = 20; break;
            // * set high magic to use rage
            // * suggestion don't give barbarians lots of magic or else they will fight oddly
        }
        case CLASS_TYPE_WEAPON_MASTER:
        case CLASS_TYPE_ARCANE_ARCHER:
        case CLASS_TYPE_BLACKGUARD:
        case CLASS_TYPE_SHADOWDANCER:
        case CLASS_TYPE_DWARVENDEFENDER:
        case CLASS_TYPE_ASSASSIN:
        case CLASS_TYPE_FIGHTER:
        case CLASS_TYPE_ROGUE : //SpawnScriptDebugger();
        case CLASS_TYPE_MONK :
            nOffense = 40; nCompassion = 0; nMagic = 0; break;
        case CLASS_TYPE_UNDEAD:
            nOffense = 40; nCompassion = 40; nMagic = 40; break;
        case CLASS_TYPE_OUTSIDER:
        {
            nOffense = 40; nMagic = 40;
            if (GetAlignmentGoodEvil(OBJECT_SELF) == ALIGNMENT_GOOD)
            {
                nCompassion = 40;
            }
            else nCompassion = 0;
            break;
        }
        case CLASS_TYPE_CONSTRUCT:
        case CLASS_TYPE_ELEMENTAL:
            nOffense = 40; nCompassion = 0; nMagic = 40; break;
        case CLASS_TYPE_DRAGON:
            nOffense = 40; nCompassion = 20; nMagic = 40; break;
        default:
            nOffense = 50; nCompassion = 25; nMagic = 55; break;
    }

    //really minor optimization - since this bit doesn't rely on the variables set
    //below, might as well check it before we do all those calculations
    // * Dragon Disciple Breath
    if (GetHasFeat(FEAT_DRAGON_DIS_BREATH) && Random(100) > 50)
    {
        ClearActions(2000);
        ActionCastSpellAtObject(690, GetNearestEnemy(), METAMAGIC_ANY, TRUE);
        DecrementRemainingFeatUses(OBJECT_SELF, FEAT_DRAGON_DIS_BREATH);
        return 99;
    }


    // MyPrintString("Made it past the class-specific settings");


    // ************************************
    // * MODIFY BEHAVIOR FOR SPECIAL CASES
    // ************************************
    if (GetRacialType(OBJECT_SELF) == RACIAL_TYPE_UNDEAD)
        nCompassion = nCompassion - 20;

    // Randomize things a bit
    //seems that nCrazy is always 0, so might as well comment them out
    nOffense = Random(10 /*+ nCrazy*/) + nOffense;
    nMagic = Random(10 /*+ nCrazy*/) + nMagic;
    nCompassion = Random(10 /*+ nCrazy*/) + nCompassion;



    // * if your opponent is close to you, then increase offense
    // * as casting defensive abilities when enemies are close
    // * is generally not a good idea.
    // * Dec 18 2002: If you have Combat Casting, you'll still be more
    // * liable to use defensive abilities
    if (GetIsObjectValid(oIntruder) && !GetHasFeat(FEAT_COMBAT_CASTING))
    {
        if (GetDistanceToObject(oIntruder) <= 5.0) {
            nOffense = nOffense + 20;
            nMagic = nMagic - 20;
        }
    }

    // * If enemies are further away, more chance of doing magic
    if (GetDistanceToObject(oIntruder) > 3.0)
        nMagic = nMagic + 15;

    // * Dec 18 2002: Add your level to your magic rating
    nMagic = nMagic + GetHitDice(OBJECT_SELF);


   // **************************************
   // * CHOOSE TALENT TO USE
   // **************************************

    //SpawnScriptDebugger();

    // * If defensive last round, try to be offensive this round
    // * this is to prevent wasting time on multiple protections
    if ((nPreviousMemory == chooseTactics_MEMORY_DEFENSE_OTHERS)
        || (nPreviousMemory == chooseTactics_MEMORY_DEFENSE_SELF))
    {
        nOffense = nOffense + 40;
    }


    // April 2003
    // If in rage should be almost no chance of doing magic
    // * June 2003
    // * If has more than 5% chance of spell failure don't try casting
    // 5% chance changed to 15%
    if (GetHasFeatEffect(FEAT_BARBARIAN_RAGE)== TRUE || GetShouldNotCastSpellsBecauseofArmor(OBJECT_SELF, nClass) == TRUE
        || GetLocalInt(OBJECT_SELF, "X2_L_STOPCASTING") == 10)
    {
        nMagic = 0;
    }




    // **************
    // * JULY 12 2003
    // * Overriding "behavior" variables.
    // * If a variable has been stored on the creature it overrides the above
    // * class defaults
    // * JULY 28 2003
    // * changed this so that its an additive process, not an overrwrite.
    // * gives more flexiblity.
    // **************
    nMagic = nMagic + AdjustBehaviorVariable(nMagic, "X2_L_BEH_MAGIC");
    nOffense = nOffense + AdjustBehaviorVariable(nOffense, "X2_L_BEH_OFFENSE");
    nCompassion = nCompassion + AdjustBehaviorVariable(nCompassion, "X2_L_BEH_COMPASSION");


    // * If invisbile of any sort, become Defensive and
    // * magical to use any buffs you may have
    // * This behavior variable setting should override all others
    // * October 22 2003 - Lines 690 and 713 modified to only work if magic
    // * setting has not been turned off. Nathyrra always going invisible
    // * can be annoying.
    if (InvisibleTrue(OBJECT_SELF) == TRUE && nMagic > 0)
    {
        // SpawnScriptDebugger();
        // * if wounded at all take this time to heal self
        // * since I am invisible there is little danger from doing this
        if (GetCurrentHitPoints(OBJECT_SELF) < GetMaxHitPoints(OBJECT_SELF))
        {
            if(TalentHealingSelf(TRUE) == TRUE) return 99;
        }

        nOffense = 7;
        nMagic = 100;

        if (GetActionMode(OBJECT_SELF, ACTION_MODE_STEALTH) == TRUE)
        {
          nOffense = 100; // * if in stealth attempt sneak attacks
        }
    }
    else
    // **************
    // * JULY 14 2003
    // * Attempt To Go Invisible
    // **************
    if (InvisibleBecome() == TRUE && nMagic > 0)
        return 99;

    // PHYSICAL, NO OFFENSE
    if (nOffense <= 5)
    {
        //SpawnScriptDebugger();
        //SpeakString("fleeing");
        if (TalentFlee(oIntruder) == TRUE) return 99;
    }

    // protect others: MAGICAL, DEFENSE, COMPASSION
    if ((nOffense<= 50) && (nMagic > 50) && (nCompassion > 50))
    {
        SetLocalInt(OBJECT_SELF, "NW_L_MEMORY", chooseTactics_MEMORY_DEFENSE_OTHERS);
        if (TalentHeal() == TRUE) return 99;
        if (TalentCureCondition() == TRUE) return 99;
        if (TalentUseProtectionOthers() == TRUE) return 99;
        if (TalentEnhanceOthers() == TRUE) return 99;

        // * Temporarily be non-compassionate to buff self
        // * if we got to this point.
        nCompassion = 0;
    }

    // protectself: MAGICAL, DEFENSE, NO COMPASSION
    if ((nOffense<= 50) && (nMagic > 50) && (nCompassion <=50))
    {
        SetLocalInt(OBJECT_SELF, "NW_L_MEMORY", chooseTactics_MEMORY_DEFENSE_SELF);

        /* Dec 19 2002:
        Against spell-casters, cast protection spells more often
        */
        int nClass = GetClassByPosition(1,oIntruder);
        if (nClass == CLASS_TYPE_WIZARD || nClass == CLASS_TYPE_SORCERER
            || nClass == CLASS_TYPE_CLERIC || nClass == CLASS_TYPE_DRUID)
        {
            if (TalentSelfProtectionMantleOrGlobe())
                return 99;
        }

        if(TalentUseProtectionOnSelf() == TRUE) return 99;
        if(TalentUseEnhancementOnSelf() == TRUE) return 99;
        if(TalentPersistentAbilities() == TRUE) return 99;
        //    int TalentAdvancedBuff(float fDistance);

        //Used for Potions of Enhancement and Protection
        if(TalentBuffSelf() == TRUE) return 99;

        if(TalentAdvancedProtectSelf() == TRUE) return 99;
        if(TalentSummonAllies() == TRUE) return 99;
        if(TalentSeeInvisible() == TRUE) return 99;
        if(TalentMeleeAttacked(oIntruder) == TRUE) return 99;
        if(TalentRangedAttackers(oIntruder) == TRUE) return 99;
        if(TalentRangedEnemies(oIntruder) == TRUE) return 99;


    }

    //  MAGICAL, OFFENSE
    if (nMagic > 50)
    {
        // // MyPrintString("in offensive spell");
        // SpawnScriptDebugger();
        SetLocalInt(OBJECT_SELF, "NW_L_MEMORY", chooseTactics_MEMORY_OFFENSE_SPELL);
        if (TalentUseTurning() == TRUE) return 99;
        if (TalentSpellAttack(oIntruder) == TRUE) return 99;
    }

    // If we got here, we're going to melee offense
    SetLocalInt(OBJECT_SELF, "NW_L_MEMORY", chooseTactics_MEMORY_OFFENSE_MELEE);

    // PHYSICAL, OFFENSE (if nothing else applies)
    if (TryKiDamage(oIntruder) == TRUE) return 99;
    if (TalentSneakAttack() == TRUE) return 99;
    if (TalentDragonCombat(oIntruder)) {return 99;}
    if (TalentMeleeAttack(oIntruder) == TRUE) return 99;


    object oHostile = GetNearestSeenEnemy();

    // * Feb 17 2003: This error could happen in the situation that someone
    // * went into combat mode and their 'hostility' ended while going through ChooseTactics
    if (GetIsObjectValid(oHostile) == TRUE)
    {

        // * BK if it returns this it means the AI found nothing
        // * Appropriate to do
        //SpeakString("BUG!!!!!!!!!!!!!!!!!!!!!!!! (Let Brent Knowles know about this. Supply savegame) Nothing valid to do !!!!!!!!!!!!!!!!!!!!!");
        //SpeakString("BUG!! Magic " + IntToString(nMagic) + " Compassion " + IntToString(nCompassion) + " Offense " + IntToString(nOffense));
    }
    return 1;

} // * END of choosetactics






void newDetermineCombatRound(object oIntruder = OBJECT_INVALID, int nAI_Difficulty = 10)
{
    // MyPrintString("************** DETERMINE COMBAT ROUND START *************");
    // MyPrintString("**************  " + GetTag(OBJECT_SELF) + "  ************");

    // ----------------------------------------------------------------------------------------
    // May 2003
    // Abort out of here, if petrified
    // ----------------------------------------------------------------------------------------
    if (GetHasEffect(EFFECT_TYPE_PETRIFY, OBJECT_SELF) == TRUE)
    {
        return;
    }

    // ----------------------------------------------------------------------------------------
    // Oct 06/2003 - Georg Zoeller,
    // Fix for ActionRandomWalk blocking the action queue under certain circumstances
    // ----------------------------------------------------------------------------------------
    if (GetCurrentAction() == ACTION_RANDOMWALK)
    {
        ClearAllActions();
    }

    // ----------------------------------------------------------------------------------------
    // July 27/2003 - Georg Zoeller,
    // Added to allow a replacement for determine combat round
    // If a creature has a local string variable named X2_SPECIAL_COMBAT_AI_SCRIPT
    // set, the script name specified in the variable gets run instead
    // see x2_ai_behold for details:
    // ----------------------------------------------------------------------------------------
    string sSpecialAI = GetLocalString(OBJECT_SELF,"X2_SPECIAL_COMBAT_AI_SCRIPT");
    if (sSpecialAI != "")
    {
        SetLocalObject(OBJECT_SELF,"X2_NW_I0_GENERIC_INTRUDER", oIntruder);
        ExecuteScript(sSpecialAI, OBJECT_SELF);
        if (GetLocalInt(OBJECT_SELF,"X2_SPECIAL_COMBAT_AI_SCRIPT_OK"))
        {
            DeleteLocalInt(OBJECT_SELF,"X2_SPECIAL_COMBAT_AI_SCRIPT_OK");
            return;
        }
    }


    // ----------------------------------------------------------------------------------------
    // DetermineCombatRound: EVALUATIONS
    // ----------------------------------------------------------------------------------------
    if(GetAssociateState(NW_ASC_IS_BUSY))
    {
        return;
    }

    if(BashDoorCheck(oIntruder)) {return;}

    // ----------------------------------------------------------------------------------------
    // BK: stop fighting if something bizarre that shouldn't happen, happens
    // ----------------------------------------------------------------------------------------

    if (bkEvaluationSanityCheck(oIntruder, GetFollowDistance()) == TRUE)
        return;

    // ** Store HOw Difficult the combat is for this round
    int nDiff = GetCombatDifficulty();
    SetLocalInt(OBJECT_SELF, "NW_L_COMBATDIFF", nDiff);

    // MyPrintString("COMBAT: " + IntToString(nDiff));

    // ----------------------------------------------------------------------------------------
    // If no special target has been passed into the function
    // then choose an appropriate target
    // ----------------------------------------------------------------------------------------
    if (GetIsObjectValid(oIntruder) == FALSE)
        oIntruder = bkAcquireTarget();


    if (GetIsDead(oIntruder) == TRUE)
    {
        // ----------------------------------------------------------------------------------------
        // If for some reason my target is dead, then leave
        // the poor guy alone. Jeez. What kind of monster am I?
        // ----------------------------------------------------------------------------------------
        return;
    }

    // ----------------------------------------------------------------------------------------
    /*
       JULY 11 2003
       If in combat round already (variable set) do not enter it again.
       This is meant to prevent multiple calls to DetermineCombatRound
       from happening during the *same* round.

       This variable is turned on at the start of this function call.
       It is turned off at each "return" point for this function
       */
    // ----------------------------------------------------------------------------------------
    if (__InCombatRound() == TRUE)
    {
        return;
    }

    __TurnCombatRoundOn(TRUE);

    // ----------------------------------------------------------------------------------------
    // DetermineCombatRound: ACTIONS
    // ----------------------------------------------------------------------------------------
    if(GetIsObjectValid(oIntruder))
    {

        if(TalentPersistentAbilities()) // * Will put up things like Auras quickly
        {
            __TurnCombatRoundOn(FALSE);
            return;
        }

        // ----------------------------------------------------------------------------------------
        // BK September 2002
        // If a succesful tactic has been chosen then
        // exit this function directly
        // ----------------------------------------------------------------------------------------

        if (newChooseTactics(oIntruder) == 99)
        {
            __TurnCombatRoundOn(FALSE);
            return;
        }

        // ----------------------------------------------------------------------------------------
        // This check is to make sure that people do not drop out of
        // combat before they are supposed to.
        // ----------------------------------------------------------------------------------------

        object oNearEnemy = GetNearestSeenEnemy();
        DetermineCombatRound(oNearEnemy);

        return;
    }
     __TurnCombatRoundOn(FALSE);

    // ----------------------------------------------------------------------------------------
    // This is a call to the function which determines which
    // way point to go back to.
    // ----------------------------------------------------------------------------------------
    ClearActions(CLEAR_NW_I0_GENERIC_658);
    SetLocalObject(OBJECT_SELF,
                   "NW_GENERIC_LAST_ATTACK_TARGET",
                   OBJECT_INVALID);
    WalkWayPoints();
}