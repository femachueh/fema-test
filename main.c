#include <stdio.h>
#include <stdlib.h>
#include "heatshrink.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

uint8_t outBuf[15360];
uint8_t inBuf[10240];
FILE *fpOut=NULL ;

int outCallback(uint32_t outSize)
{ 
   printf("Outsize =%d ",outSize);
   fwrite(outBuf,1,outSize,fpOut);    
   return 0;
}

#if 1 // decompress
int main(int argc, char *argv[]) {			
  FILE *fp=NULL ; uint32_t inBufSize; CompressHandle handle;
  fpOut = (FILE *)fopen("testDecode.fit","wb"); 
  fp = (FILE *)fopen("testEncode.fit","rb"); 
  if(fp != 0)
  {
    inBufSize = fread(inBuf,1,sizeof(inBuf),fp);
    if (inBufSize != 0)
      handle = CompressInit(inBuf, inBufSize, outBuf, sizeof(outBuf), outCallback, COMPRESS_OP_DECOMPRESS);        
    while (inBufSize != 0)    
    {
       CompressPoll(handle,inBufSize); // may callback multiple times    
       inBufSize = fread(inBuf,1,sizeof(inBuf),fp);
    }                  
    CompressFinish(handle); // may callback multiple times
    fclose(fpOut);
    fclose(fp);
  }
  else
    printf(" Open test.fit fail ");    
  getchar(); 
//  system("pause");		
	return 0;
}
#endif

#if 0 // compress            
int main(int argc, char *argv[]) {			
  FILE *fp=NULL ; uint32_t inBufSize; CompressHandle handle;
  fpOut = (FILE *)fopen("testEncode.fit","wb"); 
  fp = (FILE *)fopen("test.fit","rb"); 
  uint8_t inBuf[512];
  if(fp != 0)
  {
    inBufSize = fread(inBuf,1,sizeof(inBuf),fp);
    if (inBufSize != 0)
      handle = CompressInit(inBuf, inBufSize, outBuf, sizeof(outBuf), outCallback, COMPRESS_OP_COMPRESS);    
    
//    CompressPoll(handle,inBufSize); // may callback multiple times    
//    inBufSize = fs_fread(inBuf,1,sizeof(inBuf),fp);
    while (inBufSize != 0)    
    {
       CompressPoll(handle,inBufSize); // may callback multiple times    
       inBufSize = fread(inBuf,1,sizeof(inBuf),fp);
    }                  
    CompressFinish(handle); // may callback multiple times
    fclose(fpOut);
    fclose(fp);
  }
  else
    printf(" Open test.fit fail ");    
//  system("pause");		
  getchar();
  return 0;
  
}
#endif
