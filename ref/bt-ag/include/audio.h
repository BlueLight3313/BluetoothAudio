#ifndef AUDIO_H
#define AUDIO_H

void audio_init();
int audio_capture(char *buf, int size);
void audio_play(char *buf, int size);

#endif