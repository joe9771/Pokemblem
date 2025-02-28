#include "gbafe.h"
extern int SkillTester(struct Unit* unit, int SkillID); 
extern int ScaledUpID_Link; 
extern int Intimidate2ID_Link; 
extern int GuardBreakerID_Link; 
extern int DowngradeID_Link; 
extern int SuperFangID_Link; 

extern int MegaKick_Link;
extern int DoubleKick_Link; 
extern int HiJumpKick_Link;
extern int JumpKick_Link;
extern int LowKick_Link;
extern int RollingKick_Link;
extern int Submission_Link;
extern int GrassKnot_Link;

void RemoveAllMineTraps(void) { 
	struct Trap* trap; 
	for (int i = 0; i<=50; i++) { 
		trap = GetTrap(i); 
		if (!trap) { break; } 
		if (trap->type == TRAP_MINE) { 
			RemoveTrap(trap); 
			i--; // must start again from previous slot as RemoveTrap also moves all traps up to leave no blank spots 
		} 
	} 
} 

void SuperFangEffect(struct BattleUnit* bunitA, struct BattleUnit* bunitB) {
	if (gBattleStats.config & (BATTLE_CONFIG_REAL | BATTLE_CONFIG_SIMULATE)) {
		if ((bunitA->weaponBefore & 0xFF) == SuperFangID_Link) { 
			int hp = bunitB->hpInitial; 
			if (hp) { 
				bunitA->battleAttack = (hp+3)/4; 
				bunitB->battleDefense = 0; 
			} 
			else { bunitA->battleAttack = 0; } 
		}
	}
} 

// deal 20% more dmg 
void ScaledUp(struct BattleUnit* bunitA, struct BattleUnit* bunitB) { 
	if (SkillTester(&bunitB->unit, ScaledUpID_Link)) { 
		if (gBattleTarget.battleDefense) { // if def isn't calculated yet, do nothing 
			int dmg = bunitB->battleAttack - bunitA->battleDefense; 
			if (dmg < 0) dmg = 0; 
			int addDmg = (dmg+2)/5; // for rounding 
			bunitB->battleAttack += addDmg; 
		
		} 
	} 
} 



void EnemyWeightBonus(struct BattleUnit* bunitA, struct BattleUnit* bunitB) { 
	if (gBattleStats.config & (BATTLE_CONFIG_REAL | BATTLE_CONFIG_SIMULATE)) {
		if (!(bunitA->weaponAttributes & IA_MAGIC)) { 
		int item = bunitA->weaponBefore & 0xFF; 
			if ((item == MegaKick_Link) || (item == DoubleKick_Link) || (item == HiJumpKick_Link) || (item == JumpKick_Link) || (item == LowKick_Link) || (item == RollingKick_Link) || (item == Submission_Link) || (item == GrassKnot_Link)) { 
			bunitA->battleAttack += (bunitB->unit.pClassData->baseCon / 3); 
			} 
		}
	} 
} 



//extern IntimidateDebuffs[]; 
struct weaponDebuffTableStruct { 
	u8 mag;
	u8 str;
	u8 skl;
	u8 spd;
	u8 def;
	u8 res;
	u8 luk;
	u8 mov;
};
extern struct weaponDebuffTableStruct IntimidateDebuffs; 
extern u8 DebuffStatNumberOfBits_Link;
extern u32* GetUnitDebuffEntry(struct Unit*); 
extern int DebuffStatBitOffset_Str; 
extern int DebuffStatBitOffset_Mag; 
extern int DebuffStatBitOffset_Def; 
extern int DebuffStatBitOffset_Res; 
extern int DebuffStatBitOffset_Spd; 
extern int UnpackData_Signed(u32* debuffEntryRam, u8 bitOffset, u8 bitCount); 
extern void DebuffGivenTableEntry(u32*, struct weaponDebuffTableStruct*, int, u32*); 

