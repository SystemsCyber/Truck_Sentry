#include "header.h"

#define LOGMODE
#define bus_len 21794 //137318
#define input_file "testandanalysis/retarder.csv" // tsc1_driver_reaction_retake, hardbrake,tsc1ts_driver_reaction2
#define output_file "testandanalysis/Outputs/retarder.txt"
//"testandanalysis/Output/general.txt" //"testandanalysis/Output/tsc1_driver_reaction_retake.txt"
#define data_start_in_CSV_row 5
const int chipSelect = BUILTIN_SDCARD;
#ifdef NATIVE
unsigned long current_millis;
#include <signal.h>
unsigned long milliseconds(){
    return current_millis;
}
FILE *outfile;

void pp(const char *str){
    if (!outfile){
    outfile = fopen(output_file, "w");
    if (!outfile){
        perror("OutFile file open failed");
        exit(1);
    }
    }

    fputs(str, outfile);

}

void close_outfile(){
    if (outfile){
        fclose(outfile);
    }
}

FILE* myFile;

void init_sd_card(){
    ;
}

void read_file_from_sd_card(const char *filename){
    if (!myFile){
    myFile = fopen("truck_ips.txt", "r");
    if (!myFile){
        perror("Index file read failed");
        exit(1);
    }
}
}

void close_sd_card(){
    fclose(myFile);
}

void read_line_from_sdcard(char *line, unsigned long len){
    if (!fgets(line, len, myFile)){
        fclose(myFile);
    }
}


#else

//FILE* myFile;
File myFile;
unsigned long milliseconds(){
  return millis();
}

void pp(const char *str){
    Serial.printf("%s",str);
}
void init_sd_card(){
  print("Initializing SD card...");
//  delay(100);
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
    }
  }
  print("initialization done.\n");

}

void read_file_from_sd_card(const char *filename){
  myFile = SD.open(filename);
  if(!myFile){
    print("error opening file on SD card");
  }
}

void close_sd_card(){
     myFile.close();
}

void read_line_from_sdcard(char *line, unsigned long line_len){
  myFile.readStringUntil('\n').toCharArray(line,line_len);
}

bool has_bytes_in_file(){
    return myFile.available();
}

#endif

char prnt[256];
void print(const char * format, ...)
{
 memset(prnt, 0, 100);
 va_list args;
 va_start (args, format);
 vsprintf (prnt, format, args);
 va_end (args);
 pp(prnt);
}

uint64_t data_lookup[64];
bool pos_lookup[64];
uint8_t bps;
uint64_t extract_data(uint8_t* barray, uint8_t *t_bytes, uint8_t *t_bits, uint8_t *t_masks, uint8_t first_length){  
    bps = t_bytes[0]*8 + t_bits[0];
    //DEBUG
    pos_lookup[bps] = false;
    if (pos_lookup[bps] == false){
        if (t_bytes[0] == t_bytes[1]){
//            Serial.println("Nolo");
            data_lookup[bps] = (barray[t_bytes[0]] & t_masks[0]) >> t_bits[0];
        }else{
//            Serial.print("IN\n");
            data_lookup[bps] = 0;
            data_lookup[bps] |= (barray[t_bytes[1]] & t_masks[1]) >> t_bits[1];
//            Serial.print("val: %llu\n", data_lookup[bps]);
            uint8_t bpos = t_bytes[1] - 1;
            while (bpos > t_bytes[0]){
//                Serial.print("bpos: %u\n", bpos);
                data_lookup[bps] = (data_lookup[bps] << 8) | barray[bpos];
                bpos--;
            }
//            Serial.print("val: %llu\n", data_lookup[bps]);
            data_lookup[bps] = data_lookup[bps] << first_length;
            data_lookup[bps] |= (barray[t_bytes[0]] & t_masks[0]) >> t_bits[0];
//            Serial.print("val: %llu\n", data_lookup[bps]);
        }
        pos_lookup[bps] = true;
    }

   return data_lookup[bps];
}

void reset_dbyte(){
  for (unsigned long i = 0; i < 64; i++){
    pos_lookup[i] = false;;
  }
}

bool m;
unsigned long millisecs;
uint64_t bval;


