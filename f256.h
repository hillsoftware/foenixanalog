#define word uint16_t

//VS1053b serial bus
#define VS_SCI_CTRL  0xD700
#define VS_SCI_ADDR  0xD701
#define VS_SCI_DATA  0xD702   //2 bytes
#define VS_FIFO_STAT 0xD704   //2 bytes
#define VS_FIFO_DATA 0xD707

typedef enum {
    PAGE_0 = 0,
    PAGE_1 = 1
} io_page_t;

#define SET_IO_PAGE(page)  POKE(MMU_IO_CTRL, (uint8_t)(page))

// Base addresses for each bitmap's address registers
#define VKY_BM_ADDR_L(bm)  (0xD101 + (bm) * 8)  // low byte register
#define VKY_BM_ADDR_M(bm)  (0xD102 + (bm) * 8)  // mid byte register
#define VKY_BM_ADDR_H(bm)  (0xD103 + (bm) * 8)  // high byte register

// --- Master Control Register 0 (0xD000) bit definitions ---
#define VKY_MC0_GAMMA    (1 << 6)  // Enable gamma correction
#define VKY_MC0_SPRITE   (1 << 5)  // Enable sprites
#define VKY_MC0_TILE     (1 << 4)  // Enable tiles
#define VKY_MC0_BITMAP   (1 << 3)  // Enable bitmap mode
#define VKY_MC0_GRAPH    (1 << 2)  // Enable graphics mode
#define VKY_MC0_OVRLY    (1 << 1)  // Enable overlay mode
#define VKY_MC0_TEXT     (1 << 0)  // Enable text mode

// --- Master Control Register 1 (0xD001) bit definitions ---
#define VKY_MC1_FON_SET  (1 << 5)  // Select font 1
#define VKY_MC1_FON_OVLY (1 << 4)  // Enable font overlay background
#define VKY_MC1_MON_SLP  (1 << 3)  // Monitor sleep
#define VKY_MC1_DBL_Y    (1 << 2)  // Double height text
#define VKY_MC1_DBL_X    (1 << 1)  // Double width text
#define VKY_MC1_CLK_70   (1 << 0)  // 70 Hz refresh (else 60 Hz)

// Register addresses
#define VKY_MSTR_CTRL_0  0xD000
#define VKY_MSTR_CTRL_1  0xD001

// --- Layer IDs ---
#define LAYER_BITMAP0  0
#define LAYER_BITMAP1  1
#define LAYER_BITMAP2  2
#define LAYER_TILE0    4
#define LAYER_TILE1    5
#define LAYER_TILE2    6

// --- Layer Control Register addresses ---
#define VKY_LAYER_CTRL_0  0xD002
#define VKY_LAYER_CTRL_1  0xD003

// --- Helpers to pack layers into registers ---
#define VKY_LAYER0(val)   ((val) & 0x07)          // bits 2..0 for layer 0
#define VKY_LAYER1(val)   (((val) & 0x07) << 4)   // bits 6..4 for layer 1
#define VKY_LAYER2(val)   ((val) & 0x07)          // bits 2..0 for layer 2 (second reg)

// --- Bitmap control register addresses ---
#define VKY_BM0_CTRL   0xD100
#define VKY_BM1_CTRL   0xD108
#define VKY_BM2_CTRL   0xD110

// --- CLUT selection (bits 2-1) ---
#define BM_CLUT_0   (0 << 1)
#define BM_CLUT_1   (1 << 1)
#define BM_CLUT_2   (2 << 1)
#define BM_CLUT_3   (3 << 1)

// --- Enable flag (bit 0) ---
#define BM_ENABLE   0x01
#define BM_DISABLE  0x00

// --- Helper macro to combine CLUT + enable ---
#define BM_CTRL(clut, enable)  ((clut) | ((enable) ? BM_ENABLE : BM_DISABLE))

// --- Base addresses for the 4 graphic CLUTs ---
#define VKY_GR_CLUT_0   0xD000
#define VKY_GR_CLUT_1   0xD400
#define VKY_GR_CLUT_2   0xD800
#define VKY_GR_CLUT_3   0xDC00

// --- CLUT size ---
#define CLUT_ENTRIES       256
#define CLUT_BYTES_PER     4
#define CLUT_TOTAL_BYTES   (CLUT_ENTRIES * CLUT_BYTES_PER)

static inline void openAllCODEC()
{
  POKE(0xD620, 0x1F);
  POKE(0xD621, 0x2A);
  POKE(0xD622, 0x01);
  while(PEEK(0xD622) & 0x01);
}

static inline void boostVSClock()
{
  //target the clock register
  POKE(VS_SCI_ADDR,0x03);
  //aim for 2.5X clock multiplier, no frills
  POKE(VS_SCI_DATA,0x00);
  POKE(VS_SCI_DATA+1,0xc0);
  //trigger the command
  POKE(VS_SCI_CTRL,1);
  POKE(VS_SCI_CTRL,0);
  //check to see if it's done
  while (PEEK(VS_SCI_CTRL) & 0x80);
}

static inline void vky_set_bitmap_addr(uint8_t bm_index, uint32_t address) {
    if (bm_index > 2) return; // prevent out-of-range

    POKE(VKY_BM_ADDR_L(bm_index), (uint8_t)(address & 0xFF));         // bits 7..0
    POKE(VKY_BM_ADDR_M(bm_index), (uint8_t)((address >> 8) & 0xFF));  // bits 15..8
    POKE(VKY_BM_ADDR_H(bm_index), (uint8_t)((address >> 16) & 0x03)); // bits 17..16
}

// --- Helper to set both master control registers ---
static inline void vky_set_master_ctrl(uint8_t reg0_flags, uint8_t reg1_flags) {
    POKE(VKY_MSTR_CTRL_0, reg0_flags);
    POKE(VKY_MSTR_CTRL_1, reg1_flags);
}

// --- Function to set all three layers ---
static inline void vky_set_layers(uint8_t layer0, uint8_t layer1, uint8_t layer2) {
    POKE(VKY_LAYER_CTRL_0, VKY_LAYER0(layer0) | VKY_LAYER1(layer1));
    POKE(VKY_LAYER_CTRL_1, VKY_LAYER2(layer2));
}

// --- Inline helper for clarity ---
static inline void vky_set_bitmap_ctrl(uint16_t addr, uint8_t clut, bool enable) {
    POKE(addr, BM_CTRL(clut, enable));
}

// --- CLUT loader from FAR RAM ---
static inline void vky_load_clut(uint16_t clutAddr, uint32_t farSrcAddr) {
	SET_IO_PAGE(PAGE_1);
    for (int i = 0; i < CLUT_TOTAL_BYTES; ++i) {
        POKE(clutAddr + i, FAR_PEEK(farSrcAddr + i));
    }
	SET_IO_PAGE(PAGE_0);
}