// Enemy Str/Mag is reduced by 25% when attacking. 
void Intimidate_Prebattle(struct BattleUnit* bunitA, struct BattleUnit* bunitB) { 
	if (gBattleStats.config & (BATTLE_CONFIG_REAL | BATTLE_CONFIG_SIMULATE)) {
		
		if (SkillTester(&bunitB->unit, Intimidate2ID_Link)) { 
		
			void* debuffRam = GetUnitDebuffEntry(&bunitA->unit); 
			int str = bunitA->unit.pow; 
			int subStr = (str+2)/4; 
			int mag = bunitA->unit.unk3A;
			int subMag = (mag+2)/4; 
			
			if (!(bunitA->weaponAttributes & IA_MAGIC)) { // melee 
				if (UnpackData_Signed(debuffRam, DebuffStatBitOffset_Str, DebuffStatNumberOfBits_Link) >= 0) { 
					// if enemy is not already debuffed, do stuff 
					bunitA->battleAttack -= subStr; 
					if (bunitA->battleAttack < 1) { bunitA->battleAttack = 1; } 
				} 
			}
			else { // magic 
				if (UnpackData_Signed(debuffRam, DebuffStatBitOffset_Mag, DebuffStatNumberOfBits_Link) >= 0) { 
					// if enemy is not already debuffed, do stuff 
					bunitA->battleAttack -= subMag; 
					if (bunitA->battleAttack < 1) { bunitA->battleAttack = 1; } 
				} 
			
			} 
			if (gBattleStats.config & BATTLE_CONFIG_REAL) { 
				//asm("mov r11, r11"); 
				struct weaponDebuffTableStruct tempDebuffTable; 
				tempDebuffTable.mag = subMag|0x80; 
				tempDebuffTable.str = subStr|0x80; //((str+2)/4)|0x80;  
				tempDebuffTable.skl = 0; 
				tempDebuffTable.spd = 0; 
				tempDebuffTable.def = 0; 
				tempDebuffTable.res = 0; 
				tempDebuffTable.luk = 0; 
				tempDebuffTable.mov = 0; 
				//DebuffGivenTableEntry(debuffRam, IntimidateDebuffs, 0); 
				DebuffGivenTableEntry(debuffRam, &tempDebuffTable, 0, debuffRam); 
				// apply some debuff here ? or make it always happen post-battle 
			} 
		} 
	} 
} 

// Enemy Def/Res is reduced by 25% when attacking. 
void GuardBreaker_Prebattle(struct BattleUnit* bunitA, struct BattleUnit* bunitB) { 
	if (gBattleStats.config & (BATTLE_CONFIG_REAL | BATTLE_CONFIG_SIMULATE)) {
		
		if (SkillTester(&bunitB->unit, GuardBreakerID_Link)) { 
		
			void* debuffRam = GetUnitDebuffEntry(&bunitA->unit); 
			int def = bunitA->unit.def; 
			int subDef = (def+2)/4; 
			int res = bunitA->unit.res;
			int subRes = (res+2)/4; 
			
			if (!(bunitB->weaponAttributes & IA_MAGIC)) { // melee 
				if (UnpackData_Signed(debuffRam, DebuffStatBitOffset_Def, DebuffStatNumberOfBits_Link) >= 0) { 
					// if enemy is not already debuffed, do stuff 
					bunitA->battleDefense -= subDef; 
					if (bunitA->battleDefense < 1) { bunitA->battleDefense = 1; } 
				} 
			}
			else { // magic 
				if (UnpackData_Signed(debuffRam, DebuffStatBitOffset_Res, DebuffStatNumberOfBits_Link) >= 0) { 
					// if enemy is not already debuffed, do stuff 
					bunitA->battleDefense -= subRes; 
					if (bunitA->battleDefense < 1) { bunitA->battleDefense = 1; } 
				} 
			
			} 
			
			if (gBattleStats.config & BATTLE_CONFIG_REAL) { 
				//asm("mov r11, r11"); 
				struct weaponDebuffTableStruct tempDebuffTable; 
				tempDebuffTable.mag = 0; 
				tempDebuffTable.str = 0; 
				tempDebuffTable.skl = 0; 
				tempDebuffTable.spd = 0; 
				tempDebuffTable.def = subDef|0x80; 
				tempDebuffTable.res = subRes|0x80; 
				tempDebuffTable.luk = 0; 
				tempDebuffTable.mov = 0; 
				//DebuffGivenTableEntry(debuffRam, IntimidateDebuffs, 0); 
				DebuffGivenTableEntry(debuffRam, &tempDebuffTable, 0, debuffRam); 
				// apply some debuff here ? or make it always happen post-battle 
			} 
		} 
	} 
} 

// Enemy Spd is reduced by 25% when attacking. 
void Downgrade_Prebattle(struct BattleUnit* bunitA, struct BattleUnit* bunitB) { 
	if (gBattleStats.config & (BATTLE_CONFIG_REAL | BATTLE_CONFIG_SIMULATE)) {
		
		if (SkillTester(&bunitB->unit, DowngradeID_Link)) { 
		
			void* debuffRam = GetUnitDebuffEntry(&bunitA->unit); 
			int spd = bunitA->unit.spd; 
			int subSpd = (spd+2)/4; 
			
			if (UnpackData_Signed(debuffRam, DebuffStatBitOffset_Spd, DebuffStatNumberOfBits_Link) >= 0) { 
				// if enemy is not already debuffed, do stuff 
				bunitA->battleSpeed -= subSpd; 
				if (bunitA->battleSpeed < 0) { bunitA->battleSpeed = 0; } 
			} 
			

			
			if (gBattleStats.config & BATTLE_CONFIG_REAL) { 
				//asm("mov r11, r11"); 
				struct weaponDebuffTableStruct tempDebuffTable; 
				tempDebuffTable.mag = 0; 
				tempDebuffTable.str = 0; 
				tempDebuffTable.skl = 0; 
				tempDebuffTable.spd = subSpd|0x80; 
				tempDebuffTable.def = 0; 
				tempDebuffTable.res = 0; 
				tempDebuffTable.luk = 0; 
				tempDebuffTable.mov = 0; 
				//DebuffGivenTableEntry(debuffRam, IntimidateDebuffs, 0); 
				DebuffGivenTableEntry(debuffRam, &tempDebuffTable, 0, debuffRam); 
				// apply some debuff here ? or make it always happen post-battle 
			} 
		} 
	} 
} 

