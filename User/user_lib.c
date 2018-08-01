#include "user_inc.h"
#include "stm32f10x_lib.h"

u8 JumpErrorFlag=0;

u16 memcompare(u8* a,u8* b,u16 num)
{
  u16 i,re;
  re=0;
  for(i=0;i<num;i++)
  {
    if(a[i]==b[i]) re++;
    else break;
  }
  return re;
}

typedef  void (*pFunction)(void);
void JumpToAddr(u32 programAddr)
{
    pFunction Jump_To_Application;
    vu32 JumpAddress;
    if (((*(vu32*)programAddr) & 0x2FFF0000 ) == 0x20000000) /*固件区正常*/
    { 
                JumpAddress = *(vu32*) (programAddr + 4); 
                Jump_To_Application = (pFunction) JumpAddress; 
                __MSR_MSP(*(vu32*) programAddr); 
                Jump_To_Application(); 
    }
    else
    {
      JumpErrorFlag=1;
    }
}

void memcopy(u8* src,u8* dst,u16 num)
{
  u16 i;
  for(i=0;i<num;i++) dst[i]=src[i];
}

u8 Reload_FactoryProgram(u32 src_addr,u32 dst_addr,u32 byte_length)
{
    u32* ptr=(u32*)src_addr;
    u32 i;
    u32 page_loop;
    u32 dst_addr_inc=dst_addr;
    /* FLASH解锁*/
    FLASH_Unlock(); 

    /* 清除FLASH标志位 */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
    
    FLASH_ErasePage(dst_addr);/*擦出第一页*/
    page_loop=0;
    
    for( i=0; i<byte_length; i+=4 )
    {
      if(page_loop>=FLASH_PAGE_SIZE)//有需要则擦擦FLASH
      {
        FLASH_ErasePage(dst_addr_inc);
        page_loop=0;
      }
      
      FLASH_ProgramWord(dst_addr_inc, *ptr++);
      page_loop+=4;
      dst_addr_inc+=4;
    }
    
    /*锁上FLASH*/
    FLASH_Lock();
    return 1;
}

u8 EraseAndPro_FLASH_Less1PAGE(u32* src_buf,u32 WORD_LEN,u32 flash_addr)
{
    u32* ptr=src_buf;
    u32 i;
    //u32 page_loop;
    u32 dst_addr_inc=flash_addr;
    /* FLASH解锁*/
    FLASH_Unlock(); 

    /* 清除FLASH标志位 */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
    
    FLASH_ErasePage(dst_addr_inc);/*擦出第一页*/
    //page_loop=0;
    
    for( i=0; i<WORD_LEN; i++ )
    {
      FLASH_ProgramWord(dst_addr_inc, *ptr++);
      dst_addr_inc+=4;
    }
    
    /*锁上FLASH*/
    FLASH_Lock();
    return 1;
}

s32 lmax(s32 A,s32 B)
{
  s32 Re=A;
  if(B>A) Re=B;
  return Re;
}
s32 lmin(s32 A,s32 B)
{
  s32 Re=A;
  if(B<A) Re=B;
  return Re;
}


s16 StartPWMFilter(s16 t0,s16 t1)
{
#define UP_STEP 2
  if(t1>(t0+UP_STEP))
  {
    return t0+UP_STEP;
  }
  else if(t1<(t0-UP_STEP))
  {
    return t0-UP_STEP;
  }
  return t1;
}

float GetAutoTiltAngleK(float CurrentAngle,float sd_k)
{
  #define Angle_STEP_0  0.017
  #define Angle_STEP_1  0.030
  #define Angle_STEP_2  0.040
  #define Angle_STEP_3  0.050
  #define Angle_STEP_4  0.090
  
  return sd_k;
  /*
  if((CurrentAngle>-Angle_STEP_0)&&(CurrentAngle<Angle_STEP_0))
  {
    return sd_k;
  }
  else if((CurrentAngle>-Angle_STEP_1)&&(CurrentAngle<Angle_STEP_1))
  {
    return sd_k;
  }
  else if((CurrentAngle>-Angle_STEP_2)&&(CurrentAngle<Angle_STEP_2))
  {
    return sd_k*1.1;
  }
  else if((CurrentAngle>-Angle_STEP_3)&&(CurrentAngle<Angle_STEP_3))
  {
    return sd_k*1.3;
  }
  else
  {
    return sd_k*1.5;
  }
  */
}

u32 abs_32(s32 value)
{
  if(value<0) value=-value;
  return value;
}

float roundf(float a)
{
  if(a < 0.0) a += (-0.5);
  else a += 0.5;
  a = (float)((s32)a);
  return (a);
}