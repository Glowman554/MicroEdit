#include <render.h>

#include <string.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <unistd.h>

#define TAB_WIDTH 4

syntax_header_t* syntax = NULL;

void start_frame() {
    printf("\x1b[H\x1b[2J");
}

void end_frame() {
    fflush(stdout);
}

int get_width() { 
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

int get_height() { 
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_row;
}

void draw_char(int x, int y, char c, const char* color) {
    printf("\x1b[%d;%dH%s%c\x1b[0m", y + 1, x + 1, color, c);
}

void draw_string(int x, int y, char* str, const char* color) {
    printf("\x1b[%d;%dH%s%s\x1b[0m", y + 1, x + 1, color, str);
} 

const char* color_translation[] = {
    "\x1b[30m",
    "\x1b[31m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[34m",
    "\x1b[35m",
    "\x1b[36m",
    "\x1b[37m"
};

uint8_t* color = NULL;

void rerender_color(edit_state_t* state) {
	if (syntax) {	
		if (color) {
			free(color);
		}
		color = highlight(state->input_buffer, state->current_size, syntax);
	}
}

void draw_line_number(int cur_y, int current_line) {
	char buff[16] = { 0 };
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d.", current_line);
	draw_string(0, cur_y, buff, "\x1b[34m");
}

void render_tui(edit_state_t* state) {
	start_frame();

	if (!color) {
		rerender_color(state);
	}


	int width = get_width();
	int height = get_height();

	char buff[512] = { 0 };
	if (state->read_only) {
		sprintf(buff, "File: %s Current Line: %d Line: %d", state->file_name, state->buffer_ln_idx + 1, state->ln_cnt);
	} else {
		sprintf(buff, "File: %s [%c] -- %s -- Current Line: %d Line: %d", state->file_name, state->is_edited ? '*' : '-',  state->is_in_insert_mode ? "INSERT" : "EDIT", state->buffer_ln_idx + 1, state->ln_cnt);
	}

	draw_string(0, height - 1, buff, "\x1b[34m\x1b[47m");

	int j = 0;
	int cur_x = 0;
	int cur_y = 0;
	int already_drawn = 0;
	bool initial_line_drawn = false;
	bool cursor_drawn = false;

	memset(buff, 0, 512);
	sprintf(buff, "%d .", state->ln_cnt);
	int space_to_draw = 1 + strlen(buff);
	cur_x = space_to_draw;

	int viewport_height = height - 3;
	static int start_line = 0;
	
	if (state->buffer_ln_idx < start_line) {
		start_line = state->buffer_ln_idx;
	} else if (state->buffer_ln_idx >= start_line + viewport_height) {
		start_line = state->buffer_ln_idx - viewport_height + 1;
	}
	
	if (start_line < 0) {
		start_line = 0;
	}
	if (start_line + viewport_height > state->ln_cnt) {
		start_line = state->ln_cnt - viewport_height;
		if (start_line < 0) {
			start_line = 0;
		}
	}

	int current_line = start_line + 1;

	for (int i = 0; i < state->current_size; i++) {
		if ((j >= start_line && j < start_line + viewport_height) && already_drawn <= viewport_height) {
			if (!initial_line_drawn) {
				initial_line_drawn = true;
				draw_line_number(cur_y, current_line);
			}

			if (cur_x >= width) {
				cur_y++;
				cur_x = space_to_draw;
				already_drawn++;
			}

            if (i == state->buffer_idx) {
				draw_char(cur_x, cur_y, '|', "\x1b[36m");
				cur_x++;
				cursor_drawn = true;
			}

			if (state->input_buffer[i] == '\t') {
				int col = cur_x - space_to_draw;
				int next_tab_stop = ((col / TAB_WIDTH) + 1) * TAB_WIDTH;
				int spaces = next_tab_stop - col;
				
				for (int s = 0; s < spaces; s++) {
					if (cur_x >= width) {
						cur_y++;
						cur_x = space_to_draw;
						already_drawn++;
						draw_line_number(cur_y, current_line);
					}
					draw_char(cur_x, cur_y, state->show_tab_char ? '.' : ' ', "\x1b[37m");
					cur_x++;
				}
				cur_x--;
			}

			if (state->input_buffer[i] >= 0x20 && state->input_buffer[i] <= 0x7E) {
				draw_char(cur_x, cur_y, state->input_buffer[i], (color ? color_translation[color[i]] : "\x1b[37m"));
			}
			
			cur_x++;

			if (state->input_buffer[i] == '\n') {
				already_drawn++;
				current_line++;
				j++;
				cur_x = space_to_draw;
				cur_y++;
				draw_line_number(cur_y, current_line);
			}
		} else {
			if (state->input_buffer[i] == '\n') {
				j++;
			}
		}
	}

	if (!cursor_drawn) {
		if (!initial_line_drawn) {
			memset(buff, 0, 512);
			sprintf(buff, "%d.", current_line);
			draw_string(0, cur_y, buff, "\x1b[30m");
		}
		draw_char(cur_x, cur_y, '|', "\x1b[36m");
	}

	end_frame();
}