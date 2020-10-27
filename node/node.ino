/** @file node.ino
 *  @brief
 *  
 *  This file represents baremetal implementation 
 *  of node for LEACH protocol
 *  
 *  @author Pavle Lakic
 *  @bug No known bugs
 */
#include "includes.h"

void setup() {

  unsigned char round_cnt;
  unsigned char ch_enable;

  Serial.begin(9600);
  delay(1000);
  Serial.println();

  if (mount_fs()) {
  
    read_fs(&round_cnt, &ch_enable);
  }

  cluster_head(&round_cnt, &ch_enable);
  full_circle(&round_cnt, &ch_enable);  

#if ROUND_RESET    
    write_fs(0, 1);
#endif  

#if DEBUG
  unsigned char rnd_cnt;
  unsigned char ch_en;
  read_fs(&rnd_cnt, &ch_en);
#endif
}

void loop() {

}
