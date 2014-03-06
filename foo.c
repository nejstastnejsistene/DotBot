#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/input.h>


#define CAPLEN (ABS_CNT / 8)
#define HAS_CAP(caps, cap) ((caps)[(cap) / 8] & 1 << ((cap) % 8))

typedef unsigned char capabilities_t[CAPLEN];

int main() {
    int fd = open("/dev/input/event0", O_RDONLY);

    capabilities_t caps;
    if (ioctl(fd, EVIOCGBIT(EV_ABS, CAPLEN), caps) != CAPLEN) {
        fprintf(stderr, "Error reading device capabilities.\n");
        exit(1);    
    }

    if (HAS_CAP(caps, ABS_MT_POSITION_X) &&
            HAS_CAP(caps, ABS_MT_POSITION_Y) &&
            HAS_CAP(caps, ABS_MT_TRACKING_ID)) {
        printf("Multitouch B\n");
    } else if (HAS_CAP(caps, ABS_X) && HAS_CAP(caps, ABS_Y)) {
        printf("Singletouch\n");
    }

    close(fd);
    return 0;
}
