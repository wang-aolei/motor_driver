#include "stdarg.h"
#include "string.h"
#include "sys.h"
#include "mprintf.h"
#include <stdlib.h>

void printflt(double flt);

static unsigned long m_pow(int x,int y)
{
      unsigned long sum = 1;
      while(y--)
      {
           sum *= x;
      }
      return sum;
}

//��ӡ�ַ�
void m_putchar(const char ch)
{
	while((USART1->SR&0X40)==0);//�ȴ���һ�δ������ݷ������  
			USART1->DR = ch;      	//дDR,����1����������
	//		return ch;
}
 
//��ӡ�ַ���
void m_putstr(const char *str)
{
      while(*str)
      {
            m_putchar(*str++);
      }
}


int m_printf(const char *str,...)
{
     va_list ap;              //����һ���ɱ�����ģ��ַ�ָ�룩 
     int val,r_val;
     char count,ch;
     char *s = NULL;
     int res = 0;             //����ֵ
     va_start(ap,str);        //��ʼ��ap
     while('\0' != *str)//strΪ�ַ���,�������һ���ַ��϶���'\0'���ַ����Ľ�������
     {
          switch(*str)
          {
              case '%':	     //���Ͳ���
              str++;
              switch(*str)
              {
                   case'd': //10�������
                        val = va_arg(ap, int); 
												r_val = val; 
                        count = 0; 
                        while(r_val)
                        {
                             count++;         //�����ĳ���
                             r_val /= 10;
                        }
                        res += count;         //����ֵ��������? 
                        r_val = val; 
                        while(count)
                        {
                              ch = r_val / m_pow(10,count - 1);
                              r_val %= m_pow(10,count - 1);
                              m_putchar(ch + '0');   //���ֵ��ַ���ת�� 
                              count--;
                        }
                        break;
                  case'x': //16������� 
                        val = va_arg(ap, int); 
                        r_val = val; 
                        count = 0;
                        while(r_val)
                        {
                             count++;     //�����ĳ��� 
                             r_val /= 16; 
                        } 
                        res += count;     //����ֵ�������� 
                        r_val = val; 
                        while(count) 
                        {
                              ch = r_val / m_pow(16, count - 1); 
                              r_val %= m_pow(16, count - 1); 
                              if(ch <= 9)
                                  m_putchar(ch + '0'); 	//���ֵ��ַ���ת�� 
                              else 
                                  m_putchar(ch - 10 + 'a'); 
                              count--;
                        } 
                 break;
                 case 's':         //�����ַ��� 
                      s = va_arg(ap, char *);	
                      m_putstr(s);         //�ַ���,����ֵΪ�ַ�ָ�� 
                      res += strlen(s);     //����ֵ�������� 
                 break; 
                 case 'c': 
                      m_putchar( (char)va_arg(ap, int )); //��Ҳ�Ϊʲô��дchar����Ҫдint 
                      res += 1; 
                 break;
								 default :;
             }
             break;
          case '\n':
               m_putchar('\n'); 
               res += 1;
               break;
          case '\r':
               m_putchar('\r'); 
               res += 1;
               break;
          default :         						 //��ʾԭ���ĵ�һ���������ַ���(����..��Ĳ���o)
               m_putchar(*str);
               res += 1;
          }
         str++;
     }
     va_end(ap);
     return res;
}






