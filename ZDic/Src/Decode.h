/*
 * Decode.h
 *
 * header file for Decode.h
 *
 */
 
#ifndef DECODE_H_
#define DECODE_H_

int LZSS_Decoder(const unsigned char *src, long src_length,
	unsigned char *dst, long *dst_length);
	
#endif /* DECODE_H_ */
