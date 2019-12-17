#include <stdio.h>
#include <stdlib.h>
 
extern void afunc(void);
 

 
int bss_var;//未初始化全局变量存储在BSS段
 
int data_var=42;//初始化全局存储在数据段
 
#define SHW_ADR(ID,I) printf("The %s is at address: %8x\n",ID,&I);//打印地址宏
 
int main(int argc,char *argv[])
{
	char *p,*b,*nb;

 
	SHW_ADR("main",main);//查看代码段main函数位置
	
	SHW_ADR("afunc",afunc);//查看代码段afunc函数位置
 
	printf("\nbss Locatoin:\n");
	SHW_ADR("bss_var",bss_var);//查看BSS段变量地址
 
	printf("\ndata Location:\n");
	SHW_ADR("data_var",data_var);//查看数据段变量地址
 
	printf("\nStack Loation:\n");
	afunc();
	printf("\n");
 
	p=(char*)alloca(32);//从栈中分配空间
 
	if(p!=NULL)
	{
		SHW_ADR("string p in stack start",*p);
		SHW_ADR("string p in stack end",*(p+32*sizeof(char)));
	}
 
	b=(char*)malloc(32*sizeof(char));//从堆中分配空间
	nb=(char*)malloc(16*sizeof(char));//从堆中分配空间
 
	printf("\nHeap Location:\n");
	SHW_ADR("allocated heap start",*b);//已分配的堆空间的起始地址
	SHW_ADR("allocated heap end",*(nb+16*sizeof(char)));//已分配的堆空间的结束地址
 
	printf("\np,b and nb in stack\n");
	SHW_ADR("p",p);//显示栈中数据p的地址
	SHW_ADR("b",b);//显示栈中数据b的地址
	SHW_ADR("nb",nb);//显示栈中数据nb的地址
 
	free(b);//释放申请的空间，以避免内存泄露
	free(nb);
}
 
void afunc(void)
{
	static int level=0;//初始化为0的静态数据存储在BSS段中
 
	int stack_var;//局部变量，存储在栈区
 
	if(++level==5)
		return;
 
	SHW_ADR("stack_var in stack section",stack_var);
	SHW_ADR("leval in bss section",level);
 
	afunc();
}

 