// #define debug_process_rlinks
void process_rlinks(RLink *rindexes, unsigned long num_rules, uint8_t *barray){
  for (unsigned long i = 0; i < num_rules; i++){

       if (rindexes[i].relation == 1){ //Exploit //
           m = true;
           for (unsigned long j = 0; j < rindexes[i].rule->num_moi; j++){ //
              bval = extract_data(barray,
                      rindexes[i].rule->moi[j].t_bytes,
                      rindexes[i].rule->moi[j].t_bits,
                      rindexes[i].rule->moi[j].t_masks,
                      rindexes[i].rule->moi[j].first_length);
                // print("v0: %lu, v1: %lu, bval: %lu\n", rindexes[i].rule->moi[j].value[0],rindexes[i].rule->moi[j].value[1], bval);
              m = m & ((rindexes[i].rule->moi[j].value[0] <= bval) & (bval <= rindexes[i].rule->moi[j].value[1]));
           }
           if (m == true){
             rindexes[i].rule->cth ++;
             if (rindexes[i].rule->interval != 0){
                 millisecs = milliseconds();
                 if ((millisecs - rindexes[i].rule->last_time) > rindexes[i].rule->interval){
//                      Serial.print("Hello\n");
                     rindexes[i].rule->cth = 0;
                 }
                 rindexes[i].rule->last_time = millisecs;
             }

             if (rindexes[i].rule->ncc < rindexes[i].rule->max_ncc){
//                Serial.print("Hello2\n");
                rindexes[i].rule->cth = 0;
             }
            // print ("l2rule: %lx,ncc: %lu, max_ncc: %lu cth: %lu threshgld: %lu\n", rindexes[i].rule, rindexes[i].rule->ncc, rindexes[i].rule->max_ncc, rindexes[i].rule->cth, rindexes[i].rule->threshold);

           if (rindexes[i].rule->cth >= rindexes[i].rule->threshold){
            print("DEAD!! -- l2: %lx\n", rindexes[i].rule);
                  ;
                  return;
           }
           }
       }
       else{ //Context carrier
//           m = true;
           for (unsigned long j = 0; j < rindexes[i].num_indexes; j++){
              bval = extract_data(barray,
                      rindexes[i].rule->context[rindexes[i].indexes[j]].t_bytes,
                      rindexes[i].rule->context[rindexes[i].indexes[j]].t_bits,
                      rindexes[i].rule->context[rindexes[i].indexes[j]].t_masks,
                      rindexes[i].rule->context[rindexes[i].indexes[j]].first_length
              );
            //  print("bval: %llu, v0 : %lu, v1: %lu\n", bval, rindexes[i].rule->context[rindexes[i].indexes[j]].value[0], rindexes[i].rule->context[rindexes[i].indexes[j]].value[1]);
              m = (rindexes[i].rule->context[rindexes[i].indexes[j]].value[0] <= bval) & (bval <= rindexes[i].rule->context[rindexes[i].indexes[j]].value[1]);
            //  print("m: %u prevm: %u\n", m, rindexes[i].rule->context[rindexes[i].indexes[j]].prevm);
              if (m != rindexes[i].rule->context[rindexes[i].indexes[j]].prevm){
                #ifdef debug_process_rlinks
                print("Mactch\n");
                #endif
                if (m == true){
                   rindexes[i].rule->ncc ++;
                }
                else{
                   rindexes[i].rule->ncc --;
                }
               rindexes[i].rule->context[rindexes[i].indexes[j]].prevm = m;
              }
           }
        //   print ("l2rule: %lx,ncc: %lu, max_ncc: %lu\n", rindexes[i].rule, rindexes[i].rule->ncc, rindexes[i].rule->max_ncc);
       }
    }
    
}

void process_rules(Target **targets, uint8_t num_trg, uint8_t *barray){
   for (uint8_t j = 0; j < num_trg; j++){
       process_rlinks(targets[j]->rlinks, targets[j]->num_rlinks, barray);
       
   }
}

#define LINELEN 100
char line[LINELEN];
char *pEnd;
L2Rule *l2rules = NULL;

