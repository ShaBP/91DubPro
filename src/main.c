/*
91 Dub Pro is an integration by ShaBP of two separate watchfaces:
  91 Dub by orviwan (one of the most popular Watchfaces on Pebble Store)
  ModernCalendar by Fowler/Baeumle, which works with Baeumle's popular Smartwatch Pro
Besides for the integration, week of year indication was added.
The end result is a practical design classic digital watchface, which presents calendar events read from Smartwatch Pro.
*/

#include "common.h"

static Window *window;
static Layer *window_layer;

static uint8_t batteryPercent;

//static AppSync sync;
//static uint8_t sync_buffer[64];

//#define SETTINGS_KEY 99

static TextLayer *event_layer;
static TextLayer *event_layer2; // from ModernCalendar
static bool status_showing = false; // from ModernCalendar
static char event_text[21]; // from ModernCalendar
static char event_text2[21]; // from ModernCalendar
static GBitmap *icon_status_1; // from ModernCalendar
static GBitmap *icon_status_2; // from ModernCalendar
static GBitmap *icon_status_3; // from ModernCalendar
static Layer *event_status_layer; // from ModernCalendar
static int event_status_display = 0; // from ModernCalendar
// Timers - from ModernCalendar
static AppTimer *display_timer;
static AppTimer *vibrate_timer;

/* removing settings
typedef struct persist {
	int Blink;
    int Invert;
    int BluetoothVibe;
    int HourlyVibe;
    int BrandingMask;
} __attribute__((__packed__)) persist;

persist settings = {
	.Blink = 0,
	.Invert = 0,
	.BluetoothVibe = 1,
	.HourlyVibe = 0,
	.BrandingMask = 1
};

static int valueRead, valueWritten;
*/
static bool appStarted = false;

/*
enum {
  BLINK_KEY = 0x0,
  INVERT_KEY = 0x1,
  BLUETOOTHVIBE_KEY = 0x2,
  HOURLYVIBE_KEY = 0x3,
  BRANDING_MASK_KEY = 0x4
};
*/

static GBitmap *branding_mask_image;
static BitmapLayer *branding_mask_layer;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static GBitmap *separator_image;
static BitmapLayer *separator_layer;

// static GBitmap *meter_bar_image;
// static BitmapLayer *meter_bar_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;

static GBitmap *battery_image;
static BitmapLayer *battery_image_layer;
static BitmapLayer *battery_layer;

static TextLayer *week_layer;
static char week_text[] = "w99";

static GBitmap *time_format_image;
static BitmapLayer *time_format_layer;

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

#define TOTAL_DATE_DIGITS 2	
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

#define TOTAL_BATTERY_PERCENT_DIGITS 3
static GBitmap *battery_percent_image[TOTAL_BATTERY_PERCENT_DIGITS];
static BitmapLayer *battery_percent_layers[TOTAL_BATTERY_PERCENT_DIGITS];

const int TINY_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_TINY_0,
  RESOURCE_ID_IMAGE_TINY_1,
  RESOURCE_ID_IMAGE_TINY_2,
  RESOURCE_ID_IMAGE_TINY_3,
  RESOURCE_ID_IMAGE_TINY_4,
  RESOURCE_ID_IMAGE_TINY_5,
  RESOURCE_ID_IMAGE_TINY_6,
  RESOURCE_ID_IMAGE_TINY_7,
  RESOURCE_ID_IMAGE_TINY_8,
  RESOURCE_ID_IMAGE_TINY_9,
  RESOURCE_ID_IMAGE_TINY_PERCENT
};

void change_background() {
  gbitmap_destroy(background_image);
  gbitmap_destroy(branding_mask_image);
/* remove invert setting option
  if(settings.Invert) {
    background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_INVERT);
    branding_mask_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BRANDING_MASK_INVERT);
  }
  else { */ 
    background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    branding_mask_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BRANDING_MASK);
