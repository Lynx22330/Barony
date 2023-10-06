/*-------------------------------------------------------------------------------

	BARONY
	File: monster_rat.cpp
	Desc: implements all of the rat monster's code

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "entity.hpp"
#include "monster.hpp"
#include "engine/audio/sound.hpp"
#include "items.hpp"
#include "net.hpp"
#include "collision.hpp"
#include "prng.hpp"

void initRat(Entity* my, Stat* myStats)
{
	my->flags[BURNABLE] = true;
	my->flags[UPDATENEEDED] = true;
	my->flags[INVISIBLE] = false;

	my->initMonster(131);

	if ( multiplayer != CLIENT )
	{
		MONSTER_SPOTSND = 29;
		MONSTER_SPOTVAR = 1;
		MONSTER_IDLESND = 29;
		MONSTER_IDLEVAR = 1;
	}
	if ( multiplayer != CLIENT && !MONSTER_INIT )
	{
		if ( myStats != NULL )
		{
			if ( !myStats->leader_uid )
			{
				myStats->leader_uid = 0;
			}

			// apply random stat increases if set in stat_shared.cpp or editor
			setRandomMonsterStats(myStats);

			// generate 6 items max, less if there are any forced items from boss variants
			int customItemsToGenerate = ITEM_CUSTOM_SLOT_LIMIT;

			// boss variants
			const bool boss =
			    local_rng.rand() % 50 == 0 &&
			    !my->flags[USERFLAG2] &&
			    !myStats->MISC_FLAGS[STAT_FLAG_DISABLE_MINIBOSS];
			if ( (boss || *cvar_summonBosses) && myStats->leader_uid == 0 )
			{
				myStats->setAttribute("special_npc", "algernon");
				strcpy(myStats->name, MonsterData_t::getSpecialNPCName(*myStats).c_str());
	            my->sprite = MonsterData_t::getSpecialNPCBaseModel(*myStats);
				myStats->HP = 120;
				myStats->MAXHP = 120;
				myStats->OLDHP = myStats->HP;
				myStats->STR = -1;
				myStats->DEX = 20;
				myStats->CON = 2;
				myStats->INT = 20;
				myStats->PER = -2;
				myStats->CHR = 5;
				myStats->LVL = 10;
				newItem(GEM_EMERALD, static_cast<Status>(1 + local_rng.rand() % 4), 0, 1, local_rng.rand(), true, &myStats->inventory);
				customItemsToGenerate = customItemsToGenerate - 1;
				int c;
				for ( c = 0; c < 6; c++ )
				{
					Entity* entity = summonMonster(RAT, my->x, my->y);
					if ( entity )
					{
						entity->parent = my->getUID();
						if ( Stat* followerStats = entity->getStats() )
						{
							followerStats->leader_uid = entity->parent;
						}
					}
				}
			}
			// random effects

			// generates equipment and weapons if available from editor
			createMonsterEquipment(myStats);

			// create any custom inventory items from editor if available
			createCustomInventory(myStats, customItemsToGenerate);

			// count if any custom inventory items from editor
			int customItems = countCustomItems(myStats); //max limit of 6 custom items per entity.

														 // count any inventory items set to default in edtior
			int defaultItems = countDefaultItems(myStats);

			my->setHardcoreStats(*myStats);

			// generate the default inventory items for the monster, provided the editor sprite allowed enough default slots
			switch ( defaultItems )
			{
				case 6:
				case 5:
				case 4:
				case 3:
				case 2:
				case 1:
					if ( local_rng.rand() % 4 )
					{
						if ( local_rng.rand() % 2 )
						{
							newItem(FOOD_MEAT, EXCELLENT, 0, 1, local_rng.rand(), false, &myStats->inventory);
						}
						else
						{
							newItem(FOOD_CHEESE, DECREPIT, 0, 1, local_rng.rand(), false, &myStats->inventory);
						}
					}
					break;
				default:
					break;
			}
		}
	}
	if ( multiplayer != CLIENT && myStats )
	{
		if (myStats->getAttribute("special_npc") == "algernon")
		{
			my->z -= 1; // algernon is slightly larger than an ordinary rat.
		}
	}
	myStats->EFFECTS[EFF_LEVITATING] = true;
	myStats->EFFECTS_TIMERS[EFF_LEVITATING] = 0;
}

float myZ = 0;
int bobCount = 0;

void ratAnimate(Entity* my, double dist)
{
    if (my->ticks == my->getUID() % TICKS_PER_SECOND) {
        if (multiplayer == SERVER) {
            // in case we are algernon, update the sprite for clients
            serverUpdateEntitySprite(my);
        }
    }

    float myZOffset = -6.0 + sin(my->ticks * 0.125) * 2;
	bobCount = (bobCount + 1) % 6000;

    // walk cycle
    if (dist >= 0.1 && !MONSTER_ATTACK) {
        if (my->ticks % 10 == 0)
        {
            // normal rat walk cycle
            if ( my->sprite == 1238 ) {
                my->sprite = 1238;
            } else if (my->sprite == 1238) {
                my->sprite = 1238;
            }

            // algernon walk cycle
            if ( my->sprite == 1238) {
                my->sprite = 1238;
            } else if (my->sprite == 1238) {
                my->sprite = 1238;
            }
        }
    }

    static ConsoleVariable<bool> cvar_useFocalZ("/rat_anim_use_focal_z", false);

    // attack cycle
    if (MONSTER_ATTACK) {
        const int frame = TICKS_PER_SECOND / 10;
        const bool algernon = my->sprite >= 1068;
        if (MONSTER_ATTACKTIME == frame * 0) { // frame 1
            my->sprite = algernon ? 1070 : 1238;
            if (*cvar_useFocalZ) {
                my->focalz = -1.5;
            } else {
                myZ = 4.5;
            }
        }
        if (MONSTER_ATTACKTIME == frame * 1) { // frame 2
            my->sprite = algernon ? 1071 : 1238;
            if (*cvar_useFocalZ) {
                my->focalz = -2.5;
            } else {
                myZ = 3.5;
            }
        }
        if (MONSTER_ATTACKTIME == frame * 2) { // frame 3
            my->sprite = algernon ? 1072 : 1238;
            if (*cvar_useFocalZ) {
                my->focalz = -3.5;
            } else {
                myZ = 2.5;
            }
        }
        if (MONSTER_ATTACKTIME == frame * 4) { // frame 4
            my->sprite = algernon ? 1073 : 1238;
            if (*cvar_useFocalZ) {
                my->focalz = -4;
            } else {
                myZ = 2;
            }
            const Sint32 temp = MONSTER_ATTACKTIME;
            my->attack(1, 0, nullptr); // munch
            MONSTER_ATTACKTIME = temp;
        }
        if (MONSTER_ATTACKTIME == frame * 6) { // frame 5
            my->sprite = algernon ? 1074 : 1238;
            if (*cvar_useFocalZ) {
                my->focalz = -3;
            } else {
                myZ = 3;
            }
        }
        if (MONSTER_ATTACKTIME == frame * 7) { // end
            if (algernon) {
                my->sprite = 1068;
                myZ = 5.5;
            } else {
                my->sprite = 1238;
                myZ = 6;
            }
            my->focalz = 0;
            MONSTER_ATTACK = 0;
            MONSTER_ATTACKTIME = 0;
        }
        else {
            ++MONSTER_ATTACKTIME;
            my->new_z = myZ;
        }
    }
    my->z = myZ + myZOffset;
}

	void ratDie(Entity * my)
{
	Entity* gib = spawnGib(my);
	gib->skill[5] = 1; // poof
	gib->sprite = my->sprite;
	gib->pitch = 0.0;
	serverSpawnGibForClient(gib);
	for ( int c = 0; c < 5; c++ )
	{
		Entity* gib = spawnGib(my);
		serverSpawnGibForClient(gib);
	}

	my->spawnBlood();

	playSoundEntity(my, 30, 64);

	my->removeMonsterDeathNodes();

	list_RemoveNode(my->mynode);
	return;
}
