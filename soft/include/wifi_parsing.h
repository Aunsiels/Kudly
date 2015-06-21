/**
 * \file wifi_parsing.h
 * \brief Xml parsing for wifi feature
 * \author KudlyProject
 * \version 0.1
 * \date 07/04/2015
 *
 * Finite state machine that parses xml to execute function written
 * in it
 *
 */

#ifndef _WIFI_PARSING_H_
#define _WIFI_PARSING_H_
/**
 *
 * \brief Event source to signal a photo taking
 *
 */
extern EventSource eventPhotoSrc;

/**
 *
 * \brief Event source to signal streaming
 *
 */
extern EventSource eventStreamSrc;

/**
 *
 * \brief Event source to signal a sound
 *
 */
extern EventSource eventSoundSrc;

/**
 *
 * \brief Event source to signal a story
 *
 */
extern EventSource eventStorySrc;

/**
 *
 * \brief Event source to signal a story
 *
 */
extern EventSource eventGameSrc;

/**
 *
 * \brief XML parsing to execute actions
 *
 * This function parses a xml page from serve and launches right
 * functions
 */
void parseXML(char c);

#endif
