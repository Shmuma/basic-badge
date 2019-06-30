#include <string.h>
#include "menu.h"

const char* header_text = "Use arrows+ENTER or press number";

void menu_draw_frame();
void menu_draw_title(const char* text);
void menu_draw_header();
void menu_draw_menu(uint8_t active, uint8_t items_count, 
        const struct menu_item_t* items);
void menu_draw_item(uint8_t ofs, uint8_t is_active, 
        const struct menu_item_t* item);

struct menu_item_t parent_item = {
    .id = 0,
    .title = ".."
};


void menu_run(struct menu_t* menu) {
    const struct menu_item_t* level_items = menu->items;
    uint8_t level_count = menu->items_count;
    uint8_t active = 0;
    int8_t char_out, selected_item;
    
    set_cursor_state(0);
	video_clrscr();
    menu_draw_frame();
    menu_draw_title(menu->title);
    menu_draw_header();
    menu_draw_menu(active, level_count, level_items);
    
    // draw menu items
    // handle keys pressed
    // update the items
    
//	video_set_color(0, 12);
//    video_gotoxy(1,1); 
//    stdio_write(menu->title);
    while (1) {
        if (brk_key)
            break;
        if (stdio_get(&char_out) != 0) {
            selected_item = -1;
            if (char_out == K_UP && active > 0) {
                active--;
                menu_draw_menu(active, level_count, level_items);
            }
            else if (char_out == K_DN && active < menu->items_count) {
                active++;
                menu_draw_menu(active, level_count, level_items);
            }
            else if (char_out == K_ENT)
                selected_item = active;
            else if (char_out >= '0' && char_out <= '0'+level_count) {
                selected_item = char_out - '0';
            }
            
            if (selected_item >= 0) {
                // handle selected
            }
        }
    };
    set_cursor_state(1);
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


void menu_draw_menu(uint8_t active, uint8_t items_count, 
        const struct menu_item_t* items)
{
    uint8_t i;
    
    menu_draw_item(0, active == 0, &parent_item);
    for (i = 1; i <= items_count; i++) {
        menu_draw_item(i, active == i, items + i - 1);
    }
}

void menu_draw_item(uint8_t ofs, uint8_t is_active, 
        const struct menu_item_t* item)
{
    if (is_active)
        video_set_color(MENU_ACTIVE_FG, MENU_ACTIVE_BG);
    else
        video_set_color(MENU_ITEM_FG, MENU_ITEM_BG);

    video_gotoxy((TERM_WIDTH - MENU_WIDTH) >> 1, 6 + ofs);
    stdio_c(' ');
    stdio_c('0' + ofs);
    stdio_write(": ");
    stdio_write(item->title);
    stdio_c_n(' ', MENU_WIDTH - 4 - strlen(item->title));
}