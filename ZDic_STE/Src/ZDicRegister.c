#include <PalmOS.h>
#include "ZDicRegister.h"


#pragma mark -


// GetCode(sName: string): string;
// var i,j,c0,c1,m,r: integer;
// sync,hash,rcode: array[0..16] of integer;
// 
// begin Result := '';
// 
// for i:=0 to 15 do hash[i] := 0;
// 
// hash[0]:=$64;
// hash[1]:=$43;
// hash[2]:=$4A;
// hash[3]:=$4B;
//
// for i:=0 to 11 do rcode[i] := 0;
//
// c0 := length(sName);
// 
// for i:=0 to c0-1 do sync[i] := ord(sName[i+1]);
// 
// c1 := ((c0+15) shr 4) shl 4;
//
// for i:=0 to c1-1 do hash[i mod 16] := (hash[i mod 16]+sync[i mod c0]+i-c0) and $FF;
//	for i:=0 to 5 do
//	begin
//		m := 0;
//		for j:=0 to min(11,15-i) do
//			m := m+((hash[i+j]-32) shl (j mod 4)) and $FFFF;
//		m := m+hash[15-i]-48;
//		m := m and $FF;
//		rcode[2*i] := m shr 4;
//		rcode[2*i+1] := m and $0F;
//	end;
//
//	for i:=0 to 11 do
//		begin r := rcode[i] and $3F;
//		if r<10 then rcode[i] := r+48
//		else if r<20 then rcode[i] := r+87
//		else if r<35 then rcode[i] := r+88
//		else rcode[i] := r+48;
//	end; 
//
//	for i:=0 to 11 do Result := Result + Chr(rcode[i]);
//	
//	end;

/*
#define kCodeBufSize	16

#define kHashCode0		0x64
#define kHashCode1		0x43
#define kHashCode2		0x4A
#define kHashCode3		0x4B
#define kHashCodeNum	4

#define min(a, b) (((a) < (b)) ? (a) : (b))

//xurunhua: 725df00d1400
void GetIndex(Char *desCode, const Char *srcCode)
{
	Int16	i, j, c0, c1, m, r;	
	Int16	sync[kCodeBufSize], hash[kCodeBufSize], rcode[kCodeBufSize];
	
	for (i = 0; i < kCodeBufSize; i++)
		hash[i] = 0;
	
	hash[0] = kHashCode0;
	hash[1] = kHashCode1;
	hash[2] = kHashCode2;
	hash[3] = kHashCode3;

	// 清除目标串缓冲区。
	for (i = 0; i < (kCodeBufSize - kHashCodeNum); i++)
		rcode[i] = 0;
			
	// 将源串复制到缓冲区。
	c0 = StrLen(srcCode);
	if (c0 > kCodeBufSize) c0 = kCodeBufSize;
	for (i = 0; i <= c0 - 1; i++)
		sync[i] = srcCode[i];
		
	// 将长度限制在16的整数倍，+15是为了避免长度为0。
	c1 = ((c0 + (kCodeBufSize - 1)) >> 4) << 4;
	
	for (i = 0; i <= c1 - 1; i++)
		hash[i % kCodeBufSize] = (hash[i % kCodeBufSize] + sync[i % c0] + i - c0) & 0xff;
	
	for (i = 0; i <= 5; i++)
	{
		m = 0;
		for (j = 0; j <= min(11, 15 - i); j++)
			m = m + ((hash[i + j] - 32) << (j % 4)) & 0xffff;
		m = m + hash[15 - i] - 48;
		m = m & 0xff;
		rcode[2 * i] = m >> 4;
		rcode[2 * i + 1] = m & 0x0f;
	}
	
	for (i = 0; i <= 11; i++)
	{
		r = rcode[i] & 0x3f;
		if (r < 10) rcode[i] = r + 48;
		else if (r < 20) rcode[i] = r + 87;
		else if (r < 35) rcode[i] = r +88;
		else rcode[i] = r + 48;
	}
	
	for (i = 0; i <= 11; i++)
		desCode[i] = rcode[i];
	
	desCode[i] = chrNull;
	
	return;
}
*/