#include "header.h"

uint8_t prog_state = 0;
struct Bitchunk* bt_recv;
uint8_t bitpos;
int8_t bit_endpos;
uint8_t data_byte_array[8];
uint8_t byte_start_pos = 39;
uint8_t byte_pos = 0;
bool rep = false;
bool killed = false;
unsigned long ml;
uint8_t chunk_masks_l[2][5] = {{0,0,0,0,0},{1,3,7,15,31}};
uint8_t max_last_byte = 0;
long overlap_l;


//#define debug_loop
void loop(){
    if ((bt_recv = get_bitchunk()) != NULL){
        
        if (SOF){
            SOF = false;
            bit_endpos = -1;
            if (prog_state == 0){
                node = root;
                nodepos = 4;
                prog_state = 1;
            }
        }
        bitpos = bit_endpos + 1;
        bit_endpos = bitpos + bt_recv->bitnum -1;

        #ifdef debug_loop
        print("SOF: %d bitpos: %u bitendpos: %u ", SOF, bitpos, bit_endpos);
        print(" bitchunk: ");
        for (uint8_t tmp = 0; tmp < bt_recv->bitnum; tmp ++){
        print("%d", bt_recv->bitlevel);
        }
        print("\n");
        #endif


        if (prog_state == 1){ // TRACE
            trace_radix_tree(bt_recv, bitpos, bit_endpos);
            if (attack_detected == true){
                prog_state = 0;
            }
            else{
                if (node == NULL){
                    #ifdef debug_loop
                    print ("End of tracing!!\n");
                    #endif
                    max_last_byte = 0;
                    if (num_targets == 0){
                        prog_state = 0;
                    }
                    else{
                        for (uint8_t i = 0; i < num_targets; i ++){
                        max_last_byte = MAX(max_last_byte,targets[i]->last_byte);
                        if (targets[i]->num_rlinks > 0){
                            prog_state = 2;
                        }
                        else{
                            if (prog_state != 2){
                                prog_state = 0;
                            }
                        }
                    }
                    }
                    
                }
            }
        }

    if (prog_state == 2){ // PROCESS
        rep = true;
        while (rep){
            overlap_l = MIN(bit_endpos, byte_start_pos + 7) - MAX(bitpos,byte_start_pos) + 1;
            if (overlap_l > 0){
                data_byte_array[byte_pos] = data_byte_array[byte_pos] << overlap_l | chunk_masks_l[bt_recv->bitlevel][overlap_l -1];
            }
            if (bit_endpos > byte_start_pos + 7){
                byte_start_pos = byte_start_pos + 8;
                byte_pos ++;
            }
            else{
                rep = false;
            }
        }
        #ifdef debug_loop
        print ("Buffered bytes until %u: ", byte_pos);
        for (int i = 0; i < byte_pos + 1; i++){
            print("%02x ", data_byte_array[i]);
        }
        print ("\n");
        #endif
        if (byte_pos > max_last_byte){
            process_rules(targets, num_targets, data_byte_array);
            prog_state = 0;
            byte_pos = 0;
            byte_start_pos = 39;
            data_byte_array[byte_pos] = 0;
        }

    }

    #ifdef debug_loop
    print ("PROG STATE: %u\n", prog_state);
    #endif
}
}

void setup(){
    Serial.begin(9600);
    while (!Serial){
      ;
    }
    controller_setup();
    load();
}

