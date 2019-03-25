#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_lvgl.h"

#define FREE_RTOS_TASK

static lv_style_t blue_red_style;
static lv_style_t yellow_cyan_style;
static lv_obj_t *letter_w;
static lv_obj_t *update_count_lbl;

static void create_ui(void) {
  lv_style_copy(&blue_red_style, &lv_style_plain);
  blue_red_style.body.main_color = LV_COLOR_BLUE;
  blue_red_style.body.grad_color = LV_COLOR_BLUE;

  lv_style_copy(&yellow_cyan_style, &lv_style_plain);
  yellow_cyan_style.body.main_color = LV_COLOR_YELLOW;
  yellow_cyan_style.body.grad_color = LV_COLOR_YELLOW;

  // Create a matrix with 24 rows and 32 columns where each cell
  // contains a 10 by 10 pixel object. Half of the time the cells
  // will be displayed in a blue/yellow chequered pattern. The other
  // half of the time the objects will be displayed in a red/cyan
  // pattern.
  for (int row = 0; row <= 23; row++) {
    for (int col = 0; col <= 31; col++) {
      lv_obj_t *cell = lv_obj_create(lv_scr_act(), NULL);
      lv_obj_set_size(cell, 10, 10);
      lv_obj_set_pos(cell, col * 10, row * 10);

      bool is_blue_red_style =
        (row % 2 == 0 && col % 2 == 0) ||  (row % 2 == 1 && col % 2 == 1);
      
      if (is_blue_red_style) {
        lv_obj_set_style(cell, &blue_red_style);
      } else {
        lv_obj_set_style(cell, &yellow_cyan_style);
      }
    }
  }

  // The points for a big wide letter "w". The letter will be
  // displayed at the top of the screen half of the time and at
  // the bottom of the screen the other half of the time.
  static lv_point_t letter_w_points[] = {
    {5, 5}, {70, 70}, {120, 10}, {180, 60}, {240, 10}
  };

  static lv_style_t style_letter_w;
  lv_style_copy(&style_letter_w, &lv_style_plain);
  style_letter_w.line.width = 5;

  letter_w = lv_line_create(lv_scr_act(), NULL);
  lv_line_set_points(letter_w, letter_w_points, 5);
  lv_line_set_style(letter_w, &style_letter_w);
  lv_obj_align(letter_w, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

  // Create a label for displaying the number to times update_ui
  // was called.
  update_count_lbl = lv_label_create(lv_scr_act(), NULL);
  lv_obj_align(update_count_lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 5, 0);
}

static void update_ui() {
  static int update_count = 0;

  if (update_count % 2 == 0) {
    blue_red_style.body.main_color = LV_COLOR_RED;
    blue_red_style.body.grad_color = LV_COLOR_RED;

    yellow_cyan_style.body.main_color = LV_COLOR_CYAN;
    yellow_cyan_style.body.grad_color = LV_COLOR_CYAN;

    lv_obj_align(letter_w, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
  } else {
    blue_red_style.body.main_color = LV_COLOR_BLUE;
    blue_red_style.body.grad_color = LV_COLOR_BLUE;

    yellow_cyan_style.body.main_color = LV_COLOR_YELLOW;
    yellow_cyan_style.body.grad_color = LV_COLOR_YELLOW;

    lv_obj_align(letter_w, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
  }

  lv_obj_report_style_mod(NULL);

  update_count += 1;

  char buffer[50];
  sprintf(buffer, "%d", update_count); 
  lv_label_set_text(update_count_lbl, buffer);
}

#ifdef FREE_RTOS_TASK
static void rtos_task(void *pvParameter) {
  while (1) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    update_ui();
  }
}

#else

static void lv_task(void *pvParameter) {
  update_ui();
}
#endif

extern "C" void app_main() {
  lvgl_init();

  create_ui();

#ifdef FREE_RTOS_TASK
  xTaskCreate(rtos_task, "rtos_task", 2048, NULL, 1, NULL);
#else
  lv_task_create(lv_task, 10, LV_TASK_PRIO_MID, NULL);
#endif

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

