// #include "header.h"

// uint8_t barrays[2][8];
// uint8_t *barray;
// FieldFilter cpf;
// uint64_t val;
// unsigned long nr1;
// unsigned long nr2;

// #define max_params 33
// #define max_rindexes 500
// RLink rindexes[max_rindexes];
// int8_t t_byte_diffs[33] = {-1, 3, 3, 3, 6, 6, 1, 2, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, -1, -1, -1, 0, 0, 0};
// FieldFilter fake_test_ff;
// FieldFilter *moi;
// SFieldFilter *context;
// void setup() {
//  Serial.begin(9600);
//   while (!Serial) {
//     ; // wait for serial port to connect. Needed for native USB
//   }
//     for (unsigned long i = 0; i < 8; i ++){
//       barrays[0][i] = 0xff;
//     }
//     for (unsigned long i = 0; i < 8; i ++){
//       barrays[1][i] = 0;
//     }

//     moi = malloc(max_params*sizeof(FieldFilter));
//     context = malloc(max_params*sizeof(SFieldFilter));  
//     for (unsigned long i = 0; i < max_params; i++){ 
//       moi[i].value[0] = 0;
//       moi[i].value[1] = 1; 
//       context[i].prevm = false;
//       context[i].value[0] = 0;
//       context[i].value[1] = 1; 
//     }
      
//     for (unsigned long i = 0; i < max_rindexes; i++){
// //      rindexes[i].msg_type = 1;
// //      r.msg_indexes = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31]
// //      L2Rule l = malloc(sizeof(L2Rule));
//       rindexes[i].rule.ncc = 0;
//       rindexes[i].rule.max_ncc = 0;
//       rindexes[i].rule.last_time = 0;
//       rindexes[i].rule.interval = 1;
//       rindexes[i].rule.cth = 0;
//       rindexes[i].rule.threshold = 1;
//       rindexes[i].rule.moi = moi;
//       rindexes[i].rule.context = context;
//     }

//     fake_test_ff.t_bits[0] = 7;
//     fake_test_ff.t_bits[1] = 7;
//     fake_test_ff.t_masks[0] = 0xff;
//     fake_test_ff.t_masks[1] = 0xff;
//     fake_test_ff.first_length = 8;
// //    Serial.println("Start");
//     for (unsigned long i = 0; i < max_params; i++){
//       if (t_byte_diffs[i] == -1){
//         continue;
//       }
// //      Serial.println(i);
//       fake_test_ff.t_bytes[0] = 0;
//       fake_test_ff.t_bytes[1] = t_byte_diffs[i];
//       nr1 = 0;
//       nr2 = 0;
//       for (unsigned long j = 0; j < max_rindexes; j++){
//         delay(2);
// //        reset_dbyte();
//         elapsedMicros ts;
//         process_rlinks(rindexes, j, barrays[1], i, &fake_test_ff, 1);
//         long int us = ts;
//         if (us > 96){
//           nr1 = j -1;
//           break;
//         }
//       }

//       for (unsigned long j = 1; j < max_rindexes; j++){
//         delay(2);
//         barray = barrays[1];
//         elapsedMicros ts;
// //        Serial.printf("num params: %lu num_rules: %lu\n", i, j);
//         process_rlinks(rindexes, j, barray, i, &fake_test_ff, 2);
//         long int us = ts;
//         if (us > 96){
//           nr2 = j -1;
//           break;
//         }
//       }

//       Serial.printf("num params: %lu num_rules1: %lu, num_rules2: %lu, min num_rules: %lu\n", i, nr1, nr2, MIN(nr1, nr2));
//     }
    
// }
  
  
// //  // put your setup code here, to run once:
// //  cpf.t_bytes[0] = 0;
// //  cpf.t_bytes[1] = 7;
// //  cpf.t_bits[0] = 0;
// //  cpf.t_bits[1] = 7;
// //  cpf.t_masks[0] = 0xff;
// //  cpf.t_masks[1] = 0xff;
// //  cpf.first_length = 8;
// //  memset(barray,0xff,8);
// //  elapsedMicros usTimer;
// //  for (unsigned long i = 0; i < 100; i ++){
// //    val = extract_data(barray, &cpf);
// //  }
// //  long int us = usTimer;
// //  Serial.printf("val: %llu in :%lu\n", val, us);
// //}

// void loop() {
//   // put your main code here, to run repeatedly:

// }
