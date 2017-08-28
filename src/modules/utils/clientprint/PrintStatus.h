#ifndef PRINTSTATUS
#define PRINTSTATUS

// If you modify this list, remember to update the constant CP_STATE in ClientPrint.cpp
typedef enum {cp_idle, cp_printing, cp_paused, cp_eof} client_printing_state_t;
typedef enum {pe_begin, pe_pause, pe_resume, pe_end, pe_eof} print_event_t;
typedef enum {PS_SD_CARD, PS_CLIENT_PRINT, PS_UNKNOWN} print_source_t;

typedef struct {
//    client_printing_state_t old_status;   // temporarily disabled this
    client_printing_state_t new_status;
    print_source_t print_source;          // temporarily disabled this
    print_event_t event;
    bool event_was_estimated;   // true if the event was determined by an heuristic, and is not 100% confirmed
    unsigned int total_print_time;	// if event = pe_end, this stores the total time the print took in seconds.
//    const char* filename;
} print_status_change_t;

#endif // PRINTSTATUS
