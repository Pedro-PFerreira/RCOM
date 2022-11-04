// Application layer protocol implementation

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "../include/application_layer.h"
#include "../include/link_layer.h"
#include "../include/macros.h"

LinkLayer layer;


unsigned char* createControl(unsigned char flag, int fSize, int num_octet){

    unsigned char* p = malloc(4*MAX_SIZE*sizeof(unsigned char));

    p[0] = flag;

    p[1] = 0x00;

    num_octet= 0;

    int temp = fSize;

    while (temp != 0){

        num_octet++;

        temp = temp >> 8;
    }

    p[2] = num_octet;

    for (int i = 1; i <= num_octet; i++){
        p[2+i] = (fSize >> (8*num_octet-i)) & 0xFF;
    }

    return p;

}

int readControl(unsigned char flag, unsigned char* control){

    unsigned char c_flag = control[0];

    printf("%d,%d\n", ST_CTRL, control[0]);
    
    if (flag != c_flag){
        return -1;
    }
    printf("Here\n");    
    if (flag == A_RCV) return -1;

    if (control[1] != 0){
        return -1;
    }

    int v = control[2];

    int fSize = 0;

    printf("%d\n", v);

    for(int i = 0; i <= v; i++){
        
        fSize = fSize << 8;
        fSize += control[3+i];
    }    

    printf("Size of the file is: %d\n", fSize);

    return fSize;
    
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename){

    strcpy(layer.serialPort, serialPort);
    if (strcmp(role,"tx") == 0){
        layer.role = LlTx;
    }
    else if(strcmp(role,"rx") == 0){
        layer.role = LlRx;
    }

    layer.baudRate = baudRate;
    layer.nRetransmissions = nTries;
    layer.timeout = timeout;

    unsigned char buffer[MAX_PAYLOAD_SIZE+ 1];

    buffer[0] = FLAG;
    buffer[1] = ESC;
    buffer[2] = ESC;
    
    unsigned char bytes_sent;

    int fd = llopen(layer);
    
    if (fd == -1){
        printf("Connection failed\n");
        return;
    }
    printf("Connection opened\n");
    unsigned char* control;
    if (layer.role == LlTx){

        FILE* file;
        file = fopen(filename, "rb");
        if (file == NULL){
            printf("Can't open file\n");
            return;
        }

        fseek(file, 0, SEEK_END);
        
        int fSize = ftell(file);

        fseek(file, 0, SEEK_SET);
        int bytes_read = 1;
        int bytes_written = 0;

        int num_octet = 0;
        control = createControl(ST_CTRL, fSize, num_octet);

        for (int i = 0; i < 3 + num_octet; i++){
            printf("%x\n", control[i]);
        }

        unsigned char I[3 + num_octet];

        bytes_written = llwrite(control, 3 + num_octet, I);

        printf("num of bytes written: %d\n", bytes_written);

        while (bytes_read < fSize){

            unsigned char* frame = malloc(4*MAX_SIZE*sizeof(unsigned char));
            
            bytes_read = fread(frame+4, 1, num_octet - 1, file);

            if (bytes_read == -1){              
                break;
            }

            else if (bytes_read > 0){
                bytes_written = llwrite(frame, bytes_read + 4, I);
                if(bytes_written == -1 || bytes_written != bytes_read + 3){
                    break;
                }
                bytes_sent++;
                total_frames_sent += bytes_sent;

                printf("Read from file (%d) -> Write to link layer (%d), Total bytes sent: %d\n",
                        bytes_read, bytes_written, total_frames_sent);
            }

            else if(bytes_read == 0){
                llwrite(buffer, 1, I);
                printf("App layer: done reading and sending file\n");
                break;
            }
        }
        
        sleep(1);
        fclose(file);
    }
    
    else if (layer.role == LlRx){

        int bytes_read = 1;
        int bytes_written = 0;

        unsigned char* control_packet = malloc(4*MAX_SIZE*sizeof(unsigned char));

        bytes_read = llread(control_packet);

        for (int i = 0; i < sizeof(control_packet); i++){
            printf("%d\n", control_packet[i]);
        }

        int fSize = readControl(ST_CTRL, control_packet);

        if (fSize == -1){
            return;
        }
        
        FILE* file;

        printf("Here\n");

        file = fopen(filename, "wb");

        printf("Here\n");

        if (file == NULL){
            printf("Can't open file\n");
            return;
        }

        unsigned char I[3 + fSize];

        while (bytes_read > 0){
            bytes_read = llread(buffer);

            if (bytes_read == -1){
                break;
            }
            else if (bytes_read > 0){
                size_t I_size = sizeof(I) / sizeof(I[0]);

                size_t count = MAX_PAYLOAD_SIZE / I_size;
                bytes_read = fread(buffer+4, 1, count, file);
                total_received_frames++;
                bytes_written = fwrite(buffer+4, sizeof(buffer), count, file);
                
                if(bytes_written == -1 || bytes_written != bytes_read + 3){
                    break;
                }
                total_received_frames += bytes_read;

                printf("Read from file (%d) -> Write to link layer (%d), Total bytes sent: %d\n",
                        bytes_read, bytes_written, total_frames_sent);
            }

            
            else if(bytes_read == 0){
                llwrite(buffer, 1, I);
                printf("App layer: done reading and sending file\n");
                break;
            }
        }
        fclose(file);
    }
    
    llclose(TRUE);

}
