#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <switch.h>


#define CONFIG_FILE "nxcgol.config.txt"     ///<config file
#define ROWS 46                             ///<total rows
#define COLS 81                             ///< t
#define HISTORY_SIZE 8                      ///<retain x generations for comparison


#define CELL 0xDB                           ///<solid block cp437
#define UP   0x18                           ///<up arrow cp437
#define DOWN 0x19                           ///<down arrow cp437

#define ERROR_RED CONSOLE_ESC(31m)
#define CONTROLS_BLUE CONSOLE_ESC(38;2;135;206;235m)
#define RESET CONSOLE_ESC(0m)
#define CONSOLE_ESC_NSTR(fmt) "\033[" fmt   ///< console escape helper, does not stringify.


/**
 * @defgroup config Config
 * @{
 */
typedef struct {
    int red;
    int green;
    int blue;                               
    char ansi[32];                          ///<ansi escape representation
} Color;


typedef struct {
    int simulation_speed;                   ///<between 1 and 100 (%)
    int density;                            ///<between 1 and 100
    bool show_stats;                        ///<last row statistics
    bool auto_restart;                      ///<restart on static
    int stagnant_wait;                      ///<wait x generations after colony goes stagnant before restarting
    bool colorful;                          ///<use colors or b/w
    Color stable;
    Color growth;
    Color dense;
    Color sparce;
} Config;

/** 
 * @brief default configuration
 */
Config config = {
    .simulation_speed = 100,
    .density = 30,
    .show_stats = false,
    .auto_restart = true,
    .stagnant_wait = 100,
    .colorful = true,
    .stable = {86, 252, 3},
    .growth = {3, 252, 219},
    .dense = {252, 231, 3},
    .sparce = {252, 3, 3},
};

bool matrix[COLS][ROWS];
bool matrix_next[COLS][ROWS];

bool history[HISTORY_SIZE][COLS][ROWS];
int history_index = 0;                  ///<generations in history, for comparison to check for oscillation

int generation_index = 0;               ///<generation number
int restart_index = 0;                  ///<colony number
int stagnant_index = 0;                 ///<number of generations colony has been stagnant

/**
 * @brief parse r,g,b string into Color
 */
bool parse_rgb(const char* value, Color* color) {
    int r, g, b;
    if (sscanf(value, "%d,%d,%d", &r, &g, &b) == 3) {
        color->red = r;
        color->green = g;
        color->blue = b;
        return true;
    }
    return false;
}

/**
 * @brief parse configuration file
 * @param config populates Config config
 */
void parse_config_file(Config* config) {
    bool error = false;
    printf("Reading %s\n", CONFIG_FILE);
    FILE* file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        printf(CONSOLE_ESC(45; 1H) ERROR_RED "%s not found, using defaults..." RESET, CONFIG_FILE);
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        char* sep = strchr(line, ':');
        if (!sep) continue;

        *sep = '\0';
        char* key = line;
        char* value = sep + 1;

        while (*value == ' ') value++;

        printf("%s:%s\n", key, value);
        consoleUpdate(NULL);

        if (strcmp(key, "simulation_speed") == 0) {
            config->simulation_speed = atoi(value);
            if (config->simulation_speed < 1 || config->simulation_speed > 100) {
                error = true;
                config->simulation_speed = 100;
            }
        }
        else if (strcmp(key, "starting_density") == 0) {
            config->density = atoi(value);
            if (config->density < 1 || config->density > 100) {
                error = true;
                config->density = 30;
            }
        }
        else if (strcmp(key, "show_stats") == 0) {
            if (strlen(value) < 1) {
                error = true;
                config->show_stats = true;
            }
            else {
                config->show_stats = (strcmp(value, "true") == 0) ? true : false;
            }
        }
        else if (strcmp(key, "auto_restart") == 0) {
            if (strlen(value) < 1) {
                error = true;
                config->auto_restart = true;
            }
            else {
                config->auto_restart = (strcmp(value, "true") == 0) ? true : false;
            }
        }
        if (strcmp(key, "stagnant_wait") == 0) {
            config->stagnant_wait = atoi(value);
            if (config->stagnant_wait < 0 || config->simulation_speed > 1000) {
                error = true;
                config->stagnant_wait = 100;
            }
        }
        else if (strcmp(key, "colorful") == 0) {
            if (strlen(value) < 1) {
                error = true;
                config->show_stats = true;
            }
            else {
                config->colorful = (strcmp(value, "true") == 0) ? true : false;
            }
        }
        else if (strcmp(key, "stable_color") == 0) {
            if (strlen(value) < 1) {
                error = true;
            }
            else {
                if (!parse_rgb(value, &config->stable)) {
                    error = true;
                }
            }
        }
        else if (strcmp(key, "growth_color") == 0) {
            if (strlen(value) < 1) {
                error = true;
            }
            else {
                if (!parse_rgb(value, &config->growth)) {
                    error = true;
                }
            }
        }
        else if (strcmp(key, "dense_color") == 0) {
            if (strlen(value) < 1) {
                error = true;
            }
            else {
                if (!parse_rgb(value, &config->dense)) {
                    error = true;
                }
            }
        }
        else if (strcmp(key, "sparce_color") == 0) {
            if (strlen(value) < 1) {
                error = true;
            }
            else {
                if (!parse_rgb(value, &config->sparce)) {
                    error = true;
                }
            }
        }
    }
    fclose(file);
    if (error) {
        printf(CONSOLE_ESC(45;1H) ERROR_RED "An error occurred when parsing lines. Some values will default." RESET);
    }
    else {
        printf(CONSOLE_ESC(45;1H) "File parsing completed");
    }
}
/** @} **/


/**
 * @defgroup helpers Helpers
 * @{
 */

