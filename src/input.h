#include "Controller.h"

/* Processes pending SDL events and updates the NES controller state until the
 * no pending events remain or an exit is requested (escape pressed or window closed).
 * Sets *should_exit to true when exit is requested, otherwise keeps it as is. */
void poll_for_input(Controller *controller, bool *should_exit);