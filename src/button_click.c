/*
 * main.c
 * Creates a Window and TextLayer, then subscribes to wakeup events.
 */

#include <pebble.h>

#define WAKEUP_REASON 0
#define PERSIST_KEY_WAKEUP_ID 42
#define REST 1
#define INTERVAL 0

static int rest_or_interval;
static Window *s_main_window;
static TextLayer *s_output_layer;

static WakeupId s_wakeup_id;

static char type[] = "medium";
static int init_time;
static int rest_time;
static int interval_num;

static int get_rest_time(char *type) {
  if(!strcmp("easy", type))
    return 10;
  else if(!strcmp("medium", type))
    return 15;
  else if(!strcmp("hard", type))
    return 20;
  return 0;
}

static int get_init_time(char *type) {
  if(!strcmp("easy", type))
    return 20;
  else if(!strcmp("medium", type))
    return 40;
  else if(!strcmp("hard", type))
    return 60;
  return 0;
}

static void check_wakeup() {
  // Get the ID
  s_wakeup_id = persist_read_int(PERSIST_KEY_WAKEUP_ID);

  if (s_wakeup_id > 0) {
    // There is a wakeup scheduled soon
    time_t timestamp = 0;
    wakeup_query(s_wakeup_id, &timestamp);
    int seconds_remaining = timestamp - time(NULL);

    // Show how many seconds to go
    static char s_buffer[64];
    snprintf(s_buffer, sizeof(s_buffer), "%d seconds Left!", seconds_remaining);
    text_layer_set_text(s_output_layer, s_buffer);
  }
}

static void wakeup_handler(WakeupId id, int32_t reason) {
  // Delete the ID
  persist_delete(PERSIST_KEY_WAKEUP_ID);
  
  if(interval_num == 1) {
    text_layer_set_text(s_output_layer, "You are done, Good Work!");
    rest_or_interval = 3;
  }
  
  if(rest_or_interval == REST)
    rest_or_interval = INTERVAL;
  else if(rest_or_interval == INTERVAL)
    rest_or_interval = REST;
  
  if(rest_or_interval == REST) {
    // Current time + rest_time seconds
    time_t future_time = time(NULL) + rest_time;
  
    // Schedule wakeup event and keep the WakeupId
    s_wakeup_id = wakeup_schedule(future_time, WAKEUP_REASON, true);
    persist_write_int(PERSIST_KEY_WAKEUP_ID, s_wakeup_id);
  
    // Prepare for waking up later
    static char s_buffer[64];
    snprintf(s_buffer, sizeof(s_buffer), "Rest will end in %d seconds.", rest_time);
    text_layer_set_text(s_output_layer, s_buffer); 
  }
  else if (rest_or_interval == INTERVAL ){
     // Current time + init_time seconds
    time_t future_time = time(NULL) + init_time;
  
    // Schedule wakeup event and keep the WakeupId
    s_wakeup_id = wakeup_schedule(future_time, WAKEUP_REASON, true);
    persist_write_int(PERSIST_KEY_WAKEUP_ID, s_wakeup_id);
  
    // Prepare for waking up later
    static char s_buffer[64];
    snprintf(s_buffer, sizeof(s_buffer), "Interval will end in %d seconds.", init_time);
    text_layer_set_text(s_output_layer, s_buffer);
    if(interval_num > 0)
      interval_num -= 1;
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Check the event is not already scheduled
  if (!wakeup_query(s_wakeup_id, NULL)) {
    // Current time + init_time seconds
    time_t future_time = time(NULL) + init_time;
    rest_or_interval = INTERVAL;
    // Schedule wakeup event and keep the WakeupId
    s_wakeup_id = wakeup_schedule(future_time, WAKEUP_REASON, true);
    persist_write_int(PERSIST_KEY_WAKEUP_ID, s_wakeup_id);

    // Prepare for waking up later
    static char s_buffer[64];
    snprintf(s_buffer, sizeof(s_buffer), "Interval will end in %d seconds.", init_time);

    text_layer_set_text(s_output_layer, s_buffer);
  } else {
    // Check existing wakeup
    check_wakeup();
  }
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // get interval and rest times
  init_time = get_init_time(type);
  rest_time = get_rest_time(type);
  rest_or_interval = 3;
  interval_num = 2;
  
  // Create output TextLayer
  s_output_layer = text_layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_text(s_output_layer, "Press SELECT to start");
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Subscribe to Wakeup API
  wakeup_service_subscribe(wakeup_handler);

  // Was this a wakeup launch?
  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    // The app was started by a wakeup
    WakeupId id = 0;
    int32_t reason = 0;

    // Get details and handle the wakeup
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  } else {
    // Check whether a wakeup will occur soon
    check_wakeup();
  }
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}