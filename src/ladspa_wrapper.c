#include <ladspa.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "ladspa_wrapper.h"

#define BUFFER_SIZE 2048

int ladspa_init_plugin(const char *filename, ladspa_wrapper *wrapper) {
  int i;
  int rv=-1;

  wrapper->dl = dlopen(filename, RTLD_NOW);

  if(wrapper->dl==NULL) goto exitdl;

  printf("dlp: 0x%x\n",wrapper->dl);

  wrapper->pdf = (LADSPA_Descriptor_Function)dlsym(wrapper->dl, "ladspa_descriptor");

  if(wrapper->pdf==NULL) goto exitdl;

  printf("pdf: 0x%x\n",wrapper->pdf);

  for(i=0;;i++) {
    wrapper->pd = wrapper->pdf(i);
    if(wrapper->pd==NULL) break;
    printf("%i: 0x%x - %s (%s)\n",i,wrapper->pd->UniqueID,wrapper->pd->Label,wrapper->pd->Name);
  }

  wrapper->pd = wrapper->pdf(0);
  printf("portcount = %i\n",wrapper->pd->PortCount);

  for(i=0;i<wrapper->pd->PortCount;i++) {
    printf("port %i: %s\n",i,wrapper->pd->PortNames[i]);
    printf(" descriptor %i: ",wrapper->pd->PortDescriptors[i]);
    if(wrapper->pd->PortDescriptors[i] & LADSPA_PORT_INPUT) printf("input ");
    if(wrapper->pd->PortDescriptors[i] & LADSPA_PORT_OUTPUT) printf("output ");
    if(wrapper->pd->PortDescriptors[i] & LADSPA_PORT_CONTROL) printf("control ");
    if(wrapper->pd->PortDescriptors[i] & LADSPA_PORT_AUDIO) printf("audio ");
    printf("\n");
    printf("  range hint 0x%x: ",wrapper->pd->PortRangeHints[i]);
    if(LADSPA_IS_HINT_BOUNDED_BELOW(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("bounded_below ");
    if(LADSPA_IS_HINT_BOUNDED_ABOVE(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("bounded_above ");
    if(LADSPA_IS_HINT_TOGGLED(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("toggled ");
    if(LADSPA_IS_HINT_SAMPLE_RATE(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("sample_rate ");
    if(LADSPA_IS_HINT_LOGARITHMIC(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("log ");
    if(LADSPA_IS_HINT_INTEGER(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("int ");
    if(LADSPA_IS_HINT_DEFAULT_MINIMUM(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_minimum ");
    if(LADSPA_IS_HINT_DEFAULT_LOW(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_low ");
    if(LADSPA_IS_HINT_DEFAULT_MIDDLE(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_middle ");
    if(LADSPA_IS_HINT_DEFAULT_HIGH(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_high ");
    if(LADSPA_IS_HINT_DEFAULT_MAXIMUM(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_maximum ");
    if(LADSPA_IS_HINT_DEFAULT_0(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_0 ");
    if(LADSPA_IS_HINT_DEFAULT_1(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_1 ");
    if(LADSPA_IS_HINT_DEFAULT_100(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_100 ");
    if(LADSPA_IS_HINT_DEFAULT_440(wrapper->pd->PortRangeHints[i].HintDescriptor)) printf("default_440 ");
    printf("\n");
    printf("  LowerBound = %f, UpperBound = %f\n",wrapper->pd->PortRangeHints[i].LowerBound,wrapper->pd->PortRangeHints[i].UpperBound);
  }

  wrapper->ph = wrapper->pd->instantiate(wrapper->pd,44100);

  wrapper->delay = 2.5;
  wrapper->max_slow = 0.3;
  wrapper->lfo = 0.4;
  wrapper->feedback = -0.8;

  wrapper->pd->connect_port(wrapper->ph,0,&wrapper->delay);
  wrapper->pd->connect_port(wrapper->ph,1,&wrapper->max_slow);
  wrapper->pd->connect_port(wrapper->ph,2,&wrapper->lfo);
  wrapper->pd->connect_port(wrapper->ph,3,&wrapper->feedback);

  wrapper->pd->connect_port(wrapper->ph,4,wrapper->input);
  wrapper->pd->connect_port(wrapper->ph,5,wrapper->output);

  if(wrapper->pd->activate) wrapper->pd->activate(wrapper->ph);

  /*
  fp=fopen("knuckles.bgr","r");
  if(!fp) goto exitph;

  fpo=fopen("out.bgr","w");
  if(!fpo) goto exitfp;

  if(pd->activate) pd->activate(ph);

  while(!feof(fp)) {
    i = fread(rawinput,1,BUFFER_SIZE,fp);
    printf("read %i bytes\n",i);
    if(i>0) {
      for(j=0;j<i;j++) {
        input[j]=(float)rawinput[j] / 255.0;
        //printf("%f ",input[j]);
      }
      pd->run(ph,i);
      //printf("\n");
      for(j=0;j<i;j++) {
        rawoutput[j]=(unsigned char)(output[j]*255);
        //printf("%f ",output[j]);
      }
      //printf("\n");
      fwrite(rawoutput,1,i,fpo);
    }
  }

  if(pd->deactivate) pd->deactivate(ph);

  fclose(fpo);
  */
  printf("init done\n");
  return 0;
exitph:
wrapper->pd->cleanup(wrapper->ph);
exitdl:
  if(wrapper->dl!=NULL) dlclose(wrapper->dl);

  return rv;
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
			wrapper->input[i]=(float)(buffer[i+ofs]) / 255.0;
		//printf("run\n");
		wrapper->pd->run(wrapper->ph,blen);
		//printf("overwrite\n");
		for(i=0;i<blen;i++)
			//buffer[i+ofs]=(unsigned char)(wrapper->input[i]*255);
			buffer[i+ofs]=(unsigned char)(wrapper->output[i]*255);
		//printf("inc\n");
		len-=blen;
		ofs+=blen;
	}
}