RadixTreeNode* parseRadixTree(){
   read_line_from_sdcard(line,LINELEN);

   if (strncmp(line, "-1", 1) == 0){

       return NULL;
   }
   pEnd = line;

//    print("Node line : %s\n", line);

   RadixTreeNode *tnode = (RadixTreeNode *)malloc(sizeof(RadixTreeNode));
   tnode->length = strtoul(pEnd,&pEnd, 10);
   tnode->value = strtoul(pEnd,&pEnd, 10);
   if (strtoul(pEnd,&pEnd, 10) == 1){
      read_line_from_sdcard(line,LINELEN);
      pEnd = line;
    //   print("Target line : %s\n", line);
      tnode->target = (Target *)malloc(sizeof(Target));
      tnode->target->last_byte = strtoul(pEnd,&pEnd, 10);
      read_line_from_sdcard(line,LINELEN);
      pEnd = line;
    //   print("Target line : %s\n", line);
      tnode->target->num_np_l2rules = strtoul(pEnd,&pEnd, 10);
      if (tnode->target->num_np_l2rules > 0){
         tnode->target->np_l2rules = (L2Rule **)malloc(tnode->target->num_np_l2rules*sizeof(L2Rule *));
        read_line_from_sdcard(line,LINELEN);
        pEnd = line;
        // print("np rules line : %s\n", line);
        for (unsigned long k = 0; k < tnode->target->num_np_l2rules; k ++){
            tnode->target->np_l2rules[k] = &l2rules[strtoul(pEnd,&pEnd, 10)];
        }
      }
     
      read_line_from_sdcard(line,LINELEN);
      pEnd = line;
    //   print("Target line : %s\n", line);
      tnode->target->num_rlinks = strtoul(pEnd,&pEnd, 10);
      if (tnode->target->num_rlinks > 0){
        tnode->target->rlinks = (RLink *)malloc(tnode->target->num_rlinks*sizeof(RLink));
        for (unsigned long k = 0; k < tnode->target->num_rlinks; k ++){
            read_line_from_sdcard(line,LINELEN);
            pEnd = line;
            // print("rlink line : %s\n", line);
            tnode->target->rlinks[k].rule = &l2rules[strtoul(pEnd,&pEnd, 10)];
            tnode->target->rlinks[k].relation = strtoul(pEnd,&pEnd, 10);
            tnode->target->rlinks[k].num_indexes = strtoul(pEnd,&pEnd, 10);
            if (tnode->target->rlinks[k].num_indexes > 0){
              tnode->target->rlinks[k].indexes = (uint8_t *)malloc(tnode->target->rlinks[k].num_indexes*sizeof(uint8_t *));
              read_line_from_sdcard(line,LINELEN);
              pEnd = line;
              for (unsigned long l = 0; l < tnode->target->rlinks[k].num_indexes; l++){
                tnode->target->rlinks[k].indexes[l] = strtoul(pEnd,&pEnd, 10);
              }
            }
            
        }
      }
     
   }
   else{
       tnode->target = NULL;
   }
   tnode->lchild = parseRadixTree();
   tnode->rchild = parseRadixTree();
   return tnode;
}

RadixTreeNode *node;
RadixTreeNode *root;

void load(){
   init_sd_card();
   read_file_from_sd_card("truck_ips.txt");
   read_line_from_sdcard(line,LINELEN);
   pEnd = line;

   unsigned long num_l2rules = strtoul(pEnd,&pEnd,10);
   l2rules = (L2Rule *)malloc(num_l2rules*sizeof(L2Rule));

   for (unsigned long i = 0; i < num_l2rules; i++){
       read_line_from_sdcard(line,LINELEN);
       pEnd = line;
       unsigned long index = strtoul(pEnd,&pEnd,10);
       l2rules[index].ncc = 0; l2rules[index].cth = 0; l2rules[index].last_time = 0;
       l2rules[index].max_ncc = strtoul(pEnd,&pEnd,10);
       l2rules[index].threshold = strtoul(pEnd,&pEnd,10);
       l2rules[index].interval = strtoul(pEnd,&pEnd,10);

       read_line_from_sdcard(line,LINELEN);
       pEnd = line;
       l2rules[index].num_moi = strtoul(pEnd,&pEnd,10);
       l2rules[index].moi = (FieldFilter *)malloc(l2rules[index].num_moi*sizeof(FieldFilter));
       for (unsigned long j = 0; j < l2rules[index].num_moi; j++){
           read_line_from_sdcard(line,LINELEN);
           pEnd = line;
           l2rules[index].moi[j].t_bytes[0] = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].t_bytes[1] = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].t_bits[0] = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].t_bits[1] = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].t_masks[0] = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].t_masks[1] = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].first_length = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].value[0] = strtoul(pEnd,&pEnd,10);
           l2rules[index].moi[j].value[1] = strtoul(pEnd,&pEnd,10);
       }

       read_line_from_sdcard(line,LINELEN);
       pEnd = line;
       l2rules[index].num_context = strtoul(pEnd,&pEnd,10);
       l2rules[index].context = (SFieldFilter *)malloc(l2rules[index].num_context*sizeof(SFieldFilter));
       for (unsigned long k = 0; k < l2rules[index].num_context; k++){
          read_line_from_sdcard(line,LINELEN);
          pEnd = line;
          l2rules[index].context[k].prevm = 0;
          l2rules[index].context[k].t_bytes[0] = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].t_bytes[1] = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].t_bits[0] = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].t_bits[1] = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].t_masks[0] = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].t_masks[1] = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].first_length = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].value[0] = strtoul(pEnd,&pEnd,10);
          l2rules[index].context[k].value[1] = strtoul(pEnd,&pEnd,10);
       }
   }
   node = parseRadixTree();
   root = node;

   for (unsigned long i = 0; i < num_l2rules; i++){
    print ("L2Rule %lu = 0x%08lx\n", i, &l2rules[i]);
   }
}


