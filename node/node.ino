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
  unsigned char CH;

  Serial.begin(9600);
  delay(1000);
  Serial.println();

  if (mount_fs()) {
#if ROUND_RESET    
    write_fs(0, 1);
#endif    
    read_fs(&round_cnt, &ch_enable);
  }

  if (cluster_head(round_cnt, ch_enable)) {
    CH = 1;
  }
  else {
    CH = 0;
  }

}

void loop() {

}
