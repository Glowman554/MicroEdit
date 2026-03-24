#include <input.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <render.h>

void move_up(edit_state_t* state) {
	if (state->buffer_ln_idx <= 0 || state->buffer_idx <= 0) {
	} else {
		// move one line up
		int prev_buff = state->buffer_idx;

		for (int i = state->buffer_idx; i > 0; i--) {
			state->buffer_idx--;
			if (state->input_buffer[i - 1] == '\n' || state->buffer_idx < 0) {
				break;
			}
		}
		if (state->buffer_idx < 0) {
			state->buffer_idx = prev_buff;
		} else {
			state->buffer_ln_idx--;
		}
	}
}

void move_down(edit_state_t* state) {
	if (state->buffer_ln_idx >= state->ln_cnt - 1 || state->buffer_idx >= state->current_size) {
	} else {
		// move one line up
		int prev_buff = state->buffer_idx;

		for (int i = state->buffer_idx; i < state->current_size; i++) {
			state->buffer_idx++;
			if (state->input_buffer[i] == '\n' || state->buffer_idx > state->current_size) {
				break;
			}
		}
		if (state->buffer_idx > state->current_size) {
			state->buffer_idx = prev_buff;
		} else {
			state->buffer_ln_idx++;
		}
	}
}

void move_left(edit_state_t* state) {
	if (!(state->buffer_idx <= 0)) {
		if (state->input_buffer[state->buffer_idx - 1] == '\n') {
			state->buffer_ln_idx--;
		}
		state->buffer_idx -= 1;
	}
}

void move_right(edit_state_t* state) {
	if (state->buffer_idx < state->current_size) {
		if (state->input_buffer[state->buffer_idx] == '\n') {
			state->buffer_ln_idx++;
		}
		state->buffer_idx += 1;
	}
}


bool listen_input(edit_state_t* state) {
	char input = 0;
	if (read(STDIN_FILENO, &input, 1) == 1) {
		if (input == '\e') {
			char seq[2];
			if (read(STDIN_FILENO, seq, 2) == 2 && seq[0] == '[') {
				switch (seq[1]) {
					case 'A':
						move_up(state);
						state->needs_render = true;
						return false;
					case 'B':
						move_down(state);
						state->needs_render = true;
						return false;
					case 'D':
						move_left(state);
						state->needs_render = true;
						return false;
					case 'C':
						move_right(state);
						state->needs_render = true;
						return false;
				}
			}
		}
	} else {
		return false;
	}

	if (state->read_only) {
		switch (input) {
			case 'q':
				return true;
			case '\e':
				return true;
			case 'a':
				move_left(state);
				state->needs_render = true;
				break;
			case 'd':
				move_right(state);
				state->needs_render = true;
				break;
			case 'w':
				move_up(state);
				state->needs_render = true;
				break;
			case 's':
				move_down(state);
				state->needs_render = true;
				break;
		}
	} else {
		if (!state->is_in_insert_mode) {
			switch (input) {
				case 'q':
					return true;
				case '\e':
					state->is_in_insert_mode = !state->is_in_insert_mode;
					state->needs_render = true;
					break;

				case 'a':
					move_left(state);
					state->needs_render = true;
					break;
				case 'd':
					move_right(state);
					state->needs_render = true;
					break;
				case 'w':
					move_up(state);
					state->needs_render = true;
					break;
				case 's':
					move_down(state);
					state->needs_render = true;
					break;

				case '+':
					state->file = freopen(state->file_name, "w", state->file);
					fseek(state->file, 0, SEEK_SET);
					fwrite(state->input_buffer, state->current_size, 1, state->file);
					fseek(state->file, state->current_size, SEEK_SET);
					// ftruncate(state->file);
					state->is_edited = false;
					state->needs_render = true;
					break;
			}
		} else {
			switch (input) {
				case 127: {
					if (state->buffer_idx < 1 || state->current_size < 1) {
					} else {
						if (state->input_buffer[state->buffer_idx - 1] == '\n') {
							state->buffer_ln_idx--;
						}

						if (state->buffer_idx == state->current_size) {
						} else {
						memmove((void*) &state->input_buffer[state->buffer_idx - 1], (void*) &state->input_buffer[state->buffer_idx], (state->current_size - state->buffer_idx) * sizeof(char));
						}
						if (state->input_buffer[state->buffer_idx] == '\n') {
							state->ln_cnt--;
						} else {
							state->char_cnt--;
						}
						state->input_buffer = (char*) realloc((void*) state->input_buffer, --state->current_size);
						state->buffer_idx--;

						rerender_color(state);
					}
				}
				state->needs_render = true;
				state->needs_render = true;
				break;

				case '\e': {
					state->is_in_insert_mode = !state->is_in_insert_mode;
					state->needs_render = true;
				}
				break;


				default: {
					if (input == '\r') {
						state->ln_cnt++;
						state->buffer_ln_idx++;
					} else {
						state->char_cnt++;
					}

					state->is_edited = true;
					state->current_size++;
					state->input_buffer = (char*) realloc((void*) state->input_buffer, state->current_size);
					memmove((void*) &state->input_buffer[state->buffer_idx + 1], (void*) &state->input_buffer[state->buffer_idx], (state->current_size - state->buffer_idx - 1) * sizeof(char));
					state->input_buffer[state->buffer_idx] = input == '\r' ? '\n' : input;
					state->buffer_idx++;

					rerender_color(state);
					state->needs_render = true;
				}
				break;
			}
		}
	}

	return false;
}