//  }  
  bitmap_layer_set_bitmap(branding_mask_layer, branding_mask_image);
  layer_mark_dirty(bitmap_layer_get_layer(branding_mask_layer));
  
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_mark_dirty(bitmap_layer_get_layer(background_layer));
}

void change_battery_icon(bool charging) {
  gbitmap_destroy(battery_image);
  if(charging) {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGE);
  }
  else {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  }  
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed);

/*
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case BLINK_KEY:
      settings.Blink = new_tuple->value->uint8;
      tick_timer_service_unsubscribe();
      if(settings.Blink) {
        tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
      }
      else {
        tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
      }
      break;
    case INVERT_KEY:
      settings.Invert = new_tuple->value->uint8;
      change_background();
      break;
    case BLUETOOTHVIBE_KEY:
      settings.BluetoothVibe = new_tuple->value->uint8;
      break;      
    case HOURLYVIBE_KEY:
      settings.HourlyVibe = new_tuple->value->uint8;
      break;
    case BRANDING_MASK_KEY:
      settings.BrandingMask = new_tuple->value->uint8;
      layer_set_hidden(bitmap_layer_get_layer(branding_mask_layer), !settings.BrandingMask);
      break;
  }
}
*/
static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;
  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
  if (old_image != NULL) {
	gbitmap_destroy(old_image);
	old_image = NULL;
  }
}

static void update_battery(BatteryChargeState charge_state) {

  batteryPercent = charge_state.charge_percent;

  if(batteryPercent==100) {
	change_battery_icon(false);
	layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
    for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
      layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), true);
    }  
    return;
  }

  layer_set_hidden(bitmap_layer_get_layer(battery_layer), charge_state.is_charging);
  change_battery_icon(charge_state.is_charging);
    
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), false);
  }  
  set_container_image(&battery_percent_image[0], battery_percent_layers[0], TINY_IMAGE_RESOURCE_IDS[charge_state.charge_percent/10], GPoint(89, 43));
  set_container_image(&battery_percent_image[1], battery_percent_layers[1], TINY_IMAGE_RESOURCE_IDS[charge_state.charge_percent%10], GPoint(95, 43));
  set_container_image(&battery_percent_image[2], battery_percent_layers[2], TINY_IMAGE_RESOURCE_IDS[10], GPoint(101, 43));
 
}

static void toggle_bluetooth_icon(bool connected) {
  if(appStarted && !connected) { // && settings.BluetoothVibe) {
    //vibe!
    vibes_long_pulse();
  }
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

void battery_layer_update_callback(Layer *me, GContext* ctx) {        
  //draw the remaining battery percentage
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(2, 2, ((batteryPercent/100.0)*11.0), 5), 0, GCornerNone);
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_week(struct tm *tick_time){
	strftime(week_text, sizeof(week_text), "w%V", tick_time);
	text_layer_set_text(week_layer, week_text);    
}

static void update_days(struct tm *tick_time) {
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint(69, 61));
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(108, 61));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(121, 61));
}

static void update_hours(struct tm *tick_time) {

  /* removed hourly vibe (and it's setting)
  if(appStarted && settings.HourlyVibe) {
    //vibe!
    vibes_short_pulse();
  }
  */
  
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(10, 84));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(40, 84));

  if (!clock_is_24h_style()) {
    if (tick_time->tm_hour >= 12) {
      set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(17, 68));
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
    } 
    else {
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), true);
    }
    
    if (display_hour/10 == 0) {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
    }
    else {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
    }

  }
}
static void update_minutes(struct tm *tick_time) {
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(77, 84));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(105, 84));
}
/* remove seconds handling
static void update_seconds(struct tm *tick_time) {
  if(settings.Blink) {
    layer_set_hidden(bitmap_layer_get_layer(separator_layer), tick_time->tm_sec%2);
  }
  else {
    if(layer_get_hidden(bitmap_layer_get_layer(separator_layer))) {
      layer_set_hidden(bitmap_layer_get_layer(separator_layer), false);
    }
  }
}
*/

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & DAY_UNIT) {
    update_days(tick_time);
    update_week(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT) {
    update_minutes(tick_time);
  }	
/* canceled seconds handling
if (units_changed & SECOND_UNIT) {
    update_seconds(tick_time);
  }		
*/
}

