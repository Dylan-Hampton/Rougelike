#ifndef IO_H
# define IO_H

class dungeon;

int io_target_monster(dungeon *d);
void io_list_pc_inv(dungeon *d);
void io_list_pc_wearing(dungeon *d);
void io_inspect_item(dungeon *d, int slot);
void io_init_terminal(void);
void io_reset_terminal(void);
void io_display(dungeon *d);
void io_handle_input(dungeon *d);
void io_queue_message(const char *format, ...);

#endif