/**
 * @brief calculate simulation speed based on percentage
 * at 100(%) the usleep time will be 1ms
 * at 1(%) the usleep time will be 100ms
 */
int sim_speed_calc(int percent) {
    if (percent > 100) percent = 100;
    if (percent < 1) percent = 1;

    double usleep_val = 1000.0 + ((100.0 - percent) * 99000.0 / 99.0);
    return (int)round(usleep_val);
}

/**
 * @brief convert r,g,c from Color to ansi escape sequence
 */
void color_to_ansi(Color* c) {
    snprintf(c->ansi, sizeof(c->ansi), "\x1b[38;2;%d;%d;%dm", c->red, c->green, c->blue);
}

/**
 * @brief if show_stats is true we decrement 1 from rows to make room for stats
 * we do this when iterating through the rows
 */
int active_rows() {
    return config.show_stats ? ROWS - 1 : ROWS;
}
/** @} **/

/**
 * @defgroup logic Simulation logic
 * @{
 */

 /**
  * @brief get the neighbors of this cell
  */
int count_neighbors(int x, int y) {
    int sum = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;   //skip self
            int ni = (x + dx + COLS) % COLS;    //wrapped x
            int nj = (y + dy + ROWS) % ROWS;    //wrapped y
            sum += matrix[ni][nj];              //1 if alive, 0 if dead
        }
    }
    return sum;
}

/**
 * @brief generate cells for new colony
 */
void randomize_matrix() {
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            matrix[i][j] = (rand() % 100) < config.density;
        }
    }
}

/**
 * @brief reset history, generation_index and increment restart_index, generate new colony.
 */
void restart_simulation() {
    randomize_matrix();
    memset(history, 0, sizeof(history));
    history_index = 0;
    generation_index = 0;
    stagnant_index = 0;
    restart_index++;
}

int main(int argc, char** argv)
{
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    srand(time(NULL));

    //load config
    parse_config_file(&config);
    consoleUpdate(NULL);
    sleep(3);

    restart_simulation();

    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        //exit
        if (kDown & HidNpadButton_Plus) {
            break;
        }

        //restart simulation
        if (kDown & HidNpadButton_Minus) {
            restart_simulation();
        }

        u64 simulation_speed;
        //lower sim speed
        if (kDown & HidNpadButton_Down) {
            if (config.simulation_speed - 1 > 0) {
                config.simulation_speed -= 1;
                simulation_speed = sim_speed_calc(config.simulation_speed);
            }
        }
        //raise sim speed
        if (kDown & HidNpadButton_Up) {
            if (config.simulation_speed + 1 <= 100) {
                config.simulation_speed += 1;
                simulation_speed = sim_speed_calc(config.simulation_speed);
            }
        }
        //set up colors
        color_to_ansi(&config.stable);
        color_to_ansi(&config.dense);
        color_to_ansi(&config.sparce);
        color_to_ansi(&config.growth);

        //set initial simulation speed
        simulation_speed = sim_speed_calc(config.simulation_speed);

        //for show stats
        int max_rows = active_rows();

        for (int i = 0; i < COLS; i++) {
            for (int j = 0; j < max_rows; j++) {
                if (matrix[i][j]) {
                    int neighbors = count_neighbors(i, j);

                    if (config.colorful) {
                        const char* color;
                        switch (neighbors) {
                        case 2: color = config.stable.ansi; break;
                        case 3: color = config.growth.ansi; break;
                        case 4: color = config.dense.ansi; break;
                        default: color = config.sparce.ansi; break;
                        }
                        printf(CONSOLE_ESC_NSTR("%d;%dH") "%s%c\033[0m", j, i, color, CELL);
                    }
                    else {
                        printf(CONSOLE_ESC_NSTR("%d;%dH") "%c", j, i, CELL);
                    }
                }
                else {
                    printf(CONSOLE_ESC_NSTR("%d;%dH") " ", j, i);
                }
            }

        }
        if (config.show_stats) {
            printf(CONSOLE_ESC(45;1H));     //move cursor
            printf(CONSOLE_ESC(2K));        //clear line
            printf(CONSOLE_ESC(45;1H) CONTROLS_BLUE "[-] " RESET "New Colony" CONTROLS_BLUE " [%c/%c] " RESET "Change speed (%i%%)" CONTROLS_BLUE " [+] " RESET "Exit | Generation: %i/%i", UP, DOWN, config.simulation_speed, restart_index, generation_index);
        }

        usleep(simulation_speed);

        // compute next generation
        for (int i = 0; i < COLS; i++) {
            for (int j = 0; j < max_rows; j++) {
                int neighbors = count_neighbors(i, j);
                if (matrix[i][j]) {
                    // live cell dies if it has 2 or 3 live neighbors
                    matrix_next[i][j] = !(neighbors < 2 || neighbors > 3);
                }
                else {
                    // dead cell becomes alive if 3 neighbors
                    matrix_next[i][j] = (neighbors == 3);
                }
            }
        }

        memcpy(matrix, matrix_next, sizeof(matrix));
        generation_index++;

        if (config.auto_restart) {
            // check for static state
            bool oscillating = true;
            for (int h = 0; h < HISTORY_SIZE; h++) { 
                if (h == history_index) continue;
                if (memcmp(matrix, history[h], sizeof(matrix)) == 0) {
                    oscillating = false;
                    break;
                }
            }

            // save to history
            memcpy(history[history_index], matrix, sizeof(matrix));
            history_index = (history_index + 1) % HISTORY_SIZE;

            //if nothing fun is happening restart
            if (!oscillating) {
                stagnant_index++;
                if (stagnant_index >= config.stagnant_wait) {
                    restart_simulation();
                }
            }
            else {
                stagnant_index = 0;
            }
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
/** @} **/