/* removed settings
static void loadPersistentSettings() {	
	valueRead = persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void savePersistentSettings() {
	valueWritten = persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}
*/

// Update Event text - from ModernCalendar
void display_event_text(char *text, char *relative) {
  strncpy(event_text2, relative, sizeof(event_text2));
  text_layer_set_text(event_layer2, event_text2);
  strncpy(event_text, text, sizeof(event_text));
  text_layer_set_text(event_layer, event_text);
}

// Hides status icons. Call draw of default battery/bluetooth icons (which will show or hide icon based on set logic). 
void hide_status() {
	status_showing = false;
  layer_set_hidden(text_layer_get_layer(event_layer), true);
  layer_set_hidden(text_layer_get_layer(event_layer2), true);
//  layer_set_hidden(bitmap_layer_get_layer(meter_bar_layer), false);
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), false);
  layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
  layer_set_hidden(text_layer_get_layer(week_layer), false);
  layer_set_hidden(bitmap_layer_get_layer(battery_image_layer), false);
  layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
  layer_set_hidden(bitmap_layer_get_layer(day_name_layer), false);
  layer_set_hidden(event_status_layer, false);
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(date_digits_layers[i]), false);
  }
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), false);
  }
}

// Shows status icons. Call draw of default battery/bluetooth icons (which will show or hide icon based on set logic).
void show_status() {
  if (event_status_display != STATUS_ALERT_SET) { // in case there is no event to display - do nothing
    return;
  }
  if (status_showing){
    app_timer_cancel(display_timer); // Cancels previous timer if another show_status is called within the 4000ms
  }
	status_showing = true;
//  layer_set_hidden(bitmap_layer_get_layer(meter_bar_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(battery_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(battery_image_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(time_format_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(day_name_layer), true);
  layer_set_hidden(event_status_layer, true);
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(date_digits_layers[i]), true);
  }
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), true);
  }
  layer_set_hidden(text_layer_get_layer(week_layer), true);
//  text_layer_set_text(event_layer2, "In 32 minutes"); // to be removed (just to test)
//  text_layer_set_text(event_layer, "Test event"); // to be removed (just to test)
  layer_set_hidden(text_layer_get_layer(event_layer), false);
  layer_set_hidden(text_layer_get_layer(event_layer2), false);
	// 4 Sec timer then call "hide_status". Cancels previous timer if another show_status is called within the 4000ms
	app_timer_cancel(display_timer);
	display_timer = app_timer_register(4000, hide_status, NULL);
}

// Shake/Tap Handler. On shake/tap... call "show_status"
void tap_handler(AccelAxisType axis, int32_t direction) {
//  app_log(APP_LOG_LEVEL_INFO, "ninety_one_dub_2", 417, "tap handler");
	show_status();	
}

// Status icon callback handler - from ModernCalendar
void event_status_layer_update_callback(Layer *layer, GContext *ctx) {
  
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
	
  if (event_status_display == STATUS_REQUEST) {
     graphics_draw_bitmap_in_rect(ctx, icon_status_1, GRect(0, 0, 17, 9));
  } else if (event_status_display == STATUS_REPLY) {
     graphics_draw_bitmap_in_rect(ctx, icon_status_2, GRect(0, 0, 17, 9));
  } else if (event_status_display == STATUS_ALERT_SET) {
     graphics_draw_bitmap_in_rect(ctx, icon_status_3, GRect(0, 0, 17, 9));
  }
  // graphics_draw_bitmap_in_rect(ctx, icon_status_3, GRect(0, 0, 17, 9)); // remove remark to test graphics, remark when working!
}
// Setting event status - from ModernCalendar
void set_event_status(int new_event_status_display) {
  event_status_display = new_event_status_display;
	layer_mark_dirty(event_status_layer);
}

