#ifndef __GST_LADSPA_WRAPPER_H__
#define __GST_LADSPA_WRAPPER_H__

#include <ladspa.h>

#define BUFFER_SIZE 2048

typedef struct {
	  void *dl;
	  LADSPA_Descriptor_Function pdf;
	  const LADSPA_Descriptor *pd;
} ladspa_plugin;

typedef LADSPA_Data ladspa_buffer[BUFFER_SIZE];

typedef struct {
  LADSPA_Handle ph;
  ladspa_buffer *inputs;
  ladspa_buffer *outputs;
  LADSPA_Data *param;
  ladspa_plugin *plugin;
} ladspa_wrapper;

int ladspa_load_plugin(const char *filename, ladspa_plugin *plugin);
int ladspa_init_plugin(ladspa_plugin *plugin, ladspa_wrapper *wrapper);
void ladspa_run_plugin(ladspa_wrapper *wrapper, size_t len, unsigned char *buffer);

#endif /* __GST_LADSPA_WRAPPER_H__ */
