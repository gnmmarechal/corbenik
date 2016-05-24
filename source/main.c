#include "common.h"
#include "firm/firm.h"
#include "input.h"
#include "config.h"

int menu_handler();

int doing_autoboot = 0;
void shut_up();

int
main()
{
    if (fmount()) {
        // Failed to mount SD. Bomb out.
        fprintf(BOTTOM_SCREEN, "%pFailed to mount SD card.\n",
                COLOR(RED, BLACK));
    }

    load_config(); // Load configuration.

    // Autoboot. Non-standard code path.
    if (config.options[OPTION_AUTOBOOT] && !(HID_PAD & BUTTON_R)) {
        if (config.options[OPTION_SILENCE])
            shut_up(); // This does exactly what it sounds like.
        doing_autoboot = 1;
        boot_cfw(); // Just boot shit.
    }

    int in_menu = 1;
    while (in_menu) {
        in_menu = menu_handler();
    }

    save_config(); // Save config file.

    boot_cfw();
    // Under ideal conditions, we never get here.
}
