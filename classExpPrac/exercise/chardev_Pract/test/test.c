#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>	//open
#include<unistd.h>	//exit
#include<sys/ioctl.h>	//ioctl

static char *dev="/dev/hello",message[100];

int main()
{
  int file_desc;

  // open the device file, ex: /dev/hello
  if((file_desc = open(dev, O_RDWR)) < 0)
  {
    printf("Can't open device file: %s\n", dev);
    exit(-1);
  }
  // read from the device file
  if(read(file_desc, message, sizeof(message)) < 0)
  {
    printf("read failed\n");
    exit(-1);
  }
  printf("get_msg message:%s\n",message);

  // write to the device file
  if(write(file_desc, "test", 5) < 0)
  {
    perror("write fail:\n");
  }
  close(file_desc);
  // close file

  return 0;
}
