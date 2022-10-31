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

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

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

    unsigned char I[MAX_SIZE + 6];
    unsigned char bytes_sent;

    fd = llopen(layer);
    
    if (fd == -1){
        printf("Connection failed\n");
        return;
    }
    printf("Connection opened\n");

    if (layer.role == LlTx){

        FILE* file;
        file = fopen(filename, "rb");
        if (file == NULL){
            printf("Can't open file\n");
            return;
        }
        int bytes_read = 1;
        int bytes_written = 0;
        while (bytes_read > 0){
            
            size_t I_size = sizeof(I) / sizeof(I[0]);

            size_t count = MAX_PAYLOAD_SIZE / I_size;
            bytes_read = fread(I+4, 1, count, file);

            if (bytes_read == -1){              
                break;
            }

            else if (bytes_read > 0){
                bytes_written = llwrite(I+4, MAX_PAYLOAD_SIZE - bytes_sent);
                if(bytes_written == -1 || bytes_written != bytes_read + 3){
                    break;
                }
                bytes_sent++;
                total_frames_sent += bytes_sent;

                printf("Read from file (%d) -> Write to link layer (%d), Total bytes sent: %d\n",
                        bytes_read, bytes_written, total_frames_sent);
            }


            else if(bytes_read == 0){
                llwrite(buffer, 1);
                printf("App layer: done reading and sending file\n");
                break;
            }
        }
        sleep(1);
        fclose(file);
    }
    else if (layer.role == LlRx){
        FILE* file;
        file = fopen(filename, "wb");
        
        if (file == NULL){
            printf("Can't open file\n");
            return;
        }

        int bytes_read = 1;
        int bytes_written = 0;
        while (bytes_read > 0){
            bytes_read = llread(buffer);

            if (bytes_read == -1){
                break;
            }
            else if (bytes_read > 0){
                size_t I_size = sizeof(I) / sizeof(I[0]);

                size_t count = MAX_PAYLOAD_SIZE / I_size;
                bytes_read = fread(buffer+4, 1, count, file);
                bytes_written = fwrite(buffer+4, sizeof(I), count, file);
                if(bytes_written == -1 || bytes_written != bytes_read + 3){
                    break;
                }
                total_received_frames += bytes_read;

                printf("Read from file (%d) -> Write to link layer (%d), Total bytes sent: %d\n",
                        bytes_read, bytes_written, total_frames_sent);
            }

            
            else if(bytes_read == 0){
                llwrite(buffer, 1);
                printf("App layer: done reading and sending file\n");
                break;
            }
        }
        fclose(file);
    }
    
    llclose(TRUE);

}
