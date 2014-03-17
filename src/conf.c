#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "conf.h"


/* Get information about the touchscreen. */
void get_touchscreen(screen_conf_t *conf) {
    if (access(confpath, R_OK) == 0) {
        read_config(conf);
    } else {
        update_config(conf);
    }
}


void read_config(screen_conf_t *conf) {
    
    /* Attempt to open up the config file for reading. If unable, get
     * the information the hard way.
     */
    FILE *f = fopen(confpath, "r");
    if (f == NULL) {
        perror("Unable to read config file");
        update_config(conf);
    }

    /* Read each row from the config file. */
    int success = 0;
    success |= fscanf(f, devicename_fmt, conf->path)        == 1;
    success |= fscanf(f, screentype_fmt, (int*)&conf->type) == 1;
    success |= fscanf(f, xmin_fmt, &conf->xmin)             == 1;
    success |= fscanf(f, xmax_fmt, &conf->xmax)             == 1;
    success |= fscanf(f, ymin_fmt, &conf->ymin)             == 1;
    success |= fscanf(f, ymax_fmt, &conf->ymax)             == 1;
    fclose(f);

    /* If the format differed from what was expected int he slightest,
     * lookup the information again and rewrite the config file.
     */
    if (success != 1) {
        update_config(conf);
    }
}


void update_config(screen_conf_t *conf) {
    
    /* Figure out the screen information. */
    find_touchscreen(conf);

    /* Attempt to open up the file for writing. */
    FILE *f = fopen(confpath, "w+");
    if (f == NULL) {
        perror("Unable to write config file");
    }

    /* Write the config file. */
    fprintf(f, devicename_fmt, conf->path);
    fprintf(f, screentype_fmt, conf->type);
    fprintf(f, xmin_fmt, conf->xmin);
    fprintf(f, xmax_fmt, conf->xmax);
    fprintf(f, ymin_fmt, conf->ymin);
    fprintf(f, ymax_fmt, conf->ymax);
    fclose(f);
}


/* Find which device is the touchscreen and other relevant information.
 * Sets conf->type to INVALID if it fails to find a touchscreen.
 */
void find_touchscreen(screen_conf_t *conf) {
    char *filename;
    DIR *dir;
    struct dirent *de;
    const char *dirname = "/dev/input";

    /* Try to open /dev/input to list the available input devices. */
    dir = opendir(dirname);
    if (dir == NULL) {
        perror(dirname);
        conf->type = INVALID;
        return;
    }

    /* Create a pointer to the end of the path name, so we can place
     * filenames there.
     */
    strcpy(conf->path, dirname);
    filename = conf->path + strlen(conf->path);
    *filename++ = '/';

    /* Check each file in the directory. */
    while ((de = readdir(dir))) {
        if (strcmp(de->d_name, ".") == 0 ||
                strcmp(de->d_name, "..") == 0) {
            continue;
        }

        /* Attempt to open the file. If unable, just skip it. */
        strcpy(filename, de->d_name);
        int fd = open(conf->path, O_RDONLY);
        if (fd < 0) {
            perror(conf->path);
            continue;
        } 

        /* Look up information about the current input device. If the
         * type is not INVALID, we've found a suitable device!
         */
        get_screen_info(fd, conf);
        close(fd);
        if (conf->type != INVALID) {
            break;
        }
    }
}


/* Look at a device's capalities to decide if it's a touchscreen. Also,
 * determine the touchscreen's type and the range of values it accepts.
 */
void get_screen_info(int fd, screen_conf_t *conf) {

    /* Read the available EV_ABS codes. */
    capabilities_t caps;
    if (ioctl(fd, EVIOCGBIT(EV_ABS, CAPLEN), caps) != CAPLEN) {
        fprintf(stderr, "Error reading device capabilities.\n");
        conf->type = INVALID;
        return;
    }

    struct input_absinfo abs;

    /* Multi-touch B. */
    if (HAS_CAP(caps, ABS_MT_POSITION_X) &&
            HAS_CAP(caps, ABS_MT_POSITION_Y) &&
            HAS_CAP(caps, ABS_MT_TRACKING_ID)) {
        conf->type = MULTI_TOUCH_B;
        ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &abs);
        conf->xmin = abs.minimum;
        conf->xmax = abs.maximum;
        ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs);
        conf->ymin = abs.minimum;
        conf->ymax = abs.maximum;

    /* Single-touch. */
    } else if (HAS_CAP(caps, ABS_X) && HAS_CAP(caps, ABS_Y)) {
        conf->type = SINGLE_TOUCH;
        ioctl(fd, EVIOCGABS(ABS_X), &abs);
        conf->xmin = abs.minimum;
        conf->xmax = abs.maximum;
        ioctl(fd, EVIOCGABS(ABS_Y), &abs);
        conf->ymin = abs.minimum;
        conf->ymax = abs.maximum;

    /* TODO: Multi-touch A */
    } else {
        conf->type = INVALID;
    }
}


/*int main() {
    screen_conf_t conf;
    get_touchscreen(&conf);
    printf("path=%s, type=%d\n", conf.path, conf.type);
    return 0;
}*/
