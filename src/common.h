#ifndef common_h
#define common_h

#include <pebble.h>
	
//#define INVERSE

#define RECONNECT_KEY 0
#define REQUEST_CALENDAR_KEY 1
#define CLOCK_STYLE_KEY 2
#define CALENDAR_RESPONSE_KEY 3
#define ALERT_EVENT 10

#define CLOCK_STYLE_12H 1
#define CLOCK_STYLE_24H 2
	
#define MAX_EVENTS 15

#define STATUS_REQUEST 1
#define STATUS_REPLY 2
#define STATUS_ALERT_SET 3
	
#define	MAX_ALLOWABLE_ALERTS 10
	
typedef struct {
  uint8_t index;
  char title[21];
  bool has_location;
  char location[21];
  bool all_day;
  char start_date[18];
  int32_t alarms[2];
} Event;

typedef struct {
  AppTimer *handle;
  char event_desc[21];
  bool active;
  char relative_desc[21];
  char location[21];
} TimerRecord;

#define REQUEST_CALENDAR_INTERVAL_MS 600003
#define REQUEST_STEPS_INTERVAL_MS 90000
  
#define REQUEST_STEP_COUNTER_KEY 29
#define STEP_COUNTER_RESPONSE_KEY 30
#define STEP_COUNTER_CHANGE_KEY 31

typedef struct {
  int number_of_steps;
  int goal;
  int goal_percentage;
} StepCounter;

void calendar_init();
void steps_init();
void handle_calendar_timer(void *cookie);
void handle_steps_timer(void *cookie);
void display_event_text(char *text, char *relative, char *location);
void display_steps_text(int num_steps);
void display_goal_text(int goal_perc);
void show_status();
//void display_goal_percent(int goal_perc);

//void draw_date();
void received_message(DictionaryIterator *received, void *context);
void set_event_status(int new_status_display);

#endif