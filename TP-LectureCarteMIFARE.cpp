/*

  Copyright (c) 2021 ODALID SARL - https://odalid.com

  Mifare_Classic.c
  ------------

*/

#include <time.h>
#include <stdio.h>

#include "MfErrNo.h"
#include "Core.h"
#include "Sw_Device.h"
#include "Sw_Mf_Classic.h"
#include "Sw_Poll.h"
#include "Sw_ISO14443A-3.h"
#include "TypeDefs.h"
#include "Tools.h"

int card_read(uint8_t sect_count);

BOOL bench = FALSE;

ReaderName MonLecteur;

int main()
{
    int16_t status = MI_OK;
    uint8_t i;
    char s_buffer[64];
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint8_t sect_count = 0;
    //uint8_t data[240] = {0};


    //memset(data, 0x00, 240);

    MonLecteur.Type = ReaderCDC;
    MonLecteur.device = 0;

    printf("ODALID SDK\n");
    printf("\n");
    printf("NXP MIFARE 'CLASSIC' (1k and 4k) reference demo\n");
    printf("-----------------------------------------------\n");
    printf("https://odalid.com\n\n");

    /*status = GetLibrary(s_buffer, sizeof(s_buffer));
    if (status != MI_OK){
        printf("Failed to get API version\n");
        goto done;
    }
    printf("API version : %s\n", s_buffer);
    printf("\n");*/


  // Open reader
    status = OpenCOM(&MonLecteur);
    if (status != MI_OK){
        printf("Reader not found\n");
        goto done;
    }
    else{
        switch(MonLecteur.Type)
        {
            case ReaderTCP:
                sprintf(s_buffer, "IP : %s", MonLecteur.IPReader);
            break;
            case ReaderCDC:
                sprintf(s_buffer, "COM%d", MonLecteur.device);
            break;

        }
        printf("Reader found on %s\n", s_buffer);
    }


    status = Version(&MonLecteur);
    if (status == MI_OK){
        printf("Reader firwmare is %s\n", MonLecteur.version);
        printf("Reader serial is %02X%02X%02X%02X\n", MonLecteur.serial[0], MonLecteur.serial[1], MonLecteur.serial[2], MonLecteur.serial[3]);
        printf("Reader stack is %s\n", MonLecteur.stack);
    }

    status = LEDBuzzer(&MonLecteur, LED_YELLOW_ON);
    if (status != MI_OK){
        printf("LED [FAILED]\n");
        goto close;
    }

    // RF field ON
    RF_Power_Control(&MonLecteur, TRUE, 0);

init:
    printf("Attente carte !\n");
    while(ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len));
    if (status != MI_OK){
        printf("No available tag in RF field\n");
        goto close;
    }

    printf("Tag found: UID=");
    for (i = 0; i < uid_len; i++)
        printf("%02X", uid[i]);
    printf(" ATQ=%02X%02X SAK=%02X\n", atq[1], atq[0], sak[0]);


    /*if ((atq[1] != 0x00) || ((atq[0] != 0x02) && (atq[0] != 0x04) && (atq[0] != 0x18))){
        printf("This is not a Mifare classic tag\n");
        goto tag_halt;
    }*/

    if ((sak[0] & 0x1F) == 0x08){
        // Mifare classic 1k : 16 sectors, 3+1 blocks in each sector
        printf("Tag appears to be a Mifare classic 1k\n");
        sect_count = 16;
    } else if ((sak[0] & 0x1F) == 0x18){
        // Mifare classic 4k : 40 sectors, 3+1 blocks in 32-first sectors, 15+1 blocks in the 8-last sectors
        printf("Tag appears to be a Mifare classic 4k\n");
        sect_count = 40;
    }

    status = card_read(sect_count);

    goto tag_halt;

tag_halt:

    // Halt the tag
    status = ISO14443_3_A_Halt(&MonLecteur);
    if (status != MI_OK){
        printf("Failed to halt the tag\n");
        goto close;
    }


    status = LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(1);
    status = LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    goto init;
close:
  // Close the reader

    RF_Power_Control(&MonLecteur, FALSE, 0);


    CloseCOM(&MonLecteur);

done:
  // Display last error
  if (status == MI_OK)
  {
    printf("Done\n");
  } else
  {
    printf("%s (%d)\n", GetErrorMessage(status), status);
  }
  return 0;
}


int card_read(uint8_t sect_count)
{
    uint8_t data[240] = {0};
    clock_t t0, t1;
    uint8_t bloc_count, bloc, sect;
    uint8_t offset;
    int16_t status = 0;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;

    if (bench){
        printf("Reading %d sectors...\n", sect_count);
        t0 = clock();
    }
    bloc = 0;
    for (sect = 0; sect < sect_count; sect++){
        if (!bench)
        printf("Reading sector %02d : ", sect);


        memset(data, 0x00, 240);
        status = Mf_Classic_Read_Sector(&MonLecteur, TRUE, sect, data, AuthKeyA, 0);

        if (status != MI_OK){
            if (bench)
                printf("Read sector %02d ", sect);
            printf("[Failed]\n");
            printf("  %s (%d)\n", GetErrorMessage(status), status);
            status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
            if (status != MI_OK){
                printf("No available tag in RF field\n");
                //goto close;
            }
        }
        else{
            if (!bench){
                printf("[OK]\n");
                // Display sector's data
                if (sect < 32)
                    bloc_count = 3;
                else
                    bloc_count = 15;
                for (bloc = 0; bloc < bloc_count; bloc++){
                    printf("%02d : ", bloc);
                    // Each blocks is 16-bytes wide
                    for (offset = 0; offset < 16; offset++){
                        printf("%02X ", data[16 * bloc + offset]);
                    }
                    for (offset = 0; offset < 16; offset++){
                        if (data[16 * bloc + offset] >= ' '){
                            printf("%c", data[16 * bloc + offset]);
                        } else
                            printf(".");

                    }
                    printf("\n");
                }
            }
        }
    }

    if (bench){
        t1 = clock();
        printf("Time elapsed: %ldms\n", (t1 - t0) / (CLOCKS_PER_SEC/1000));
    }
    return MI_OK;
}


