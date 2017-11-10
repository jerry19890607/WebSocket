#ifndef __ENCODER_H__
#define __ENCODER_H__
   
#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  
  
const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";   
char* base64_encode(const char* data, int data_len);   
char *base64_decode(const char* data, int data_len);   
static char find_pos(char ch);   
  
/* */   
char *base64_encode(const char* data, int data_len)   
{   
    //int data_len = strlen(data);   
    int prepare = 0;   
    int ret_len;   
    int temp = 0;   
    char *ret = NULL;   
    char *f = NULL;   
    int tmp = 0;   
    char changed[4];   
    int i = 0;   
    ret_len = data_len / 3;   
    temp = data_len % 3;   
    if (temp > 0)   
    {   
        ret_len += 1;   
    }   
    ret_len = ret_len*4 + 1;   
    ret = (char *)malloc(ret_len);   
        
    if ( ret == NULL)   
    {   
        printf("No enough memory.\n");   
        exit(0);   
    }   
    memset(ret, 0, ret_len);   
    f = ret;   
    while (tmp < data_len)   
    {   
        temp = 0;   
        prepare = 0;   
        memset(changed, '\0', 4);   
        while (temp < 3)   
        {   
            //printf("tmp = %d\n", tmp);   
            if (tmp >= data_len)   
            {   
                break;   
            }   
            prepare = ((prepare << 8) | (data[tmp] & 0xFF));   
            tmp++;   
            temp++;   
        }   
        prepare = (prepare<<((3-temp)*8));   
        //printf("before for : temp = %d, prepare = %d\n", temp, prepare);   
        for (i = 0; i < 4 ;i++ )   
        {   
            if (temp < i)   
            {   
                changed[i] = 0x40;   
            }   
            else   
            {   
                changed[i] = (prepare>>((3-i)*6)) & 0x3F;   
            }   
            *f = base[changed[i]];   
            //printf("%.2X", changed[i]);   
            f++;   
        }   
    }   
    *f = '\0';   
        
    return ret;   
        
}   
/* */   
static char find_pos(char ch)     
{   
    char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[]   
    return (ptr - base);   
}   
/* */   
char *base64_decode(const char *data, int data_len)   
{   
    int ret_len = (data_len / 4) * 3;   
    int equal_count = 0;   
    char *ret = NULL;   
    char *f = NULL;   
    int tmp = 0;   
    int temp = 0;   
    char need[3];   
    int prepare = 0;   
    int i = 0;   
    if (*(data + data_len - 1) == '=')   
    {   
        equal_count += 1;   
    }   
    if (*(data + data_len - 2) == '=')   
    {   
        equal_count += 1;   
    }   
    if (*(data + data_len - 3) == '=')   
    {//seems impossible   
        equal_count += 1;   
    }   
    switch (equal_count)   
    {   
    case 0:   
        ret_len += 4;//3 + 1 [1 for NULL]   
        break;   
    case 1:   
        ret_len += 4;//Ceil((6*3)/8)+1   
        break;   
    case 2:   
        ret_len += 3;//Ceil((6*2)/8)+1   
        break;   
    case 3:   
        ret_len += 2;//Ceil((6*1)/8)+1   
        break;   
    }   
    ret = (char *)malloc(ret_len);   
    if (ret == NULL)   
    {   
        printf("No enough memory.\n");   
        exit(0);   
    }   
    memset(ret, 0, ret_len);   
    f = ret;   
    while (tmp < (data_len - equal_count))   
    {   
        temp = 0;   
        prepare = 0;   
        memset(need, 0, 4);   
        while (temp < 4)   
        {   
            if (tmp >= (data_len - equal_count))   
            {   
                break;   
            }   
            prepare = (prepare << 6) | (find_pos(data[tmp]));   
            temp++;   
            tmp++;   
        }   
        prepare = prepare << ((4-temp) * 6);   
        for (i=0; i<3 ;i++ )   
        {   
            if (i == temp)   
            {   
                break;   
            }   
            *f = (char)((prepare>>((2-i)*8)) & 0xFF);   
            f++;   
        }   
    }   
    *f = '\0';   
    return ret;   
}  
  
int tolower(int c)   
{   
    if (c >= 'A' && c <= 'Z')   
    {   
        return c + 'a' - 'A';   
    }   
    else   
    {   
        return c;   
    }   
}   
  
