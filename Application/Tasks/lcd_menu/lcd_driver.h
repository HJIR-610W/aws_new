#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_ROWS 8
#define LCD_COLS 20
#define LCD_PAGE_MAX 10

typedef struct
{
	int current_row;
	int view_row;
	int view_col;
	int scroll_offset[LCD_PAGE_MAX];
	int total_items[LCD_PAGE_MAX];
	int current_page;
	int total_pages;
} lcd_win_t;

// 모드 enum 제거 - 단순화

// LCD hardware functions (사용자가 구현해야 함)
void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(int row, int col);
void lcd_write_char(char c);
void lcd_write_string(const char* str);

// LCD window management functions
void lcd_create_win(lcd_win_t* win);
void lcd_clear_win(lcd_win_t* win);
void lcd_print_row(lcd_win_t* win, int row_index, const char* text);
void lcd_refresh_win(lcd_win_t* win);

// Navigation functions
void lcd_handle_scroll(lcd_win_t* win, int key);
int lcd_get_key_input(void);

// Key definitions (Enter 제거)
#define LCD_KEY_UP    1
#define LCD_KEY_DOWN  2
#define LCD_KEY_LEFT  3
#define LCD_KEY_RIGHT 4
#define LCD_KEY_ESC   5

#ifdef __cplusplus
}
#endif

#endif // LCD_DRIVER_H