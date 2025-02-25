.thumb 
.macro blh to, reg=r3
  ldr \reg, =\to
  mov lr, \reg
  .short 0xf800
.endm

.equ GetItemWType, 0x8017548 

.type SortSupply, %function
.global SortSupply
SortSupply: 

STR r2,[SP, #0x0]
push {r4, lr}
mov r4, r0 @ weapon id  
blh GetItemWType
LSL r0 ,r0 ,#28
LSR r0 ,r0 ,#28

mov r1, r4 
bl GetAdjustedWepTypeForSupply



pop {r4} 
pop {r3}
LDR r2,[SP, #0x0]
bx r3 
.ltorg 

.type SortSupply2, %function
.global SortSupply2 
SortSupply2: 
push {lr}
ADD r5 ,r0, R1
LDRH r0, [r5, #0x2] @# pointer:020122D6
blh GetItemWType 
LSL r0 ,r0 ,#28
LSR r0 ,r0 ,#28
LDRH r1, [r5, #0x2] @# pointer:020122D6
bl GetAdjustedWepTypeForSupply 


pop {r3} 
bx r3 
.ltorg 

.type SortSupply3, %function 
.global SortSupply3 

SortSupply3:
push {lr} 
mov r0, r6 
blh GetItemWType 
mov r1, r6 
bl GetAdjustedWepTypeForSupply 
ldrb r1, [r4] 
pop {r3}
ldr r3, =0x8098025 
bx r3 
.ltorg 


GetAdjustedWepTypeForSupply:
push {r4, lr} 
@ r0 as current weapon type 
@ r1 as item ID 

mov r2, #0xFF 
and r1, r2

cmp r1, #0xFB @ stat equipment 
bne NotStatEquipment 
mov r0, #1 
b Exit 
NotStatEquipment: 
cmp r1, #0xF6 
beq SkillEquipment 
cmp r1, #0xFC 
blt NotSkillEquipment 
SkillEquipment: 
mov r0, #2 
b Exit 
NotSkillEquipment: 
cmp r1, #0xF8 
bne NotSpellScroll 
mov r0, #3 
b Exit 
NotSpellScroll: 
cmp r1, #0xF7 @ skill scroll 
bne NotSkillScroll
mov r0, #4 @ skill scrolls to appear 4th 
NotSkillScroll: 
cmp r1, #0x6C @ elixir 
blt NotHealing 
cmp r1, #0x6D 
bgt NotHeal1 
mov r0, #0 
b Exit 
NotHeal1: 
cmp r1, #0xA2 
blt NotHealing 
cmp r1, #0xA4 
bgt NotHealing 
mov r0, #0 @ Potion, Berry, JellyDonut, HyperPotion, MaxPotion 
NotHealing: 
mov r3, #0 
ldr r4, =PromotionItemsList
PromoLoop: 
ldrb r2, [r4, r3] 
add r3, #1 
cmp r2, #0 
beq BreakPromoLoop 
cmp r1, r2 
beq PromoItem 
b PromoLoop 
PromoItem: 
mov r0, #6 @ column 6 for promo items 
b Exit 
BreakPromoLoop: 
mov r3, #0 
ldr r4, =StatBoostersItemList
StatBoosterLoop: 
ldrb r2, [r4, r3] 
add r3, #1 
cmp r2, #0 
beq BreakStatBoostersLoop 
cmp r1, r2 
beq StatBoosterItem 
b StatBoosterLoop 
StatBoosterItem: 
mov r0, #5 @ column 6 for promo items 
b Exit 
BreakStatBoostersLoop: 


Exit: 

pop {r4} 
pop {r1} 
bx r1 
.ltorg 
