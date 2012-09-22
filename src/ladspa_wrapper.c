#include <ladspa.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ladspa_wrapper.h"

#define BUFFER_SIZE 2048

int ladspa_load_plugin(const char *filename, ladspa_plugin *plugin) {
	int rv = -1;
	int i;

	plugin->dl = dlopen(filename, RTLD_NOW);

	if (plugin->dl == NULL )
		goto exit;

	printf("dlp: 0x%x\n", plugin->dl);

	  plugin->pdf = (LADSPA_Descriptor_Function)dlsym(plugin->dl, "ladspa_descriptor");

	  if(plugin->pdf==NULL) goto exitdl;

	  printf("pdf: 0x%x\n",plugin->pdf);

	  for(i=0;;i++) {
		plugin->pd = plugin->pdf(i);
	    if(plugin->pd==NULL) break;
	    printf("%i: 0x%x - %s (%s)\n",i,plugin->pd->UniqueID,plugin->pd->Label,plugin->pd->Name);
	  }

	  plugin->pd = plugin->pdf(0);
	  printf("portcount = %i\n",plugin->pd->PortCount);

	  return 0;

/*
	  for(i=0;i<plugin->pd->PortCount;i++) {
	    printf("port %i: %s\n",i,plugin->pd->PortNames[i]);
	    printf(" descriptor %i: ",plugin->pd->PortDescriptors[i]);
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_INPUT) printf("input ");
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_OUTPUT) printf("output ");
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_CONTROL) printf("control ");
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_AUDIO) printf("audio ");
	    printf("\n");
	    printf("  range hint 0x%x: ",plugin->pd->PortRangeHints[i]);
	    if(LADSPA_IS_HINT_BOUNDED_BELOW(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("bounded_below ");
	    if(LADSPA_IS_HINT_BOUNDED_ABOVE(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("bounded_above ");
	    if(LADSPA_IS_HINT_TOGGLED(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("toggled ");
	    if(LADSPA_IS_HINT_SAMPLE_RATE(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("sample_rate ");
	    if(LADSPA_IS_HINT_LOGARITHMIC(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("log ");
	    if(LADSPA_IS_HINT_INTEGER(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("int ");
	    if(LADSPA_IS_HINT_DEFAULT_MINIMUM(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_minimum ");
	    if(LADSPA_IS_HINT_DEFAULT_LOW(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_low ");
	    if(LADSPA_IS_HINT_DEFAULT_MIDDLE(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_middle ");
	    if(LADSPA_IS_HINT_DEFAULT_HIGH(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_high ");
	    if(LADSPA_IS_HINT_DEFAULT_MAXIMUM(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_maximum ");
	    if(LADSPA_IS_HINT_DEFAULT_0(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_0 ");
	    if(LADSPA_IS_HINT_DEFAULT_1(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_1 ");
	    if(LADSPA_IS_HINT_DEFAULT_100(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_100 ");
	    if(LADSPA_IS_HINT_DEFAULT_440(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_440 ");
	    printf("\n");
	    printf("  LowerBound = %f, UpperBound = %f\n",plugin->pd->PortRangeHints[i].LowerBound,plugin->pd->PortRangeHints[i].UpperBound);
	  }
*/
exitdl:
	dlclose(plugin->dl);
exit:
	return rv;
}

int ladspa_init_plugin(ladspa_plugin *plugin, ladspa_wrapper *wrapper) {
  int i;
  int rv=-1;
  int num_input, num_output, num_param;

  memset(wrapper,0,sizeof(ladspa_wrapper));
  wrapper->plugin=plugin;

  num_input = 0;
  num_output = 0;
  num_param = 0;

  for(i=0;i<plugin->pd->PortCount;i++) {
	    printf("port %i: %s\n",i,plugin->pd->PortNames[i]);
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_CONTROL) {
	    	if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_INPUT)
	    		num_param++;
	    } else {
	    	if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_INPUT)
	    		num_input++;
	    	else
	    		num_output++;
	    }
  }

  // allocate memory for parameters and buffers

  wrapper->inputs=calloc(sizeof(LADSPA_Data), num_input * BUFFER_SIZE);
  wrapper->outputs=calloc(sizeof(LADSPA_Data), num_output * BUFFER_SIZE);
  wrapper->param=calloc(sizeof(LADSPA_Data), num_param);

  wrapper->ph = plugin->pd->instantiate(plugin->pd,44100);

