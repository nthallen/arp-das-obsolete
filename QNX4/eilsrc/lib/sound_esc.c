#include <stdio.h>
#include <stdlib.h>

int sound_esc(int fd, unsigned freq, unsigned dur) {
    char buf[12];

    if(dur > (255 - ' ') * 50) return -1;
    if(freq > 192 * 192) return -1;
    sprintf(buf, "\033s%c%c%c\007\033s   ",
        ' ' + (dur + 49) / 50,
        ' ' + freq % 192,
        ' ' + freq / 192);
    write(fd, buf, 11);
    return 0;
}