int htoi(const char s[],int start,int len)   
{   
  int i,j;   
    int n = 0;   
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) //判断是否有前导0x或者0X  
    {   
        i = 2;   
    }   
    else   
    {   
        i = 0;   
    }   
    i+=start;  
    j=0;  
    for (; (s[i] >= '0' && s[i] <= '9')   
       || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >='A' && s[i] <= 'F');++i)   
    {     
        if(j>=len)  
    {  
      break;  
    }  
        if (tolower(s[i]) > '9')   
        {   
            n = 16 * n + (10 + tolower(s[i]) - 'a');   
        }   
        else   
        {   
            n = 16 * n + (tolower(s[i]) - '0');   
        }   
    j++;  
    }   
    return n;   
}   
  
  
typedef struct SHA1Context{  
    unsigned Message_Digest[5];        
    unsigned Length_Low;               
    unsigned Length_High;              
    unsigned char Message_Block[64];   
    int Message_Block_Index;           
    int Computed;                      
    int Corrupted;                     
} SHA1Context;  
  
void SHA1Reset(SHA1Context *);  
int SHA1Result(SHA1Context *);  
void SHA1Input( SHA1Context *,const char *,unsigned);  

#define SHA1CircularShift(bits,word) ((((word) << (bits)) & 0xFFFFFFFF) | ((word) >> (32-(bits))))  
  
void SHA1ProcessMessageBlock(SHA1Context *);  
void SHA1PadMessage(SHA1Context *);  
  
void SHA1Reset(SHA1Context *context){// 初始化动作  
    context->Length_Low             = 0;  
    context->Length_High            = 0;  
    context->Message_Block_Index    = 0;  
  
    context->Message_Digest[0]      = 0x67452301;  
    context->Message_Digest[1]      = 0xEFCDAB89;  
    context->Message_Digest[2]      = 0x98BADCFE;  
    context->Message_Digest[3]      = 0x10325476;  
    context->Message_Digest[4]      = 0xC3D2E1F0;  
  
    context->Computed   = 0;  
    context->Corrupted  = 0;  
}  
  
int SHA1Result(SHA1Context *context){// 成功返回1，失败返回0  
    if (context->Corrupted) {  
        return 0;  
    }  
    if (!context->Computed) {  
        SHA1PadMessage(context);  
        context->Computed = 1;  
    }  
    return 1;  
}  
  
  
void SHA1Input(SHA1Context *context,const char *message_array,unsigned length){  
    if (!length) return;  
  
    if (context->Computed || context->Corrupted){  
        context->Corrupted = 1;  
        return;  
    }  
  
    while(length-- && !context->Corrupted){  
        context->Message_Block[context->Message_Block_Index++] = (*message_array & 0xFF);  
  
        context->Length_Low += 8;  
  
        context->Length_Low &= 0xFFFFFFFF;  
        if (context->Length_Low == 0){  
            context->Length_High++;  
            context->Length_High &= 0xFFFFFFFF;  
            if (context->Length_High == 0) context->Corrupted = 1;  
        }  
  
        if (context->Message_Block_Index == 64){  
            SHA1ProcessMessageBlock(context);  
        }  
        message_array++;  
    }  
}  
  
