#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>	//open
#include<unistd.h>	//exit
#include<sys/ioctl.h>	//ioctl

#define TestTimeMax 10
const *TestWriteString[10] = {"Cat", "Dog", "Body", "MEME4916", "MEME5816", 
    "gfdogiregrgergregre", "sd3", "fdvcd",  "453gbdscf", "fdsfefefefg"};

static char *dev="/dev/hello",message[100];

int main()
{
    int file_desc;
    int TestTimeCnt = 0;
    int BytesTemp = -1;

    for(int TestCnt = 0; TestCnt < TestTimeMax; TestCnt++)
    {
        printf("\033[34m%d Time\033[0m Homework2 Test Process:\n", TestCnt + 1);
        // open the device file, ex: /dev/hello
        if((file_desc = open(dev, O_RDWR)) < 0)
        {
            printf("Open file Test \033[31mFail:\033[0m\n");
            printf("Can't open device file: %s\n", dev);
            exit(-1);
        }
        else
        {
            printf("Open file Test \033[32mPass:\033[0m\n");
            printf("Open device file: \033[35m%s\033[0m\n", dev);
        }
        // read from the device file
        TestTimeCnt = 0;
        BytesTemp = -1;
        while (BytesTemp != 0)
        {
            TestTimeCnt++;
            memset(message, 0, 100);
            BytesTemp = read(file_desc, message, sizeof(message));
            if(BytesTemp < 0)
            {
                printf("Read file Test \033[31mFail:\033[0m\n");
                printf("read failed\n");
                exit(-1);
            }
            else if(BytesTemp != 0)
            {
                printf("%d Times, get %d bytes, get_msg message:\033[31m%s\033[0m", TestTimeCnt, BytesTemp, message);
            }
            else if(BytesTemp == 0)
            {
                printf("%d Times, Read file Test \033[32mPass:\033[0m\n", TestTimeCnt);
            }
        }

        if(write(file_desc, TestWriteString[TestCnt], strlen(TestWriteString[TestCnt])) < 0)
        {
            printf("Write file Test \033[31mFail:\033[0m\n");
            printf("write fail:\n");
            exit(-1);
        }
        else
        {
            printf("Write file Test \033[32mPass:\033[0m\n");
            printf("send_message success.\n");
        }

        TestTimeCnt = 0;
        BytesTemp = -1;
        while (BytesTemp != 0)
        {
            TestTimeCnt++;
            memset(message, 0, 100);
            BytesTemp = read(file_desc, message, sizeof(message));
            if(BytesTemp < 0)
            {
                printf("Read the writing result \033[31mFail:\033[0m\n");
                printf("read failed\n");
                exit(-1);
            }
            else if(BytesTemp != 0)
            {
                printf("%d Times, get %d bytes, get_msg message:\033[31m%s\033[0m", TestTimeCnt, BytesTemp, message);
            }
            else if(BytesTemp == 0)
            {
                printf("%d Times, Read file Test \033[32mPass:\033[0m\n", TestTimeCnt);
            }
        }

        printf("close file \033[32mPass.\033[0m\n");
        close(file_desc);
        printf("\n");
    }

    printf("\033[32mThe test all PASS\033[0m\n");
    // close file
    return 0;
}
