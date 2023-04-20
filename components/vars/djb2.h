#ifndef _DJB2_HEADER_
#define _DJB2_HEADER_

/**
 * Calculate djb2 hash.
 * 
 * Note that this works within 32bit integers, so it overflows inside
 * the 2^32 range 
 */
unsigned long djb2_hash(const unsigned char *str) {
    unsigned long hash = 5381;
    int c;

    while((c = *str++))
        hash = ((hash << 5) + hash) + c;
		
    return hash;
}

#endif