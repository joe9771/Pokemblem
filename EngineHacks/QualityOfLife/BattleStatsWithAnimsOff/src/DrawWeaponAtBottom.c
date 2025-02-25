#include "gbafe.h"

extern u16 gBG0MapBuffer[32][32]; // 0x02022CA8.
static void PrepareText(TextHandle* handle, char* string);
void DrawWeaponAtBottom(BattleUnit* unit, u8 xx, u8 yy); 

extern const u16* gArrows;
extern void RegisterObjectTileGraphics(const void* source, void* target, int width, int height); // 8012FF4

extern u32 UseWeaponTriangleForArrows; 

// PushToHiOAM(u16 xBase, u16 yBase, const struct ObjData* data, u16 tileBase);
// void RegisterTileGraphics(const void* source, void* target, unsigned size); //! FE8U = 0x8002015
// extern const struct ObjData gObj_16x16;

// BattleGetFollowUpOrder, 0x802AF91
extern bool BattleGetFollowUpOrder(BattleUnit* unit); 

void DrawWeaponAtBottom(BattleUnit* unit, u8 xx, u8 yy) { 
// Text_ResetTileAllocation(); 
	u8 y_offset = 6; 
	u8 x_offset = 1; 
	
	struct BattleUnit* opponent; 
	if (&gBattleActor == unit) { opponent = &gBattleTarget; } 
	if (&gBattleTarget == unit) { opponent = &gBattleActor; } 
	
	u8 iconY = (y_offset + yy); 
	u8 iconX = (x_offset + xx); 
	if (unit->weaponBefore) { 
		DrawIcon(&gBG0MapBuffer[iconY][iconX],GetItemIconId((unit->weaponBefore)),TILEREF(0, 0x4));

		LoadIconPalette(0, 0x4); // 0 is the item icon palette 
		EnablePaletteSync(); 
		
		TextHandle handles[1];
		PrepareText(&handles[0], GetItemName(unit->weaponBefore)); // my function 
		Text_Display(&handles[0], &gBG0MapBuffer[yy+6][xx+3]); // vanilla function 
		u16 tileStart = 0x4A; // edit for various arrows 
		RegisterTileGraphics(gArrows, (void*)(0x6000000 | (tileStart*0x20)), 256);
		SyncRegisteredTiles();
		u8 xDimension = 1; 
		u8 yDimension = 2; 
		u8 xImageSize = 4; 
		
		Unit* unitA = unit; 
		Unit* opponentA = opponent; 
		
		//asm("mov r11, r11"); 
		//bool p1 = (1 + BattleGetFollowUpOrder(unitA));
		//bool p2 = (1 + BattleGetFollowUpOrder(opponentA));
		int unitHp = (((unit->hpInitial - (opponent->battleAttack - unit->battleDefense)) * 100 / opponent->battleEffectiveHitRate) * 100 / unitA->maxHP ); 
		//asm("mov r11, r11");
		int opponentHp = (((opponent->hpInitial - (unit->battleAttack - opponent->battleDefense)) * 100 / unit->battleEffectiveHitRate) * 100 / opponentA->maxHP ); 
		//asm("mov r11, r11");
		int advantage = ((unitHp > opponentHp) && (unit->canCounter)) || (!opponent->canCounter); 
		
		// pokemblem type effectiveness 
		int effective = IsItemEffectiveAgainst(unit->weaponBefore, opponent);
		advantage = effective; 
		bool disadvantage = ((effective == 7) | (effective == 2) | (effective == 1));
		if (disadvantage) { advantage = -1; } 
		
		if (advantage == 9) { advantage = 0; } 


		if (UseWeaponTriangleForArrows) { advantage = (unit->wTriangleDmgBonus > opponent->wTriangleDmgBonus); }  // for others 
		
		if (!UseWeaponTriangleForArrows || (UseWeaponTriangleForArrows && (unit->wTriangleDmgBonus != 0))) { 
			if (advantage)  { 
				for (u8 y = 0; y < yDimension; y++) { 
					for (u8 x = 0; x < xDimension; x++) { 
						gBG0MapBuffer[y+yy+6][x+xx+10] = tileStart+1 + x + (xDimension * xImageSize * y); 
					}
				} 
			
			} 
			if (advantage < 0) { 
				for (u8 y = 0; y < yDimension; y++) { 
					for (u8 x = 0; x < xDimension; x++) { 
						gBG0MapBuffer[y+yy+6][x+xx+10] = tileStart+3 + x + (xDimension * xImageSize * y); 
					}
				} 
			} 
		} 
		EnableBgSyncByIndex(0); 
	} 
} 


void PrepareText(TextHandle* handle, char* string)
{
	u32 width = (Text_GetStringTextWidth(string)+8)/8;
	Text_InitClear(handle, width); 
    handle->tileWidth = width;
	
	Text_SetColorId(handle,TEXT_COLOR_NORMAL);
	Text_DrawString(handle,string);
	//Text_Display(&handle,&gBG0MapBuffer[y][x]);
}