static void init(void) {
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
  memset(&battery_percent_layers, 0, sizeof(battery_percent_layers));
  memset(&battery_percent_image, 0, sizeof(battery_percent_image));

/* canceled settings
  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);  
*/
  
  window = window_create();
  if (window == NULL) {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
  window_stack_push(window, true /* Animated */);
  window_layer = window_get_root_layer(window);
  
//  loadPersistentSettings();
	
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
  
  separator_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SEPARATOR);
  GRect frame = (GRect) {
    .origin = { .x = 69, .y = 91 },
    .size = separator_image->bounds.size
  };
  separator_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(separator_layer, separator_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(separator_layer));   

  /*
  meter_bar_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_METER_BAR);
  GRect frame2 = (GRect) {
    .origin = { .x = 17, .y = 43 },
    .size = meter_bar_image->bounds.size
  };
  meter_bar_layer = bitmap_layer_create(frame2);
  bitmap_layer_set_bitmap(meter_bar_layer, meter_bar_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(meter_bar_layer));  
*/
  
  bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  GRect frame3 = (GRect) {
    .origin = { .x = 37, .y = 43 },
    .size = bluetooth_image->bounds.size
  };
  bluetooth_layer = bitmap_layer_create(frame3);
  bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
  
  battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  GRect frame4 = (GRect) {
    .origin = { .x = 111, .y = 43 },
    .size = battery_image->bounds.size
  };
  battery_layer = bitmap_layer_create(frame4);
  battery_image_layer = bitmap_layer_create(frame4);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_set_update_proc(bitmap_layer_get_layer(battery_layer), battery_layer_update_callback);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
  
	 //week status layer (ShaBP)
  week_layer = text_layer_create(GRect(55, 38, 27, 14));
  text_layer_set_text_color(week_layer, GColorBlack);
	text_layer_set_text_alignment(week_layer, GTextAlignmentLeft);
  text_layer_set_background_color(week_layer, GColorClear);
  text_layer_set_font(week_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_layer, text_layer_get_layer(week_layer));
  
  //time format
  GRect frame5 = (GRect) {
    .origin = { .x = 17, .y = 68 },
    .size = {.w = 19, .h = 8}
  };
  time_format_layer = bitmap_layer_create(frame5);
  if (clock_is_24h_style()) {
    time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
    bitmap_layer_set_bitmap(time_format_layer, time_format_image);
    
  }
  layer_add_child(window_layer, bitmap_layer_get_layer(time_format_layer));

  // Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
  day_name_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));
  
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
    battery_percent_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(battery_percent_layers[i]));
  }
    
  //mask the pebble branding
  GRect framemask = (GRect) {
    .origin = { .x = 0, .y = 0 },
    .size = { .w = 144, .h = 19 }
  };
  branding_mask_layer = bitmap_layer_create(framemask);
  layer_add_child(window_layer, bitmap_layer_get_layer(branding_mask_layer));
  branding_mask_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BRANDING_MASK);
  bitmap_layer_set_bitmap(branding_mask_layer, branding_mask_image);
  layer_set_hidden(bitmap_layer_get_layer(branding_mask_layer), false);// changed by ShaBP from: !settings.BrandingMask);
  
  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  update_battery(battery_state_service_peek());
