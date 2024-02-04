#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#define MAX_FUNC_LENGTH 10
#define MAX_ARGC 10
#define MAX_ARG_LENGTH 10

#define text_file "code.txt"

const int MOTOR_1 = 17;
const int MOTOR_2 = 16;

const int LED = 22;
const int LED_ERR = 26;
const int LED_BLINKEN = PICO_DEFAULT_LED_PIN;

int blinken_status = 1;

// TODO: load after flashing
const char *code =
    "blink(0.5, 5)\n"
    "motorson()\n"
    "wait(10)\n"
    "motorsoff()\n";

void blinken_das_light(void) {
    gpio_put(LED_BLINKEN, blinken_status);
    blinken_status = !blinken_status;
    sleep_ms(500);
}

// Takes in a line of text, translates into appropriate arduino functions
void translate_line(char* line, int argc) {
    blinken_das_light();
    char* func = calloc(MAX_FUNC_LENGTH, sizeof(char));
    char** args = malloc(MAX_ARGC*sizeof(char*));

    for (int i = 0; i < MAX_ARGC; i++) {
        args[i] = malloc(MAX_ARG_LENGTH);
    }

    int arg_curr = 0;
    int arg_len = 0;
    
    bool end_func = false;

    if (line == NULL) { 
        return;
    }

    for (int i = 0; i < strlen(line); i++) {
        if ((line[i] != '(' ) && (line[i] != ')')) {
            // appends to the function, or the arguments, depending on relative position of ( , )
            if (!end_func) {
                func[i] = line[i]; 
            } else if (line[i] == ',') {
                arg_curr++;
                arg_len = 0;
            } else if (line[i] != ' ') {
                args[arg_curr][arg_len] = line[i];
                arg_len++;
            }
        } else if (line[i] == '(') {
            // We are finished getting the string for the function 
            end_func = true;
        } 
    }

  // Deciding what to do based on func & args

    // Blink receives single arg float seconds, number of times
    if (!strcmp(func, "blink")) {
        int delay = atof(args[0]) * 1000;
        int blinks = atoi(args[1]);
        for (int i = 0; i < blinks; i++) {
            gpio_put(LED, 1);
            sleep_ms(delay);
            gpio_put(LED, 0);
            sleep_ms(delay);
        }
        //printf("BLINKING BLINKING %d FOR %d MILLISECONDS! \n", blinks, delay);
    } 
    // LightOn receives no args 
    else if (!strcmp(func, "lighton")) {
        gpio_put(LED,1);
        //printf("LIGHT ON\n");
    } 
    // LightOff receives no args
    else if (!strcmp(func, "lightoff")) {
        gpio_put(LED,0);
        //printf("LIGHT OFF\n");
    }
    // Buzz recieves two args - int freq (in Hz), float milliseconds
    else if (!strcmp(func, "buzz")) {
        int freq = atoi(args[0]);
        int duration = atof(args[1]) * 1000;
        //printf("BUZZ BUZZ %d HZ FOR %d MILLISECONDS! \n", freq,duration);
    }
    // Wait receives one arg - float seconds
    else if (!strcmp(func, "wait")) {
        int delay = atof(args[0])*1000;
        sleep_ms(delay);
        //printf("WAITING FOR %d MILLISECONDS! \n", delay);
    }
    else if (!strcmp(func, "motorson")) {
        gpio_put(MOTOR_1, 1);
        gpio_put(MOTOR_2, 1);
    }
    else if (!strcmp(func, "motorsoff")) {
        gpio_put(MOTOR_1, 0);
        gpio_put(MOTOR_2, 0);
    }
    else if (!strcmp(func, "motoron")) {
        int id = atoi(args[0]);
        gpio_put(id==1 ? MOTOR_1 : MOTOR_2, 1);
    }
    else if (!strcmp(func, "motoroff")) {
        int id = atoi(args[0]);
        gpio_put(id==1 ? MOTOR_1 : MOTOR_2, 0);
    }
    else {
        gpio_put(LED_ERR, 1);
    }
      
    return;
}

int main() {
    //FILE *infname = fopen(text_file, "r"); 
    //char *line = NULL; 
    //size_t len = 0;
    //ssize_t read;

    gpio_init(MOTOR_1);
    gpio_init(MOTOR_2);
    gpio_init(LED);
    gpio_init(LED_ERR);
    gpio_init(LED_BLINKEN);

    gpio_set_dir(MOTOR_1, GPIO_OUT);
    gpio_set_dir(MOTOR_2, GPIO_OUT);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_set_dir(LED_ERR, GPIO_OUT);
    gpio_set_dir(LED_BLINKEN, GPIO_OUT);

    gpio_put(LED, 1);

    stdio_init_all();

    blinken_das_light();

    // "rp2040 doesn't have have input/output direction distinction"
    // you can learn this misinformation and more!
   
    // Turn the light on with long delay to indicate failure
//    if (infname == NULL) {
//        gpio_put(LED_ERR, 1);
//    }

    // Do some translating stuff to get the new lines:
    // while (true) 
    //
    // temp inc to not run forever during testing, uncomment while(true) when sending to bot
    int j = 1;
    while (true) { 
        int code_idx = 0;
        while (code_idx < strlen(code)) {
            int linelen = 0;
            for (; code[code_idx+linelen] != '\n'; linelen++) 0;
            char *line = malloc(linelen+1);
            strncpy(line, code+code_idx, linelen);
            code_idx += linelen + 1;
            ////printf("Retrieved line of length %zu \n", read);
            int argc = 0;

            for (int i=0; i<strlen(line); i++) {
                // First is for at least 1 argument, others all preceded by comma
                if ((line[i] == '(') && (line[i+1] != ')') ) {
                    argc++;
                } else if (line[i] == ',') {
                    argc++;
                }
            }
            translate_line(line, argc);
            free(line);
        }
        j--;
    }
}
