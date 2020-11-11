
#include "boardparms.h"
#include "boardparms_voice.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_VOICE_BOARD_ID_NAMES    20

int main (int argc, char **argv)
{

   char boardids[512];
   char boardid[32][BP_BOARD_ID_LEN];
   char voiceBoardids[BP_BOARD_ID_LEN * MAX_VOICE_BOARD_ID_NAMES];   
   char *opname;
   char *sname;
   int ret = 0;
   int i, n, op;
   int nerrs;
   int id, nboards, nVoiceBoards, idVoice;
   int start;
   unsigned short Function[BP_PINMUX_MAX];
   unsigned int Muxinfo[BP_PINMUX_MAX];
   nboards = BpGetBoardIds (boardids, 512);
   
   for (i = 0; i < nboards; i++)
   {
      printf ("found %s\n", &boardids[BP_BOARD_ID_LEN * i]);
      if (strlen(&boardids[BP_BOARD_ID_LEN * i]) > BP_BOARD_ID_LEN) 
      {
         printf ("TOO LONG!\n");
         exit(1);
      }
      strcpy (boardid[i], &boardids[BP_BOARD_ID_LEN * i]);
   }
   
   for (id = 0; id < nboards; id++)
   {
      printf ("\ncheck %s\n", boardid[id]);
      BpSetBoardId (boardid[id]);
      
      /* Get number of voice boards */
      nVoiceBoards = BpGetVoiceBoardIds(voiceBoardids, MAX_VOICE_BOARD_ID_NAMES, boardid[id]);   
      
      /* If no voice boards supported, set index to -1 to iterate through loop once */
      idVoice = (( nVoiceBoards )? 0 : -1) ;
      
      for( ; idVoice<nVoiceBoards; idVoice++ ) 
      {
         if( nVoiceBoards )
         {
            BpSetVoiceBoardId(&voiceBoardids[idVoice * BP_BOARD_ID_LEN]);
            printf ("\n+VoiceBoard %s\n", &voiceBoardids[idVoice * BP_BOARD_ID_LEN]);
         }
         
         start = 1;
         ret = BpGetAllPinmux (BP_PINMUX_MAX, &n, &nerrs, Function, Muxinfo);
         for (i = 0 ; i < n ; i++) {
               if (Function[i] & BP_GPIO_SERIAL) {
                  sname = "serial";
               } else {
                  sname = "gpio  ";
               }
               op = Muxinfo[i] & BP_PINMUX_OP_MASK;
               switch (op)
               {
               case BP_PINMUX_SWLED :
                  opname = "SWLED";
                  break;
               case BP_PINMUX_SWGPIO :
                  opname = "SWGPIO";
                  break;
               case BP_PINMUX_HWLED :
                  opname = "HWLED";
                  break;
               case BP_PINMUX_DIRECT_HWLED :
                  opname = "DIRECT_HWLED";
                  break;
               default :
                  opname = "?";
               }
               printf ("%s GPIO %d Mux %ld Pin %ld OP %d %s\n",
                       sname,
                       Function[i] & BP_GPIO_NUM_MASK,
                       (Muxinfo[i] & BP_PINMUX_VAL_MASK) >> BP_PINMUX_VAL_SHIFT,
                       Muxinfo[i] & BP_PINMUX_PIN_MASK, op >> BP_PINMUX_OP_SHIFT,
                       opname);
         }
         if (nerrs) {
            fprintf(stderr,"%s: Errors were found\n", argv[0]);
            exit(1);
         }
      }
   }
   return(ret);
}

