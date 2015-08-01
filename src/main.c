#include <pebble.h>
#include <time.h>

Window *window;
Layer *layer;
GBitmap *bmp;
BitmapLayer *bl;
GFont *font;

uint8_t colors[3] = {255, 255, 255};

char datebuf[11] = {0};
char timebuf[9] = {0};

int DATE_HEIGHT = 18;
int TIME_HEIGHT = 18;

int blockW;
int blockH;
int blocksH;

int W;
int H;

void refresh (struct tm *ts, TimeUnits changed)
{
  strftime(datebuf, 11, "%Y/%m/%d", ts);
  strftime(timebuf, 9, "%H:%M:%S", ts);

  uint8_t buf[3][6];
  for(int i = 0; i < 6; i++)
  {
    buf[0][5 - i] = (ts->tm_hour >> (i)) & 1;
    buf[1][5 - i] = (ts->tm_min >> (i)) & 1;
    buf[2][5 - i] = (ts->tm_sec >> (i)) & 1;
  }

  uint8_t *p = gbitmap_get_data(bmp);
  for(int i = 0; i < W * H; i++)
  {
    int y = (i / W) - DATE_HEIGHT;
    int x = i % blockW;

    if(x == 0 || y < 0 || y > blocksH || y % blockH == 0)
      continue;

    int block_y = y / blockH;
    p[i] = buf[block_y][((i / blockW) % 6)] ? colors[block_y] : 0;
  }

  layer_mark_dirty(bitmap_layer_get_layer(bl));
}

void render(struct Layer *layer, GContext *ctx)
{
  GRect rect = layer_get_bounds(layer);
  graphics_draw_bitmap_in_rect(ctx, bmp, rect);

  int h = rect.size.h;
  rect.size.h = DATE_HEIGHT;
  graphics_draw_text(ctx, datebuf, font, rect, 0, GTextAlignmentCenter, 0);

  rect.size.h = h - TIME_HEIGHT;
  rect.origin.y = DATE_HEIGHT + blockH * 3;
  graphics_draw_text(ctx, timebuf, font, rect, 0, GTextAlignmentCenter, 0);
}

void init()
{
  window = window_create();
  window_stack_push(window, true);

  layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(layer);

  W = bounds.size.w;
  H = bounds.size.h;

  blockW = W / 6;
  blocksH = (H - DATE_HEIGHT - TIME_HEIGHT);
  blockH = blocksH / 3;

  font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  bl = bitmap_layer_create(layer_get_frame(layer));
  bmp = gbitmap_create_blank(bounds.size, 1);

  layer_add_child(layer, bitmap_layer_get_layer(bl));

  layer_set_update_proc(bitmap_layer_get_layer(bl), render);

  time_t temp = time(NULL);
  struct tm *ts = localtime(&temp);

  refresh(ts, 0);

  tick_timer_service_subscribe(SECOND_UNIT, refresh);
}

void deinit()
{
  layer_remove_from_parent(bitmap_layer_get_layer(bl));
  gbitmap_destroy(bmp);
  bitmap_layer_destroy(bl);
  window_destroy(window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}
