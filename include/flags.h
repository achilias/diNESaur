#include <cstdint>

#define PPUCTRL_NT(ppuctrl)                         ((ppuctrl) & 0b11)
#define PPUCTRL_SET_NT(ppuctrl, val)                {ppuctrl = ((ppuctrl) & ~0b11) | (val) & 0b11;}

#define PPUCTRL_INCR(ppuctrl)                       ((bool) ((ppuctrl) & 0b100))
#define PPUCTRL_SET_INCR(ppuctrl, val)              {ppuctrl = ((ppuctrl) & ~0b100) | ((uint8_t)val) << 2;}

#define PPUCTRL_SPRITE_PT(ppuctrl)                  (((ppuctrl) & 0b1000) >> 3)
#define PPUCTRL_SET_SPRITE_PT(ppuctrl, val)         {ppuctrl = ((ppuctrl) & ~0b1000) | ((uint8_t)val) << 3;}

#define PPUCTRL_BG_PT(ppuctrl)                      (((ppuctrl) & 0b10000) >> 4)
#define PPUCTRL_SET_BG_PT(ppuctrl, val)             {ppuctrl = ((ppuctrl) & ~0b10000) | ((uint8_t)val) << 4;}

#define PPUCTRL_SPRITE_SZ(ppuctrl)                  ((bool) ((ppuctrl) & 0b100000))
#define PPUCTRL_SET_SPRITE_SZ(ppuctrl, val)         {ppuctrl = ((ppuctrl) & ~0b100000) | ((uint8_t)val) << 5;}

#define PPUCTRL_MASTER_SLAVE(ppuctrl)               ((bool) ((ppuctrl) & 0b1000000))
#define PPUCTRL_SET_MASTER_SLAVE(ppuctrl, val)      {ppuctrl = ((ppuctrl) & ~0b1000000) | ((uint8_t)val) << 6;}

#define PPUCTRL_VBLANK_ENABLE(ppuctrl)              ((bool) ((ppuctrl) & 0b10000000))
#define PPUCTRL_SET_VBLANK_ENABLE(ppuctrl, val)     {ppuctrl = ((ppuctrl) & ~0b10000000) | ((uint8_t)val) << 7;}

#define PPUSTATUS_SPR_OVF(ppustatus)                ((bool) ((ppustatus) & 0b100000))
#define PPUSTATUS_SET_SPR_OVF(ppustatus, val)       {ppustatus = ((ppustatus) & ~0b100000) | ((uint8_t)val) << 5;}

#define PPUSTATUS_SPR0_HIT(ppustatus)               ((bool) ((ppustatus) & 0b1000000))
#define PPUSTATUS_SET_SPR0_HIT(ppustatus, val)      {ppustatus = ((ppustatus) & ~0b1000000) | ((uint8_t)val) << 6;}

#define PPUSTATUS_VBLANK(ppustatus)                 ((bool) ((ppustatus) & 0b10000000))
#define PPUSTATUS_SET_VBLANK(ppustatus, val)        {ppustatus = ((ppustatus) & ~0b10000000) | ((uint8_t)val) << 7;}

#define PPUMASK_GRAYSCALE(ppumask)                  ((ppumask) & 0b1)
#define PPUMASK_SET_GRAYSCALE(ppumask, val)         {ppumask = ((ppumask) & ~0b1) | (val) & 0b1;}

#define PPUMASK_BG_LEFT(ppumask)                    ((bool) ((ppumask) & 0b10))
#define PPUMASK_SET_BG_LEFT(ppumask, val)           {ppumask = ((ppumask) & ~0b10) | ((uint8_t)val) << 1;}

#define PPUMASK_SPR_LEFT(ppumask)                   (((ppumask) & 0b100) >> 3)
#define PPUMASK_SET_SPR_LEFT(ppumask, val)          {ppumask = ((ppumask) & ~0b100) | ((uint8_t)val) << 2;}

#define PPUMASK_BG_RENDER(ppumask)                  (((ppumask) & 0b1000) >> 4)
#define PPUMASK_SET_BG_RENDER(ppumask, val)         {ppumask = ((ppumask) & ~0b1000) | ((uint8_t)val) << 3;}

#define PPUMASK_SPR_RENDER(ppumask)                 ((bool) ((ppumask) & 0b10000))
#define PPUMASK_SET_SPR_RENDER(ppumask, val)        {ppumask = ((ppumask) & ~0b10000) | ((uint8_t)val) << 4;}

#define PPUMASK_EMPHASIZE_RED(ppumask)              ((bool) ((ppumask) & 0b100000))
#define PPUMASK_SET_EMPHASIZE_RED(ppumask, val)     {ppumask = ((ppumask) & ~0b100000) | ((uint8_t)val) << 5;}

#define PPUMASK_EMPHASIZE_GREEN(ppumask)            ((bool) ((ppumask) & 0b1000000))
#define PPUMASK_SET_EMPHASIZE_GREEN(ppumask, val)   {ppumask = ((ppumask) & ~0b1000000) | ((uint8_t)val) << 6;}

#define PPUMASK_EMPHASIZE_BLUE(ppumask)             ((bool) ((ppumask) & 0b10000000))
#define PPUMASK_SET_EMPHASIZE_BLUE(ppumask, val)    {ppumask = ((ppumask) & ~0b10000000) | ((uint8_t)val) << 7;}