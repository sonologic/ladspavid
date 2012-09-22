#ifndef __GST_LADSPA_WRAPPER_H__
#define __GST_LADSPA_WRAPPER_H__

#include <ladspa.h>

#define BUFFER_SIZE 2048

typedef struct {
  void *dl;
  LADSPA_Handle ph;
  LADSPA_Descriptor_Function pdf;
  const LADSPA_Descriptor *pd;
  LADSPA_Data input[BUFFER_SIZE];
  LADSPA_Data output[BUFFER_SIZE];
  LADSPA_Data delay;
  LADSPA_Data max_slow;
  LADSPA_Data lfo;
  LADSPA_Data feedback;
} ladspa_wrapper;

int ladspa_init_plugin(const char *filename, ladspa_wrapper *wrapper);
void ladspa_run_plugin(ladspa_wrapper *wrapper, size_t len, unsigned char *buffer);

#endif /* __GST_LADSPA_WRAPPER_H__ */
