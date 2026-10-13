#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "TimeParser.h"

int time_parse(char *time)
{
    // Missing string
    if (time == NULL) {
        return TIME_ARRAY_ERROR;
    }

    // Empty string
    if (time[0] == '\0') {
        return TIME_ARRAY_ERROR;
    }

    // Check that string is six characters
    if (strlen(time) != 6) {
        return TIME_LEN_ERROR;
    }

    // All characters numbers
    for (int i = 0; i < 6; i++) {
        if (!isdigit((unsigned char)time[i])) {
            return TIME_ARRAY_ERROR;
        }
    }

    // Parse values
    int hours =
        (time[0] - '0') * 10 + (time[1] - '0');

    int minutes =
        (time[2] - '0') * 10 + (time[3] - '0');

    int seconds =
        (time[4] - '0') * 10 + (time[5] - '0');

    // Check borders
    if (hours > 23 ||
        minutes > 59 ||
        seconds > 59) {
        return TIME_VALUE_ERROR;
    }

    // Counting only minutes and seconds
    int total_seconds = minutes * 60 + seconds;

    // Timing zero not needed
    if (total_seconds == 0) {
        return TIME_ZERO_ERROR;
    }

    return total_seconds;
}