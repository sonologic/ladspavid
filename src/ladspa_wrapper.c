#include <ladspa.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 2048

int ladspa_init_plugin(const char *filename) {
  void *dlp;
  LADSPA_Handle ph; 
  const LADSPA_Descriptor *pd;
  LADSPA_Descriptor_Function pdf;
  int i,j;
  LADSPA_Data delay = 2.5;
  LADSPA_Data max_slow = 2.0;
  LADSPA_Data lfo = 0.5;
  LADSPA_Data feedback = -0.1;
  unsigned char rawinput[BUFFER_SIZE];
  unsigned char rawoutput[BUFFER_SIZE];
  LADSPA_Data input[BUFFER_SIZE];
  LADSPA_Data output[BUFFER_SIZE];
  FILE *fp;
  FILE *fpo;
  

  //dlp = dlopen("/usr/lib/ladspa/flanger_1191.so", RTLD_NOW);
  dlp = dlopen(filename, RTLD_NOW);

  if(dlp==NULL) goto exitdl;

  printf("dlp: 0x%x\n",dlp);

  pdf = (LADSPA_Descriptor_Function)dlsym(dlp, "ladspa_descriptor");

  if(pdf==NULL) goto exitdl;

  printf("pdf: 0x%x\n",pdf);

  for(i=0;;i++) {
    pd = pdf(i);
    if(pd==NULL) break;
    printf("%i: 0x%x - %s (%s)\n",i,pd->UniqueID,pd->Label,pd->Name);
  }

  pd = pdf(0);
  printf("portcount = %i\n",pd->PortCount);

  for(i=0;i<pd->PortCount;i++) {
    printf("port %i: %s\n",i,pd->PortNames[i]);
    printf(" descriptor %i: ",pd->PortDescriptors[i]);
    if(pd->PortDescriptors[i] & LADSPA_PORT_INPUT) printf("input ");
    if(pd->PortDescriptors[i] & LADSPA_PORT_OUTPUT) printf("output ");
    if(pd->PortDescriptors[i] & LADSPA_PORT_CONTROL) printf("control ");
    if(pd->PortDescriptors[i] & LADSPA_PORT_AUDIO) printf("audio ");
    printf("\n");
    printf("  range hint 0x%x: ",pd->PortRangeHints[i]);
    if(LADSPA_IS_HINT_BOUNDED_BELOW(pd->PortRangeHints[i].HintDescriptor)) printf("bounded_below ");
    if(LADSPA_IS_HINT_BOUNDED_ABOVE(pd->PortRangeHints[i].HintDescriptor)) printf("bounded_above ");
    if(LADSPA_IS_HINT_TOGGLED(pd->PortRangeHints[i].HintDescriptor)) printf("toggled ");
    if(LADSPA_IS_HINT_SAMPLE_RATE(pd->PortRangeHints[i].HintDescriptor)) printf("sample_rate ");
    if(LADSPA_IS_HINT_LOGARITHMIC(pd->PortRangeHints[i].HintDescriptor)) printf("log ");
    if(LADSPA_IS_HINT_INTEGER(pd->PortRangeHints[i].HintDescriptor)) printf("int ");
    if(LADSPA_IS_HINT_DEFAULT_MINIMUM(pd->PortRangeHints[i].HintDescriptor)) printf("default_minimum ");
    if(LADSPA_IS_HINT_DEFAULT_LOW(pd->PortRangeHints[i].HintDescriptor)) printf("default_low ");
    if(LADSPA_IS_HINT_DEFAULT_MIDDLE(pd->PortRangeHints[i].HintDescriptor)) printf("default_middle ");
    if(LADSPA_IS_HINT_DEFAULT_HIGH(pd->PortRangeHints[i].HintDescriptor)) printf("default_high ");
    if(LADSPA_IS_HINT_DEFAULT_MAXIMUM(pd->PortRangeHints[i].HintDescriptor)) printf("default_maximum ");
    if(LADSPA_IS_HINT_DEFAULT_0(pd->PortRangeHints[i].HintDescriptor)) printf("default_0 ");
    if(LADSPA_IS_HINT_DEFAULT_1(pd->PortRangeHints[i].HintDescriptor)) printf("default_1 ");
    if(LADSPA_IS_HINT_DEFAULT_100(pd->PortRangeHints[i].HintDescriptor)) printf("default_100 ");
    if(LADSPA_IS_HINT_DEFAULT_440(pd->PortRangeHints[i].HintDescriptor)) printf("default_440 ");
    printf("\n");
    printf("  LowerBound = %f, UpperBound = %f\n",pd->PortRangeHints[i].LowerBound,pd->PortRangeHints[i].UpperBound);
  }

  ph = pd->instantiate(pd,1000);

  pd->connect_port(ph,0,&delay);
  pd->connect_port(ph,1,&max_slow);
  pd->connect_port(ph,2,&lfo);
  pd->connect_port(ph,3,&feedback);

  pd->connect_port(ph,4,input);
  pd->connect_port(ph,5,output);

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
  
exitfp:
  fclose(fp);
exitph:
  pd->cleanup(ph);
exitdl:
  if(ph!=NULL) dlclose(ph);

  exit(0);
}