/*  wrapper->delay = 2.5;
  wrapper->max_slow = 0.3;
  wrapper->lfo = 0.4;
  wrapper->feedback = -0.8; */

  num_input = 0;
  num_output = 0;
  num_param = 0;

  // connect ports
  for(i=0;i<plugin->pd->PortCount;i++) {
	    printf("port %i: %s\n",i,plugin->pd->PortNames[i]);
	    printf(" descriptor %i: ",plugin->pd->PortDescriptors[i]);
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_INPUT) printf("input ");
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_OUTPUT) printf("output ");
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_CONTROL) printf("control ");
	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_AUDIO) printf("audio ");
	    printf("\n");
	    printf("  range hint 0x%x: ",plugin->pd->PortRangeHints[i]);
	    if(LADSPA_IS_HINT_BOUNDED_BELOW(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("bounded_below ");
	    if(LADSPA_IS_HINT_BOUNDED_ABOVE(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("bounded_above ");
	    if(LADSPA_IS_HINT_TOGGLED(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("toggled ");
	    if(LADSPA_IS_HINT_SAMPLE_RATE(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("sample_rate ");
	    if(LADSPA_IS_HINT_LOGARITHMIC(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("log ");
	    if(LADSPA_IS_HINT_INTEGER(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("int ");
	    if(LADSPA_IS_HINT_DEFAULT_MINIMUM(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_minimum ");
	    if(LADSPA_IS_HINT_DEFAULT_LOW(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_low ");
	    if(LADSPA_IS_HINT_DEFAULT_MIDDLE(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_middle ");
	    if(LADSPA_IS_HINT_DEFAULT_HIGH(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_high ");
	    if(LADSPA_IS_HINT_DEFAULT_MAXIMUM(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_maximum ");
	    if(LADSPA_IS_HINT_DEFAULT_0(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_0 ");
	    if(LADSPA_IS_HINT_DEFAULT_1(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_1 ");
	    if(LADSPA_IS_HINT_DEFAULT_100(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_100 ");
	    if(LADSPA_IS_HINT_DEFAULT_440(plugin->pd->PortRangeHints[i].HintDescriptor)) printf("default_440 ");
	    printf("\n");
	    printf("  LowerBound = %f, UpperBound = %f\n",plugin->pd->PortRangeHints[i].LowerBound,plugin->pd->PortRangeHints[i].UpperBound);

	    if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_CONTROL) {
	    	if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_INPUT)
	    		plugin->pd->connect_port(wrapper->ph,i,&wrapper->param[num_param++]);
	    } else {
	    	if(plugin->pd->PortDescriptors[i] & LADSPA_PORT_INPUT)
	    		plugin->pd->connect_port(wrapper->ph,i,&wrapper->inputs[num_input++]);
	    	else
	    		plugin->pd->connect_port(wrapper->ph,i,&wrapper->outputs[num_output++]);
	    }
  }

  if(plugin->pd->activate) plugin->pd->activate(wrapper->ph);

  printf("ladspa plugin init done\n");
  return 0;
}

void ladspa_run_plugin(ladspa_wrapper *wrapper, size_t len, unsigned char *buffer) {
	size_t ofs=0;
	size_t blen;
	int i;

	//printf("processing %d bytes at 0x%x\n",len,buffer);
	while(len>0) {
		blen=(len>BUFFER_SIZE)?BUFFER_SIZE:len;
		//printf(" - %d\n",blen);
		for(i=0;i<blen;i++)
			wrapper->inputs[0][i]=(float)(buffer[i+ofs])/255.0;
		//printf("run\n");
		wrapper->plugin->pd->run(wrapper->ph,blen);
		//printf("overwrite\n");
		for(i=0;i<blen;i++)
			//buffer[i+ofs]=(unsigned char)(wrapper->input[i]*255);
			buffer[i+ofs]=(unsigned char)(wrapper->outputs[0][i]*255);
		//printf("inc\n");
		len-=blen;
		ofs+=blen;
	}
}