/* ------------------------------------- TRACE RADIX TREE ------------------------------------------- */
uint8_t chunk_masks[2][5] = {{0,0,0,0,0},{1,3,7,15,31}};
long overlap;
unsigned long accum_val = 0;
uint8_t num_targets;
Target* targets[3];
uint8_t nodepos = 4;
uint8_t node_endpos;
unsigned long time_tt;
bool attack_detected;

bool check_for_attack(Target *target){
  for (uint8_t i = 0; i < target->num_np_l2rules; i++){
    target->np_l2rules[i]->cth ++;
    if (target->np_l2rules[i]->interval > 0){
      time_tt = milliseconds();
      if (time_tt - target->np_l2rules[i]->last_time > target->np_l2rules[i]->interval){
        target->np_l2rules[i]->cth = 0;
      }
      target->np_l2rules[i]->last_time = time_tt;
    }
    if (target->np_l2rules[i]->cth >= target->np_l2rules[i]->threshold){
//      print("DEAD!! -- l2: %x\n", target->np_l2rules[i]);
      return true;
    }
  }
  return false;
}

// #define debug_trace_radix_tree
void trace_radix_tree(struct Bitchunk* bt_recv, uint8_t bitpos, uint8_t bit_endpos){

  if (bitpos == 0){
    attack_detected = false;
    num_targets = 0;
    memset(targets,0,3*sizeof(Target*));
  }

   node_endpos = nodepos + node->length -1;

   overlap = MIN(bit_endpos, node_endpos) - MAX(bitpos,nodepos) + 1;

   if (overlap > 0){
       accum_val = accum_val << overlap | chunk_masks[bt_recv->bitlevel][overlap -1];
   }
   #ifdef debug_trace_radix_tree
   print ("node_pos: %u, node_endpos: %u, overlap: %ld, accum_val: %lu node-val: %lu, node-len: %u\n", nodepos, node_endpos, overlap, accum_val, node->value, node->length);
   #endif
   if (bit_endpos > node_endpos){
      nodepos = nodepos + node->length;
       #ifdef debug_trace_radix_tree
       print ("More bits than this node\n");
       #endif
      if (accum_val == node->value){
        #ifdef debug_trace_radix_tree
        print ("Match!!\n");
        #endif
          if (node->target != NULL){
              if (check_for_attack(node->target) == true){
                attack_detected = true;
                // accum_val = 0;
                // return;
              }
              targets[num_targets] = node->target;
              num_targets ++;
          }
          if (bt_recv->bitlevel == 0){
              node = node->lchild;
          }else{
              node = node->rchild;
          }
      }else{
          node = NULL;
      }
        accum_val = 0;
      if (node != NULL){
        #ifdef debug_trace_radix_tree
        print ("Moving to the next node\n");
        #endif
          trace_radix_tree(bt_recv,bitpos,bit_endpos);
      }
   } 
}

/*----------------------------------------- CONTROLLER ----------------------------- */
#define RCONTROLLER //SIMCONTROLLER

struct Bitchunk bt;
bool SOF;

#ifdef SIMCONTROLLER

