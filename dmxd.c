#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <lo/lo.h>
#include <libftdi1/ftdi.h>

#define DMX_MAB   16
#define DMX_BREAK 110
#define DMX_SIZE  128

struct ftdi_context ftdic;
unsigned char dmxData[DMX_SIZE];

const char* lo_port="7777";
lo_address lo_a;
lo_server  lo_s;
lo_server_thread lo_st;


int chkerr(int errcode);
void lo_error(int num, const char *msg, const char *path);
int randbyte();
int dmx_set(const char *path, const char *types, lo_arg **argv, int argc,
	    void *data, void *user_data);

int main(void)
{
  if(!chkerr(ftdi_init(&ftdic))) { return EXIT_FAILURE; }
  if(!chkerr(ftdi_usb_open(&ftdic, 0x0403, 0x6001))) { return EXIT_FAILURE; }

  chkerr(ftdi_usb_reset(&ftdic));
  chkerr(ftdi_setrts(&ftdic, 0));
  chkerr(ftdi_set_baudrate(&ftdic, 250000));
  chkerr(ftdi_setflowctrl(&ftdic, SIO_DISABLE_FLOW_CTRL));
    
  dmxData[0]=0;
  dmxData[1]=100;
  dmxData[2]=100;
  dmxData[3]=50;
  dmxData[4]=100;

  printf("Initializing...\n");

  //  lo_a  = lo_address_new("localhost", lo_port);
  lo_st = lo_server_thread_new(lo_port, lo_error);
  lo_server_thread_add_method(lo_st, "/dmx/set", "ii", dmx_set, NULL);
  lo_server_thread_start(lo_st);

  sleep(3);
  printf("Running...\n");

  int frame=0;

  int frameTime = (int) floor((1000.0 / 30.0) + (double)0.5);

  while(1) {
    frame++;

    /*
    if(frame%200==0) {
      printf("Randomize...\n");
      dmxData[1]=randbyte();
      dmxData[2]=randbyte();
      dmxData[3]=randbyte();
      dmxData[4]=randbyte();
    }
    */

    chkerr(ftdi_set_line_property2(&ftdic, BITS_8, STOP_BIT_2, NONE, BREAK_ON));
    usleep(DMX_BREAK);

    chkerr(ftdi_set_line_property2(&ftdic, BITS_8, STOP_BIT_2, NONE, BREAK_OFF));
    usleep(DMX_MAB);
    
    chkerr(ftdi_write_data(&ftdic, dmxData, DMX_SIZE));
    usleep(1000 * frameTime);

    //    printf("%d\n",frame);
  }
  
  ftdi_usb_close(&ftdic);
  ftdi_deinit(&ftdic);
  
  return EXIT_SUCCESS;
}

int chkerr(int errcode) {
  if(errcode<0) {
    fprintf(stderr, "Error:%d:%s\n", errcode, ftdi_get_error_string(&ftdic));
    return 0;
  }
  return 1;
}

int randbyte() {
  return (int)(((float)rand()/RAND_MAX)*256);
}

void lo_error(int num, const char *msg, const char *path)
{
  fprintf(stderr,"liblo server error %d in path %s: %s\n", num, path, msg);
  fflush(stderr);
}

int dmx_set(const char *path, const char *types, lo_arg **argv, int argc,
                 void *data, void *user_data)
{
  //  printf("<%s> %d\n",path,argc);

  if(argc!=2) return 0;

  int ctrl =  argv[0]->i;
  int value = argv[1]->i;

  if(ctrl<1) return 0;
  if(ctrl>=DMX_SIZE) return 0;

  if(value<0) value=0;
  if(value>255) value=255;

  printf("set %d to %d\n", ctrl, value);
  dmxData[ctrl]=value;

  return 0;
}
