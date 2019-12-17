#include <stdio.h>
#include <stdlib.h>
 
extern void afunc(void);
 

 
int bss_var;//δ��ʼ��ȫ�ֱ����洢��BSS��
 
int data_var=42;//��ʼ��ȫ�ִ洢�����ݶ�
 
#define SHW_ADR(ID,I) printf("The %s is at address: %8x\n",ID,&I);//��ӡ��ַ��
 
int main(int argc,char *argv[])
{
	char *p,*b,*nb;

 
	SHW_ADR("main",main);//�鿴�����main����λ��
	
	SHW_ADR("afunc",afunc);//�鿴�����afunc����λ��
 
	printf("\nbss Locatoin:\n");
	SHW_ADR("bss_var",bss_var);//�鿴BSS�α�����ַ
 
	printf("\ndata Location:\n");
	SHW_ADR("data_var",data_var);//�鿴���ݶα�����ַ
 
	printf("\nStack Loation:\n");
	afunc();
	printf("\n");
 
	p=(char*)alloca(32);//��ջ�з���ռ�
 
	if(p!=NULL)
	{
		SHW_ADR("string p in stack start",*p);
		SHW_ADR("string p in stack end",*(p+32*sizeof(char)));
	}
 
	b=(char*)malloc(32*sizeof(char));//�Ӷ��з���ռ�
	nb=(char*)malloc(16*sizeof(char));//�Ӷ��з���ռ�
 
	printf("\nHeap Location:\n");
	SHW_ADR("allocated heap start",*b);//�ѷ���Ķѿռ����ʼ��ַ
	SHW_ADR("allocated heap end",*(nb+16*sizeof(char)));//�ѷ���Ķѿռ�Ľ�����ַ
 
	printf("\np,b and nb in stack\n");
	SHW_ADR("p",p);//��ʾջ������p�ĵ�ַ
	SHW_ADR("b",b);//��ʾջ������b�ĵ�ַ
	SHW_ADR("nb",nb);//��ʾջ������nb�ĵ�ַ
 
	free(b);//�ͷ�����Ŀռ䣬�Ա����ڴ�й¶
	free(nb);
}
 
void afunc(void)
{
	static int level=0;//��ʼ��Ϊ0�ľ�̬���ݴ洢��BSS����
 
	int stack_var;//�ֲ��������洢��ջ��
 
	if(++level==5)
		return;
 
	SHW_ADR("stack_var in stack section",stack_var);
	SHW_ADR("leval in bss section",level);
 
	afunc();
}

 

