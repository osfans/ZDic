/*
 * ZDic.c
 *
 * main file for ZDic
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#include <PalmOS.h>
#include <PalmOSGlue.h>

#include "Decode.h"

#define CACHE_SIZE		4096

// 定义共用常数
#define LOOK_SIZE		(497)					// 搜寻视窗大小
#define WINDOW_SIZE		(15)					// 预视视窗大小
#define BUFFER_SIZE		(LOOK_SIZE + WINDOW_SIZE)

// 资料缓冲区大小
#define BUFFER_MASK		(BUFFER_SIZE - 1)			// 用于资料缓冲区的环形索引值计算
#define THRESHOLD		2							// 编码临界值

#define RUN_ENCODED		0							// 额外编码位元，: 只有<起始位置>和<字串长度>字段
#define RAW_ENCODED		1							//				 : 只有<跟随字元>字段

#define R(i)			(((i) + output.start) & BUFFER_MASK)	// 资料缓冲区的环形索引值计算

#define ALPHA_BIT		8
#define	ALPHA_SIZE		256

typedef struct {
	int					bit_count;	// remanent bits in current read byte
	int					now;		// current read byte index
	long				total;		// size of buffer.
	const unsigned char	*buffer;	// bytes buffer.
} InputType;

typedef InputType *InputPtr;

typedef struct {
	long				start;		// head index.
} OutputType;

typedef OutputType *OutputPtr;


// get bits number for load var.
static int use_bits(long var)
{
	int bits = 0;
	long test = 1;

	while(test < var)
	{
		test <<= 1;
		bits++;
	}

	return bits;
}


// 初始化输入缓冲区
static int init_input(InputType *input, const unsigned char *buf, long len)
{
	input->bit_count = 8;
	input->now = 0;
	input->total = len;
	input->buffer = buf;
	
	return 0;
}

// 初始化输出缓冲区
static int init_output(OutputType *output)
{

	output->start = 0;
	
	return 0;
}


// 读取压缩后资料之副程式
static __inline long read_bits(int bits, InputType *input)
{
	long word = 0;
	unsigned char bit_buffer/*, temp*/;
	int need_bits;

	while(bits > 0)
	{
		if(input->bit_count == 0)				// 位元缓冲区是否为空
		{
			if(input->now >= input->total - 1)	// 字节缓冲区是否为空
			{
				return(-1);						// 全部读取完毕
			}
			input->bit_count = 8;
			input->now++;
		}
		bit_buffer = input->buffer[input->now];
		need_bits = bits > input->bit_count ? input->bit_count : bits;
		
/*		temp = (bit_buffer >> (input->bit_count - need_bits));
		temp &= (1 << need_bits) - 1;
		word <<= need_bits;
		word |= (unsigned long)temp;
		bits = bits - input->bit_count;
		input->bit_count -= need_bits;
*/
		word = (word <<= need_bits) | ((bit_buffer >> (input->bit_count - need_bits)) & ((1 << need_bits) - 1));
		bits -= input->bit_count;
		input->bit_count -= need_bits;
		
	}

	return (word);
}


// LZSS 解码器的主要解码副程式
int LZSS_Decoder(const unsigned char *src, long src_length,
	unsigned char *dst, long *dst_length)
{
	long start, length;
	long start_bit, length_bit;
	long i;
	long write_index, cur_index;
	InputType input;
	OutputType output;

	// 初始化输入和输出缓冲区
	if(init_input(&input, src, src_length) == -1
		|| init_output(&output) == -1)
	{
		return -1;
	}

	// 计算每个编码中 <起始位置>和<字串长度> 各需多少位表示
	start_bit = use_bits(LOOK_SIZE);
	length_bit = use_bits(WINDOW_SIZE + 1);
	write_index = cur_index = 0;

	while(true)
	{
		switch(read_bits(1, &input))
		{
			case RUN_ENCODED:
			{
				// 根据<起始位置>和<字串长度>还原出预视视窗中的内容
				start = read_bits(start_bit, &input);
				length = read_bits(length_bit, &input);
				
				if(start == -1 || length == -1)
				{
					goto DECODE_END;
				}

				// 调整长度和开始位置
				length += THRESHOLD;
				start--;

				while (BUFFER_SIZE + start < write_index)
					start += BUFFER_SIZE;
				
				start += output.start;

				// 复制前面出现过的相同数据
				for(i = 0; i < length; i++)
				{
					dst[write_index++] = dst[start++];
				}

				break;
			}
			
			case RAW_ENCODED:
			{
				// 根据<跟随字元>还原出预视视窗中的内容
				dst[write_index++] = (unsigned char)read_bits(ALPHA_BIT, &input);
				//length = 1;

				break;
			}
			
			default:
				goto DECODE_END;
		}
	}

DECODE_END:	
	*dst_length = write_index;

	return 0;
}