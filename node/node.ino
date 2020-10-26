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

  Serial.begin(9600);
  delay(1000);
  Serial.println();
  float t = calculate_threshold(0.33, 0);
  Serial.println(t, 2);

}

void loop() {

}