void SHA1ProcessMessageBlock(SHA1Context *context){  
    const unsigned K[] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };  
    int         t;                  
    unsigned    temp;               
    unsigned    W[80];              
    unsigned    A, B, C, D, E;      
  
    for(t = 0; t < 16; t++) {  
    W[t] = ((unsigned) context->Message_Block[t * 4]) << 24;  
    W[t] |= ((unsigned) context->Message_Block[t * 4 + 1]) << 16;  
    W[t] |= ((unsigned) context->Message_Block[t * 4 + 2]) << 8;  
    W[t] |= ((unsigned) context->Message_Block[t * 4 + 3]);  
    }  
      
    for(t = 16; t < 80; t++)  W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);  
  
    A = context->Message_Digest[0];  
    B = context->Message_Digest[1];  
    C = context->Message_Digest[2];  
    D = context->Message_Digest[3];  
    E = context->Message_Digest[4];  
  
    for(t = 0; t < 20; t++) {  
        temp =  SHA1CircularShift(5,A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];  
        temp &= 0xFFFFFFFF;  
        E = D;  
        D = C;  
        C = SHA1CircularShift(30,B);  
        B = A;  
        A = temp;  
    }  
    for(t = 20; t < 40; t++) {  
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];  
        temp &= 0xFFFFFFFF;  
        E = D;  
        D = C;  
        C = SHA1CircularShift(30,B);  
        B = A;  
        A = temp;  
    }  
    for(t = 40; t < 60; t++) {  
        temp = SHA1CircularShift(5,A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];  
        temp &= 0xFFFFFFFF;  
        E = D;  
        D = C;  
        C = SHA1CircularShift(30,B);  
        B = A;  
        A = temp;  
    }  
    for(t = 60; t < 80; t++) {  
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];  
        temp &= 0xFFFFFFFF;  
        E = D;  
        D = C;  
        C = SHA1CircularShift(30,B);  
        B = A;  
        A = temp;  
    }  
    context->Message_Digest[0] = (context->Message_Digest[0] + A) & 0xFFFFFFFF;  
    context->Message_Digest[1] = (context->Message_Digest[1] + B) & 0xFFFFFFFF;  
    context->Message_Digest[2] = (context->Message_Digest[2] + C) & 0xFFFFFFFF;  
    context->Message_Digest[3] = (context->Message_Digest[3] + D) & 0xFFFFFFFF;  
    context->Message_Digest[4] = (context->Message_Digest[4] + E) & 0xFFFFFFFF;  
    context->Message_Block_Index = 0;  
}  
  
void SHA1PadMessage(SHA1Context *context){  
    if (context->Message_Block_Index > 55) {  
        context->Message_Block[context->Message_Block_Index++] = 0x80;  
        while(context->Message_Block_Index < 64)  context->Message_Block[context->Message_Block_Index++] = 0;  
        SHA1ProcessMessageBlock(context);  
        while(context->Message_Block_Index < 56) context->Message_Block[context->Message_Block_Index++] = 0;  
    } else {  
        context->Message_Block[context->Message_Block_Index++] = 0x80;  
        while(context->Message_Block_Index < 56) context->Message_Block[context->Message_Block_Index++] = 0;  
    }  
    context->Message_Block[56] = (context->Length_High >> 24 ) & 0xFF;  
    context->Message_Block[57] = (context->Length_High >> 16 ) & 0xFF;  
    context->Message_Block[58] = (context->Length_High >> 8 ) & 0xFF;  
    context->Message_Block[59] = (context->Length_High) & 0xFF;  
    context->Message_Block[60] = (context->Length_Low >> 24 ) & 0xFF;  
    context->Message_Block[61] = (context->Length_Low >> 16 ) & 0xFF;  
    context->Message_Block[62] = (context->Length_Low >> 8 ) & 0xFF;  
    context->Message_Block[63] = (context->Length_Low) & 0xFF;  
  
    SHA1ProcessMessageBlock(context);  
}  
  
/* 
int sha1_hash(const char *source, char *lrvar){// Main 
    SHA1Context sha; 
    char buf[128]; 
 
    SHA1Reset(&sha); 
    SHA1Input(&sha, source, strlen(source)); 
 
    if (!SHA1Result(&sha)){ 
        printf("SHA1 ERROR: Could not compute message digest"); 
        return -1; 
    } else { 
        memset(buf,0,sizeof(buf)); 
        sprintf(buf, "%08X%08X%08X%08X%08X", sha.Message_Digest[0],sha.Message_Digest[1], 
        sha.Message_Digest[2],sha.Message_Digest[3],sha.Message_Digest[4]); 
        //lr_save_string(buf, lrvar); 
         
        return strlen(buf); 
    } 
} 
*/  
  
char * sha1_hash(const char *source){// Main  
    SHA1Context sha;  
    char *buf;//[128];  
  
    SHA1Reset(&sha);  
    SHA1Input(&sha, source, strlen(source));  
  
    if (!SHA1Result(&sha)){  
        printf("SHA1 ERROR: Could not compute message digest");  
        return NULL;  
    } else {  
      buf=(char *)malloc(128);  
        memset(buf,0,sizeof(buf));  
        sprintf(buf, "%08X%08X%08X%08X%08X", sha.Message_Digest[0],sha.Message_Digest[1],  
        sha.Message_Digest[2],sha.Message_Digest[3],sha.Message_Digest[4]);  
        //lr_save_string(buf, lrvar);  
          
        //return strlen(buf);  
        return buf;  
    }  
}  

#endif //__ENCODER_H__

