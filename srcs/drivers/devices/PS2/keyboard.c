/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyboard.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 23:24:49 by mbirou            #+#    #+#             */
/*   Updated: 2026/11/24 18:20:25 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <kernel.h>


enum KEYS{	NONE, KEY_ESCAPE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB,
			KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFT_BRAQUET, KEY_RIGHT_BRAQUET, KEY_ENTER, KEY_LEFT_CTRL,
			KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_SINGLE_QUOTE, KEY_BACK_TICK, KEY_LEFT_SHIFT, KEY_BACK_SLASH,
			KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMA, KEY_DOT, KEY_SLASH, KEY_RIGHT_SHIFT, KEY_KP_MULT, KEY_LEFT_ALT, KEY_SPACE, KEY_CAPS_LOCK,
			KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_NUMLOCK, KEY_SCROLLLOCK,
			KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_SUB, KEY_KP_4, KEY_KP_5, KEY_KP_6, KEY_KP_ADD, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_0, KEY_KP_DOT, BAD, BAD1, BAD2, KEY_F11, KEY_F12 };
static const char		*key_values[89] = {	"BAD", "ESCAPE", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BACKSPACE", "TAB",
									"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "ENTER", "LEFT_CTRL",
									"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "LEFT_SHIFT", "\\",
									"z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "RIGHT_SHIFT", "*", "LEFT_ALT", " ", "CAPS_LOCK",
									"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM_LOCK", "SCROLL_LOCK",
									"7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "BAD", "BAD", "BAD", "F11", "F12"};
static uint8_t	keyboard_state = 0;


inline STATE	get_state_key_status(STATE_KEY key)
{
	return ((keyboard_state & (1 << key)) >> key);
}

static inline void	set_state_key_status(STATE_KEY key, STATE state)
{
	keyboard_state = (keyboard_state & ~(1 << key)) | (state << key);
}

int	setup_keyboard(void)
{
	set_state_key_status(NUM, ON);
	// set scan code to 1 -> this changes the code made by keys
	if (ACK_safe_write_port(0x60, 0xF0) == -1)
		return (0);
	if (ACK_safe_write_port(0x60, 1) == -1)
		return (0);
	return (1);
}


static int	special_keys(uint8_t keycode)
{
	switch (keycode)
	{
		case 0xE0:
			flush(); // TODO : decide if we handle some special keys such as the arrows.
			break;
		case KEY_BACKSPACE:
			erase_char();
			break;
		case KEY_TAB:
			tabulation();
			break;
		case KEY_ENTER:
			newline();
			break ;

		case KEY_LEFT_CTRL:
			set_state_key_status(CTRL, ON);
			break ;
		case KEY_LEFT_SHIFT: case KEY_RIGHT_SHIFT:
			set_state_key_status(SHIFT, ON);
			break ;
		case KEY_LEFT_ALT:
			set_state_key_status(ALT, ON);
			break ;
		case KEY_CAPS_LOCK:
			set_state_key_status(CAPS, !get_state_key_status(CAPS));
			break ;
		case KEY_NUMLOCK:
			set_state_key_status(NUM, !get_state_key_status(NUM));
			break ;
		case KEY_SCROLLLOCK:
			set_state_key_status(SCRL, !get_state_key_status(SCRL));
			break ;

		case KEY_F1: case KEY_F2: case KEY_F3: case KEY_F4: case KEY_F5: case KEY_F6:
		case KEY_F7: case KEY_F8: case KEY_F9: case KEY_F10: case KEY_F11: case KEY_F12:
		case NONE: case BAD: case BAD1: case BAD2:
			break;

		default:
			if (keycode > KEY_F12)
				break;
			return (0);
	}
	if (keycode > KEY_F12)
	{
		keycode -= 0x80;
		switch (keycode)
		{
			case 0xE0: case KEY_BACKSPACE: case KEY_TAB: case KEY_ENTER:
				break ;

			case KEY_LEFT_CTRL:
				set_state_key_status(CTRL, OFF);
				break ;
			case KEY_LEFT_SHIFT: case KEY_RIGHT_SHIFT:
				set_state_key_status(SHIFT, OFF);
				break ;
			case KEY_LEFT_ALT:
				set_state_key_status(ALT, OFF);
				break ;
			
			default:
				return (0);
		}
	}
	return (1);
}

static int	is_alpha(char c)
{
	return ('a' <= c && c <= 'z');
}

static int	key_is_from_keypad(uint8_t keycode)
{
	switch (keycode)
	{
		case KEY_KP_7: case KEY_KP_8: case KEY_KP_9: case KEY_KP_4:case KEY_KP_5:
		case KEY_KP_6: case KEY_KP_1: case KEY_KP_2: case KEY_KP_3: case KEY_KP_0: case KEY_KP_DOT:
			return (1);
		
		default:
			return (0);
	}
}

int	poll_keyboard(void)
{
	int	scancode = safe_read_port(0x60);

	if (scancode == -1)
		return (0);

	update_mouse_pos();

	if (special_keys(scancode))
		return (1);

	if (scancode > KEY_F12)
		return (1);

	if (get_state_key_status(CTRL))
		return (1);

	if (get_state_key_status(ALT))
		return (1);
	
	if (get_state_key_status(NUM) == OFF && key_is_from_keypad(scancode))
		return (1);

	if ((get_state_key_status(SHIFT) ^ get_state_key_status(CAPS)) && is_alpha(key_values[scancode][0]))
		printk("%c", 'A' + (key_values[scancode][0] - 'a'));
	else
		printk("%c", key_values[scancode][0]);
	return (1);
}