uint32_t *frame;
unsigned int txcount;
uint8_t no_ifs_bits = 5;
long trg = -1;
bool bit = true;
unsigned long mask;
void set_mask(uint8_t p_val){
   mask = (0x1 << p_val);
}
bool stuff_it = false;
void get_bit_from_frame(){
   txcount++;
   if (txcount == 1){
       bit = false;
       stuff_it = true;
   }
   else if (txcount >= 13 && txcount <= 14){
       bit =  true;
   }
   else if (txcount >= 33 && txcount <= 35){
       bit = false; //Just a place-holder
   }
   else if (txcount == 36){
       bit = true; //Just a place-holder
   }
   else if (txcount == 37){
       bit = false; //Just a place-holder
   }
   else if (txcount == 38){
       bit = false; //Just a place-holder
   }
   else if (txcount == 39){
       bit = false;
       set_mask(31);
       trg++;
   }
   else if (txcount >= 104 && txcount <= 118){
       bit = !bit; //just a placeholder
   }
   else if (txcount == 119){
       bit = true;
       stuff_it = false;
   }
   else if (txcount == 120){
       bit = false;
   }
   else if (txcount >= 121 && txcount <= (131 + no_ifs_bits)){
       bit = true;
   }
   else if (txcount == 131 + no_ifs_bits + 1){
       bit = true;
       set_mask(28);
       txcount = 0;
       trg++;
       // printy("\n ---------------------- \n");
   }
   else{
       if ((frame[trg] & mask) > 0){bit  = true;}
       else{bit = false;} //convert this to a one liner if possible
       frame[trg] = frame[trg] << 1;
       if (txcount == 71){
           set_mask(31);
           trg++;
       }
   }
}

bool bl = true;
uint8_t btn = 11;
bool sof = false;
unsigned long bus_index = -1;
#ifndef LOGMODE
uint32_t bus[bus_len][3] = {
   {0x00000021, 0xDE0010D7, 0xCAFEBABE},
   {0x0C00000F, 0xFDC05D7D, 0xCAFEBABE}, //7D is 125 which is 0
    {0x00000021, 0xDE0010D7, 0xCAFEBABE},
   {0x18F0010B, 0x00FFFFFF, 0xFFFDBABE},
   {0x0C00000B, 0xFDC05DFF, 0xCAFEBABE},
   {0x18F0010B, 0xFFFFFFFF, 0xFFFDBABE},
   {0x0C00000B, 0xFDC05DFF, 0xCAFEBABE},
};
#else
uint32_t bus[bus_len][3];
double timestamps[bus_len];
#endif


struct Bitchunk* get_bitchunk(){
   if (trg == -1){
       set_mask(28);
       txcount = 0;
   }

   // > Get a frame
   if (trg == -1 || trg >= 3){
       trg = 0;
       bus_index ++;
       frame = bus[bus_index];
       current_millis = timestamps[bus_index]*1000;
       print("Frame: 0x%08lx 0x%08lx 0x%08lx %lf %lu\n", frame[0], frame[1], frame[2], timestamps[bus_index], current_millis);
   }

   // > call get_bit_from_frame(frame) and check for stuffs
    do{
       get_bit_from_frame();
       if (bl != bit || (btn >= 5 && stuff_it)){
           bt.bitlevel = bl;
           bt.bitnum = btn;
           SOF = sof;
           bl = bit;
           btn = 1;
           if (bt.bitnum > 5){ sof = true; }
           else{ sof = false; break; }
       }
       else{
           btn++;
       }
   }while(trg < 3);

   // > end
   if (bus_index >= bus_len){ 
     return NULL;
     }
   else{return &bt;}
}

FILE *logfile;
char *token;
uint8_t tok_num;
uint32_t num;
bool header = true;
unsigned long bus_load_index;
unsigned long data1;
unsigned long data2;
#define LLINELEN 100
char lline[LLINELEN];
void controller_setup(){
   bt.bitlevel = true;
   bt.bitnum = 10;
   #ifdef LOGMODE
    if (!logfile){
    logfile = fopen(input_file, "r");
    if (!logfile){
        print("Log file read failed");
        exit(1);
    }
    }
    bus_load_index = 0;
    while (fgets(lline, LLINELEN, logfile)){  
        if (header){
            header = false;
            continue;
        }      
        // print("%s", lline);
        token = strtok(lline, ",");
        timestamps[bus_load_index] = strtod(token, NULL);
        // printf("ts: %f\n", timestamps[bus_load_index]);
        tok_num = 1;
        data1 = 0;
        data2 = 0;
        while(token != NULL)
        {
            // printf( " %s\n", token );

            token = strtok(NULL, ",");
            tok_num++;

            // if (tok_num == 2){        
            //     timestamps[bus_load_index] = strtof(token, NULL);
            //     printf("ts: %f\n", timestamps[bus_load_index]);
            // }

            if (tok_num == 3){
                num = strtoul(token, NULL, 16);
                bus[bus_load_index][0] = num;
            }

            if (tok_num >= data_start_in_CSV_row && tok_num <= data_start_in_CSV_row+3){
                data1 = data1 << 8 | strtoul(token, NULL, 16);
            }

            if (tok_num == data_start_in_CSV_row+3){
                bus[bus_load_index][1] = data1;
            }

            if (tok_num >= data_start_in_CSV_row+4 && tok_num <= data_start_in_CSV_row+7){
                data2 = data2 << 8 | strtoul(token, NULL, 16);
            }
            // print ("tok_num: %lu\n", tok_num);
            if (tok_num == data_start_in_CSV_row+7){
                bus[bus_load_index][2] = data2;
                // break;
            }

        }
        // printf("Bus: %08lx %08lx %08lx\n\n", bus[bus_load_index][0], bus[bus_load_index][1], bus[bus_load_index][2]);
        bus_load_index++;
        // printf ("bus_load_index: %lu\n", bus_load_index);
        // if (bus_load_index == bus_len -2){break;}
    }
    fclose(logfile);
   #endif
}
#else
//Changeable Information
const unsigned short baudrate = 250;//baudrate in thousands
const bool high_noise_tolerance = true;//Set to false to make bit capture timing more strict
const unsigned short RX = 0;//GPIO pin for receiving CAN messages
const unsigned short TX = 1;//GPIO pin to implement BitBanging technique on CAN line

