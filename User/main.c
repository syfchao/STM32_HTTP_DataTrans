#include "stm32f10x.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "usart1.h"
#include "usart2.h"

#include "adc.h"
#include "delay.h"
#include "srf05.h"
#include "tim2.h"

#include "esp8266.h"

#define KEY "cd0101"

uint32_t HTTP_PostPkt(char *pkt, const char *key, int data);
int main(void)
{
  char HTTP_Buf[100];     //HTTP报文缓存区
  u16 len;
	
  u16 i, j, k;          //计数变量
	u8 array[500] = {0};
  u16 count[100] = {0};
  u16 temp = 0;
  u8 person_num = 0;
  u8 data = 0;
  u16 distance;
  
	Delay_Init();
	ADC_Config();
	
  USART1_Config();        //USART1作为调试串口
  USART2_Config();        //USART2用于连接ESP8266模块
  
	TIM2_Configuration();
	SRF05_Init();
	
  ESP8266_Init();         //ESP8266初始化
  printf("ESP8266 init over\r\n");
  
  while(1)
  {
    distance = SRF05_StartMeasure();
	  DelayMs(5000);
		
		printf("SRF :\r\n%d\r\n", distance);
		
	  if(distance < 20)
	  {
			person_num = 0;
			temp = 0;
			data = 0;
			memset(count, 0, sizeof(count));
			
	  	for(i = 0; i < 500; i++)            
      {
	      temp = adcConvertedValue;
				
				if((temp/10 -90) < 0)
					array[i] = 0;
				else
					array[i] = temp / 10 - 90;				
				DelayMs(100);
	    }
			
			for(k = 0; k < 5; k++)
			{
				temp = 0;
			  j = 0;
			
        for(i = 100 * k + 1; i < 100 * (k+1); i++)
	      {
	      	if((array[i] - array[i - 1]) >= 0 && array[i] > 0)
	      	{
			  		temp = temp + array[i] - array[i - 1];
	      	}
			  	else
			  	{
			  		count[j++] = temp;
			  		temp = 0;
          }
        }
				
			  for(i = 1; i < j && count[i] < 300; i++)
			  {					
			  	if((count[i - 1] < count[i]) && (count[i + 1] < count[i]))
			  	{
						if(count[i] > 30 && count[i] <= 60)
						{
							person_num = person_num + 1;
						}
						else if(count[i] > 60 && count[i] <= 90)
						{
							person_num = person_num + 2;
						}
						else if(count[i] > 90)
						{
							person_num = person_num + 3;
						}
			  	}
        }
				
				printf("person_num:%d\r\n", person_num);
		  }
			
		  data = person_num;
			
      USART2_Clear();
      len = HTTP_PostPkt(HTTP_Buf, KEY, data); //HTTP组包
      USART2_Write(USART2, (unsigned char *)(HTTP_Buf), len);			//报文发送
      printf("send HTTP msg:\r\n%s\r\n", HTTP_Buf);
        
      DelayMs(1000);
      printf("rcv response:\r\n%s\r\n", usart2_rcv_buf);
      DelayMs(1000);
		}
  }
}