/*
  Tuplet initial_values[] = {
    TupletInteger(BLINK_KEY, settings.Blink),
    TupletInteger(INVERT_KEY, settings.Invert),
    TupletInteger(BLUETOOTHVIBE_KEY, settings.BluetoothVibe),
    TupletInteger(HOURLYVIBE_KEY, settings.HourlyVibe),
    TupletInteger(BRANDING_MASK_KEY, settings.BrandingMask)
  };
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, NULL, NULL);
*/   
  appStarted = true;
  
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT);

  // Status icons and status layer creation - from ModernCalendar
  icon_status_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STATUS_1);
  icon_status_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STATUS_2);
  icon_status_3 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STATUS_3);
  GRect sframe;
  sframe.origin.x = 16;
  sframe.origin.y = 43;
  sframe.size.w = 17;
  sframe.size.h = 9;

  event_status_layer = layer_create(sframe);
  layer_set_update_proc(event_status_layer, event_status_layer_update_callback);
  layer_add_child(window_get_root_layer(window), event_status_layer);

  // Event layers init - from ModernCalendar 
  event_layer = text_layer_create(GRect(10, 55, 124, 20));
  text_layer_set_text_color(event_layer, GColorBlack);
  text_layer_set_text_alignment(event_layer, GTextAlignmentCenter);
  text_layer_set_background_color(event_layer, GColorClear);
  text_layer_set_font(event_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(event_layer));
	layer_set_hidden(text_layer_get_layer(event_layer), true);

  event_layer2 = text_layer_create(GRect(10, 38, 124, 20));
  text_layer_set_text_color(event_layer2, GColorBlack);
  text_layer_set_text_alignment(event_layer2, GTextAlignmentCenter);
  text_layer_set_background_color(event_layer2, GColorClear);
  text_layer_set_font(event_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(event_layer2));
	layer_set_hidden(text_layer_get_layer(event_layer2), true);
  
  // Message inbox and Calendar init - from ModernCalendar
  app_message_register_inbox_received(received_message);
  app_message_open(124, 256);

  calendar_init();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(&update_battery);
  accel_tap_service_subscribe(tap_handler);

}


static void deinit(void) {

//  savePersistentSettings(); //canceled settings!

//  app_sync_deinit(&sync);
  
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
	accel_tap_service_unsubscribe();

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  background_image = NULL;
  
  layer_remove_from_parent(bitmap_layer_get_layer(branding_mask_layer));
  bitmap_layer_destroy(branding_mask_layer);
  gbitmap_destroy(branding_mask_image);
  branding_mask_image = NULL;

  layer_remove_from_parent(bitmap_layer_get_layer(separator_layer));
  bitmap_layer_destroy(separator_layer);
  gbitmap_destroy(separator_image);
  separator_image = NULL;
  
  /*
  layer_remove_from_parent(bitmap_layer_get_layer(meter_bar_layer));
  bitmap_layer_destroy(meter_bar_layer);
  gbitmap_destroy(meter_bar_image);
  background_image = NULL;
	*/
  
  layer_remove_from_parent(bitmap_layer_get_layer(bluetooth_layer));
  bitmap_layer_destroy(bluetooth_layer);
  gbitmap_destroy(bluetooth_image);
  bluetooth_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(battery_layer));
  bitmap_layer_destroy(battery_layer);
  gbitmap_destroy(battery_image);
  battery_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
  bitmap_layer_destroy(battery_image_layer);

  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);
  time_format_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
  day_name_image = NULL;
	
  layer_remove_from_parent(text_layer_get_layer(week_layer));
  text_layer_destroy(week_layer);
  week_layer = NULL;
	
  // from ModernCalendar
  
  layer_remove_from_parent(text_layer_get_layer(event_layer));
  text_layer_destroy(event_layer);
  event_layer = NULL;
	
  layer_remove_from_parent(text_layer_get_layer(event_layer2));
  text_layer_destroy(event_layer2);
  event_layer2 = NULL;
	
  layer_remove_from_parent(event_status_layer);
  gbitmap_destroy(icon_status_1); 
  gbitmap_destroy(icon_status_2); 
  gbitmap_destroy(icon_status_3); 
  layer_destroy(event_status_layer);
  event_status_layer = NULL;
	
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    date_digits_images[i] = NULL;
    bitmap_layer_destroy(date_digits_layers[i]);
	date_digits_layers[i] = NULL;
  }

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    time_digits_images[i] = NULL;
    bitmap_layer_destroy(time_digits_layers[i]);
	time_digits_layers[i] = NULL;
  }

  for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(battery_percent_layers[i]));
    gbitmap_destroy(battery_percent_image[i]);
    battery_percent_image[i] = NULL;
    bitmap_layer_destroy(battery_percent_layers[i]); 
	battery_percent_layers[i] = NULL;
  } 
	
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	
  //window_destroy(window);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}