extern int DittoID_Link;
extern int Transform_Link; 

void TransformInit(struct BattleUnit* bunitA, struct BattleUnit* bunitB) { 
	//if (gBattleStats.config & (BATTLE_CONFIG_REAL | BATTLE_CONFIG_SIMULATE)) {
	//	return; 
	//}
	if (bunitA->unit.pClassData) { 
		if ((bunitA->unit.pClassData->number == DittoID_Link) || (bunitA->weaponBefore == Transform_Link)) { 
			bunitA->battleAttack = 0; 
			bunitA->battleDefense = 0; 
			bunitA->battleSpeed = 0; 
			bunitA->battleHitRate = 0; 
			bunitA->battleAvoidRate = 0; 
			bunitA->battleCritRate = 0; 
		}
	}
	if (bunitB->unit.pClassData) { 
		if ((bunitB->unit.pClassData->number == DittoID_Link) || (bunitB->weaponBefore == Transform_Link)) { 
			bunitB->battleAttack = 0; 
			bunitB->battleDefense = 0; 
			bunitB->battleSpeed = 0; 
			bunitB->battleHitRate = 0; 
			bunitB->battleAvoidRate = 0; 
			bunitB->battleCritRate = 0; 
		}
	} 
}
			

void TransformFunction(struct BattleUnit* bunitA, struct BattleUnit* bunitB) { 
	if (gBattleStats.config & (BATTLE_CONFIG_REAL | BATTLE_CONFIG_SIMULATE)) {
		if ((bunitB->unit.pClassData->number == DittoID_Link) || (bunitB->weaponBefore == Transform_Link)) { 
			//bunitA->battleAttack = (bunitB->battleAttack - bunitB->battleDefense) ? : 0; 
			//bunitA->battleDefense = bunitB->battleDefense; 
			//bunitA->battleSpeed = bunitB->battleSpeed; 
			//bunitA->battleHitRate = (bunitB->battleHitRate - bunitB->battleAvoidRate) ? : 0; 
			//bunitA->battleAvoidRate = bunitB->battleAvoidRate; 
			//bunitA->battleCritRate = (bunitB->battleCritRate- bunitB->battleDodgeRate) ? : 0; 
			
			bunitB->battleAttack = bunitA->battleAttack; 
			bunitB->battleDefense = bunitA->battleDefense; 
			bunitB->battleSpeed = bunitA->battleSpeed; 
			//bunitB->battleHitRate = bunitA->battleHitRate; 
			//bunitB->battleAvoidRate = bunitA->battleAvoidRate; 
			//bunitB->battleCritRate = bunitA->battleCritRate; 
			bunitB->battleEffectiveCritRate = bunitA->battleEffectiveCritRate; 
			bunitB->battleEffectiveHitRate = bunitA->battleEffectiveHitRate; 
			
			//if (bunitB->battleAttack > 99) bunitB->battleAttack = 99;
			//if (bunitB->battleDefense > 99) bunitB->battleDefense = 99; 
			//if (bunitB->battleSpeed > 99) bunitB->battleSpeed = 99; 
			//if (bunitB->battleHitRate > 250) bunitB->battleHitRate = 250; 
			//if (bunitB->battleAvoidRate > 250) bunitB->battleAvoidRate = 250; 
			//if (bunitB->battleCritRate > 250) bunitB->battleCritRate = 250; 
			//if (bunitB->battleEffectiveCritRate > 100) bunitB->battleEffectiveCritRate = 100; 
			//if (bunitB->battleEffectiveHitRate > 100) bunitB->battleHitRate = 100; 
			
		}
		if ((bunitA->unit.pClassData->number == DittoID_Link) || (bunitA->weaponBefore == Transform_Link)) { 
			bunitA->battleAttack = bunitB->battleAttack; 
			bunitA->battleDefense = bunitB->battleDefense; 
			bunitA->battleSpeed = bunitB->battleSpeed; 
			//bunitA->battleHitRate = bunitB->battleHitRate; 
			//bunitA->battleAvoidRate = bunitB->battleAvoidRate; 
			//bunitA->battleCritRate = bunitB->battleCritRate; 
			bunitA->battleEffectiveCritRate = bunitB->battleEffectiveCritRate; 
			bunitA->battleEffectiveHitRate = bunitB->battleEffectiveHitRate; 
		}
	}
}


