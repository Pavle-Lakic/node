/** @file functions.cpp
 *  @brief
 *  
 *  This file contains relevant defines, structures and
 *  functions that node.ino will call.
 *  
 *  @author Pavle Lakic
 *  @bug No known bugs
 */

#include "includes.h"

/** String length for round to be writen to filesystem.*/
#define ROUND_NUMBER_LENGTH       2

/** String length for if node was cluster head to be written to filesystem.*/
#define CH_NUMBER_LENGTH          2

/** Name of file where round and if node was cluster head
 *  will be kept.
 */
const char *filename = "/conf.txt";

/** Probability that node will be cluster head for current round.
 *  Determined apriori, depends of number of nodes.*/
const float P = 0.5;

bool cluster_head(unsigned char round_cnt, unsigned char ch_enable)
{
  bool ret = false;
  float rnd_nmb;
  float threshold;

  if (ch_enable == 0) {
    return ret;
  }

  rnd_nmb = random_number();
  threshold = calculate_threshold(P, round_cnt);

  if (rnd_nmb < threshold) {
    ret = true;
  }
#if DEBUG
  if (ret == true) {
    Serial.println("Node will be cluster head for current round.");
  }
  else {
    Serial.println("Node will not be cluster head for current round.");
  }
#endif  
  return ret;
}

bool mount_fs(void)
{ 
  bool success = SPIFFS.begin();
  if (!success){
    Serial.println("Could not mount SPIFFS!");
  }
  return success; 
}

void write_fs(int round_cnt, int ch)
{
  File fp;
  char round_cnt_str[ROUND_NUMBER_LENGTH];
  char ch_str[CH_NUMBER_LENGTH];

  sprintf(round_cnt_str, "%d", round_cnt);
  sprintf(ch_str, "%d", ch);

  fp = SPIFFS.open(filename, "w");

  if (!fp) {
    Serial.println("file open failed");
  }
  else {
      fp.write(round_cnt_str, ROUND_NUMBER_LENGTH);
      fp.write(ch_str, CH_NUMBER_LENGTH);
      fp.close();
  }
}

void read_fs(unsigned char *round_cnt, unsigned char *ch)
{
  File fp;
  char round_cnt_str[ROUND_NUMBER_LENGTH];
  char ch_str[CH_NUMBER_LENGTH];

  fp = SPIFFS.open(filename, "r");

  fp.read((uint8_t*)round_cnt_str, ROUND_NUMBER_LENGTH);
  fp.read((uint8_t*)ch_str, CH_NUMBER_LENGTH);
  fp.close();

  *round_cnt = atol(round_cnt_str);
  *ch = atol(ch_str);

#if DEBUG
  Serial.print("round_cnt = ");
  Serial.println(*round_cnt);
  Serial.print("ch_enable = ");
  Serial.println(*ch);
#endif
}

float calculate_threshold(float P, unsigned char r)
{
  float T;

  T = P/(1 - P*(r % 1/P));

  return T;
}

float random_number(void)
{
  float a;

  a = ESP8266TrueRandom.random(10000);
  a = a/10000;

  return a;
}
