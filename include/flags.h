#include <cstdint>

union PPUCtrl {
    uint8_t raw;
    struct {
        unsigned int nt : 2;
        bool incr : 1;
        bool sprite_pt : 1;
        bool bg_pt : 1;
        bool sprite_sz : 1;
        bool master_slave : 1;
        bool vblank_enable : 1;
    };
};

union PPUMask {
    uint8_t raw;
    struct {
        bool grayscale : 1;
        bool show_bg_left : 1;
        bool show_sprite_left : 1;
        bool bg_render : 1;
        bool sprite_render : 1;
        bool emphasize_r : 1;
        bool emphasize_g : 1;
        bool emphasize_b : 1;
    };
};

union PPUStatus {
    uint8_t raw;
    struct {
        unsigned int dummy : 5;
        bool sprite_ovf : 1;
        bool sprite_0_hit : 1;
        bool vblank : 1;
    };
};