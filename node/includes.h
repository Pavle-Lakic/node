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
#define DEBUG			                0

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