//Global Variable Information
//unsigned short bitpos = 0;//Bit Position
unsigned short temp_numBit = 0;//Used for samples taken without edge
unsigned short numBit = 0;//Used for calculations while keeping true numBits saved
volatile unsigned long Tdeltas[130] = {};//Timestamps gathered, Maximum amount of changes with 10 added for buffer
bool bitlevel = 1;//Bit value of samples -- initially the value is not known so 1
volatile unsigned short pending = 0;//Bits waiting to be processed
volatile unsigned short processed = 0;//Bits processed
//volatile unsigned long frame_num = 0;
unsigned short tolerance = 0;//Changes bit capture tolerance
const unsigned short bitwidth = (1000 / baudrate);//Calculated Bitwidth in us depending on CAN baudrate
const unsigned short eofwidth = (11 * bitwidth) - (bitwidth / 2);//End of Frame will reset variables used for calculations
const unsigned short eoowidth = (6 * bitwidth) - (bitwidth / 2);
unsigned short noise = 0;
bool error_overload = false;

void ISR_CAN(){
  static elapsedMicros dT;
  if (dT >= noise){
    if(dT >= eofwidth && bitlevel == 1){
      SOF = true;
      bitlevel = 0;
      pending = 0;
      processed = 0;
      error_overload = false;
    }
    else{
      Tdeltas[pending] = dT;
      pending++;  
    }
  }
  dT = 0;
}

void controller_setup(){
  //Creates tolerance based on noise/resolution error 
  if (high_noise_tolerance == false){tolerance = 1;}
  noise = (bitwidth / 2 + tolerance);
  
  //Pins for ISR and BitBanging
  pinMode(RX,INPUT);// sets the RX as input to read CAN traffic
  pinMode(TX,OUTPUT);//sets the TX as output to employ bitbang as needed
  attachInterrupt(digitalPinToInterrupt(RX),ISR_CAN,CHANGE);//Interrupt to detect edges of bit changes
}

struct Bitchunk* get_bitchunk(){
  if (pending <= processed){return NULL;}
  if (pending <= processed){return NULL;}
  if (error_overload){return NULL;}
  
  unsigned short temp_processed = processed;

  bitlevel =! bitlevel;
  bool reduce = false;

  if(temp_numBit == 5){reduce = true;}                        //Subtracts stuff bits and SOF, these are not included in calculated bitpos
  
//  PRINT("Tdeltas[temp_processed]: %lu\n", Tdeltas[temp_processed]);
  
  temp_numBit=Tdeltas[temp_processed] / bitwidth;                                    //Rounding method to ensure correct bits are calculated if delta of edges are not perfectly divisible by bitwidth
  if((Tdeltas[temp_processed] % bitwidth) >= (bitwidth / 2)){temp_numBit++;}
                           
  if(reduce == true){numBit = temp_numBit -1;}                                       //numBit is applied, keeping prev_numBit true, in case 2 stuffbits need to be calculated in succession
  
  else{numBit = temp_numBit;}

  if (numBit > 5){
    error_overload = true;
    return NULL;
  }

  temp_processed++;
  if (temp_processed == (processed + 1)){
    processed = temp_processed;
  }
  
  if (numBit == 0){return NULL;}

  bt.bitlevel = !bitlevel;
  bt.bitnum = numBit;
  return &bt;
}
#endif
    
