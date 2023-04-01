/**
 * Example:
 * 
 * typedef enum{
 * 		ip_event_sta_got_ip = 0,
 * 		ip_event_sta_lost_ip,
 * 		ip_event_ap_staipassigned,
 * 		ip_event_got_ip6,
 * 		ip_event_eth_got_ip,
 * 		ip_event_eth_lost_ip,
 * 		ip_event_ppp_got_ip,
 * 		ip_event_ppp_lost_ip
 * }ip_event_t;
 * 
 * // custom ip event enum_string
 * const enum_str_t ip_event_str[] = {
 * 		ENUMSTR_ENTRY(ip_event_sta_got_ip),
 * 		ENUMSTR_ENTRY(ip_event_sta_lost_ip),
 * 		ENUMSTR_ENTRY(ip_event_ap_staipassigned),
 * 		ENUMSTR_ENTRY(ip_event_got_ip6),
 * 		ENUMSTR_ENTRY(ip_event_eth_got_ip),
 * 		ENUMSTR_ENTRY(ip_event_eth_lost_ip),
 * 		ENUMSTR_ENTRY(ip_event_ppp_got_ip),
 * 		ENUMSTR_ENTRY(ip_event_ppp_lost_ip)
 * };
 * 
 * const char *msg = enumstr_get(ip_event_str, ip_event_ppp_lost_ip);
 * 
 * printf("%s", msg); >> "ip_event_ppp_lost_ip"
 * 
 */

#ifndef _ENUMSTR_HEADER_
#define _ENUMSTR_HEADER_

#include <string.h>

/**
 * @brief struct that holds the enum to its string form
 */
typedef struct{
	int value;
	char *string;
}enum_str_t;

/**
 * @brief Add new entry to array
 * @param x: an enum item
 */
#define ENUMSTR_ENTRY(x) {.value = (int)x, .string = #x}

/**
 * @brief do not use, use enumstr_get() instead
 */
char *_enumstr_get(const enum_str_t array[], size_t array_size, int index);

/**
 * @brief get string from enum
 */
#define enumstr_get(const_enum_str_t_array, int_index) \
	_enumstr_get(const_enum_str_t_array, sizeof(const_enum_str_t_array)/sizeof(enum_str_t), (int)int_index)

#endif