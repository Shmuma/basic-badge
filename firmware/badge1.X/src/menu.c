#include <string.h>
#include "menu.h"

const char* header_text = "Use arrows+ENTER or press number";

void menu_draw_frame();
void menu_draw_title(const char* text);
void menu_draw_header();
void menu_draw_menu(uint8_t active, const struct menu_t* items, int8_t blank_lines);
void menu_draw_item(uint8_t ofs, uint8_t is_active, const struct menu_t* item);
const struct menu_t* menu_parent_items(const struct menu_t* menu, 
        const struct menu_t* ref_item);

struct menu_t parent_item = {
    .id = 0,
    .title = ".."
};


uint16_t menu_run(const struct menu_t* menu) {
    const struct menu_t* cur_menu = menu, *prev_menu;
    uint8_t active = 0;
    int8_t char_out, selected_item;
    
    set_cursor_state(0);
	video_clrscr();
    menu_draw_frame();
    menu_draw_title(menu->title);
    menu_draw_header();
    menu_draw_menu(active, cur_menu, 0);
    
    while (1) {
        if (brk_key)
            break;
        if (stdio_get(&char_out) != 0) {
            selected_item = -1;
            if (char_out == K_UP && active > 0) {
                menu_draw_item(active, 0, cur_menu->items + active - 1);
                active--;
                menu_draw_item(active, 1, 
                        active > 0 ? cur_menu->items + active - 1 : &parent_item);
            }
            else if (char_out == K_DN && active < cur_menu->items_count) {
                menu_draw_item(active, 0, 
                        active > 0 ? cur_menu->items + active - 1 : &parent_item);
                active++;
                menu_draw_item(active, 1, cur_menu->items + active - 1);
            }
            else if (char_out == K_ENT)
                selected_item = active;
            else if (char_out >= '0' && char_out <= '0'+cur_menu->items_count) {
                selected_item = char_out - '0';
                // draw the element selected
                menu_draw_menu(selected_item, cur_menu, 0);
            }
            
            // handle selected
            if (selected_item >= 0) {
                prev_menu = cur_menu;
                // look for parent of current item
                if (!selected_item) {
                    cur_menu = menu_parent_items(menu, cur_menu);
                    // we're at the top level and go up -- just exit with zero status
                    if (!cur_menu)
                        return 0;
                }
                else {
                    cur_menu = cur_menu->items + selected_item - 1;
                    // selected item is final, return the id
                    if (!cur_menu->items_count)
                        return cur_menu->id;                    
                }
                active = 0;
                menu_draw_menu(active, cur_menu, prev_menu->items_count - cur_menu->items_count);                
            }
        }
    };
    set_cursor_state(1);
    return 0;
}


void menu_draw_frame() {
	uint16_t i;
    
    video_set_color(MENU_FRAME_FG, MENU_FRAME_BG);
	video_gotoxy(0, 0);
	stdio_c(201);
    stdio_c_n(205, TERM_WIDTH-2);
	stdio_c(187);
	video_gotoxy(0, TERM_WIDTH-1);
	stdio_c(200);
    stdio_c_n(205, TERM_WIDTH-2);
	stdio_c(188);
	for (i = 1; i < 19; i++) {
		video_gotoxy(0, i);
		stdio_c(186);
		video_gotoxy(TERM_WIDTH-1, i);
		stdio_c(186);
	}
}

void menu_draw_title(const char* text) {
    uint8_t l = strlen(text);
    l = (TERM_WIDTH - 2 - l) >> 1;
    
    video_set_color(MENU_TITLE_FG, MENU_TITLE_BG);
    video_gotoxy(1, 1);
    stdio_c_n(' ', TERM_WIDTH-2);
    video_gotoxy(1, 2);
    stdio_c_n(' ', l);
    stdio_write(text);
    stdio_c_n(' ', l);
    video_gotoxy(1, 3);
    stdio_c_n(' ', TERM_WIDTH-2);
}

void menu_draw_header() {
    uint8_t l = strlen(header_text);
    l = (MENU_WIDTH - l) >> 1;
    video_set_color(MENU_HEADER_FG, MENU_HEADER_BG);
    video_gotoxy((TERM_WIDTH - MENU_WIDTH) >> 1, 5);
    stdio_c_n(' ', l);
    stdio_write(header_text);
    stdio_c_n(' ', MENU_WIDTH - l - strlen(header_text));
}


void menu_draw_menu(uint8_t active, const struct menu_t* menu, int8_t blank_lines)
{
    uint8_t i;
    menu_draw_item(0, active == 0, &parent_item);
    for (i = 1; i <= menu->items_count; i++) {
        menu_draw_item(i, active == i, menu->items + i - 1);
    }
    video_set_color(0, 0);
    while (blank_lines-- > 0) {
        video_gotoxy((TERM_WIDTH - MENU_WIDTH) >> 1, 
                6 + menu->items_count + blank_lines + 1);
        stdio_c_n(' ', MENU_WIDTH);
    }
}

void menu_draw_item(uint8_t ofs, uint8_t is_active, 
        const struct menu_t* item)
{
    const char* title;
    
    if (is_active)
        video_set_color(MENU_ACTIVE_FG, MENU_ACTIVE_BG);
    else
        video_set_color(MENU_ITEM_FG, MENU_ITEM_BG);

    video_gotoxy((TERM_WIDTH - MENU_WIDTH) >> 1, 6 + ofs);
    stdio_c(' ');
    stdio_c('0' + ofs);
    stdio_write(": ");
    if (item->title)
        title = item->title;
    else if (item->title_func)
        title = item->title_func();
    else
        title = "ERROR: no item title or func";
    stdio_write(title);
    stdio_c_n(' ', MENU_WIDTH - 4 - strlen(title));
}


// find the parent, containing of the given menu item. 
// If no item found, NULL is returned. 
const struct menu_t* menu_parent_items(const struct menu_t* menu, 
        const struct menu_t* ref_item)
{
    const struct menu_t* res;
    uint8_t i;
    
    for (i = 0; i < menu->items_count; i++) {
        if (menu->items + i == ref_item)
            return menu;
        if (menu->items[i].items_count) {
            res = menu_parent_items(menu->items + i, ref_item);
            if (res)
                return res;
        }
    }
    
    return NULL;
}