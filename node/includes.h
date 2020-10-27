/** @file includes.h
 *  @brief Functions which node of LEACH protocol will use.
 *
 *  This file contains other necessary includes,
 *	structures, and defines needed for complete
 *	node functionality.
 *
 *  @author Pavle Lakic
 *  @bug No known bugs.
 */
 
#ifndef INCLUDES_H_
#define INCLUDES_H_

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266TrueRandom.h>
#include <stdlib.h>
#include <Ticker.h>
#include <FS.h>

/** Debug flag, set to 1 to see debug messages.*/
#define DEBUG			                1

/** With this flag enabled, round = 0, ch_enable = 1 will
  * be written to filesystem. Useful to set manually
  * when all nodes must be in initial state.*/
#define ROUND_RESET               0

/**Maximum number of nodes in network*/
#define NUMBER_OF_NODES           2

/** String length for round to be writen to filesystem.*/
#define ROUND_NUMBER_LENGTH       2

/** String length for if node was cluster head to be written to filesystem.*/
#define CH_NUMBER_LENGTH          2

/** This will be password for all nodes.*/
#define NODE_PASS                 "teorijazavere"

/** This will be password for base station.*/
#define BASE_PASS                 "teorijazavere"

/** Base station SSID.*/
#define BASE_SSID                 "5G server"

/** Port where broadcast data will be sent*/
#define BROADCAST_PORT            2000

#define NODE_0                    0
#define NODE_1                    1

/** This defines node SSID*/
#define MY_NODE                   "1"

const char* create_node_id (char *txt);

/**
 * @brief Connects to base, or to closest node.
 * @param CH Indicates if this node is cluster head for current round.
 * @return none.
 */
void wifi_connect(unsigned char CH);

/**
 * @brief Resets values of round and ch_enable if
 * all nodes were cluster heads, so full new circle can begin.
 * @param round_cnt Current round number.
 * @param ch_enable Flag if node was CH in previous rounds.
 * @return none.
 */
void full_circle(unsigned char *round_cnt, unsigned char *ch_enable);

/**
 * @brief Decides if node will be cluster head or not.
 * @param round_cnt Current round number.
 * @param ch_enable Flag if node was CH in previous rounds.
 * Also writes 0 for ch_enable if node was declared as CH
 * in this round.
 * @return 1 if node is CH for this round, 0 if not.
 */
unsigned char cluster_head(unsigned char *round_cnt, unsigned char *ch_enable);

/**
 * @brief Mounts filesystem.
 * Mounts filesystem, so values of round and cluster 
 * head enable can be written to filename file.
 * @param none.
 * @return true if mounted successfully.
 */
bool mount_fs(void);

/**
 * @brief Generates threshold.
 *  This function is called only if current node was
 *  not CH for the current round.
 * @param P Probability that node will become CH.
 * @param r Current round.
 * @return Returns threshold for current round.
 */
float calculate_threshold(float P, unsigned char r);

/**
 * @brief Generates random number between 0 and 1.
 * @param none.
 * @return random number between 0 and 1.
 */
float random_number(void);

/**
 *	@brief Reads round and if node was cluster head from
 *	filesystem.
 *	@param round_cnt Address of round is passed as pointer.
 *	@param ch Address of CH indicator is passed as pointer.
 * 	@return none.
 */
void read_fs(unsigned char *round_cnt, unsigned char *ch);

/**
 *	@brief Writes round, and indicator that node was cluster head to filesystem.
 *	@param round_cnt Value of round to be written.
 *	@param ch Value of CH indicator.
 * 	@return none.
 */
void write_fs(int round_cnt, int ch);

#endif // INCLUDES_H_
