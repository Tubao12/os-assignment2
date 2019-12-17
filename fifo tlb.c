
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PAGE_TABLE_ENTRIES 256
#define PAGE_SIZE  256
#define FRAME_SIZE 256   //128
#define TLB_SIZE   16


//masks to get page number and offset
#define PAGE_NUMBER_MASK 0x0000FFFF
#define OFFSET_MASK 0x000000FF

int pageTable[PAGE_TABLE_ENTRIES];
int PhyMem[65536];                      //physical memory (256*256) and size limit
int TLBPageNum[TLB_SIZE];
int TLBFrameNum[TLB_SIZE];


int main(int argc, char *argv[])
{
    int total_bytes;
    unsigned int laddr;              //logical address read in from addresses.txt
    unsigned int paddr;              //physical address in memory
    unsigned int value;              //signed byte value in phy mem
    unsigned int page_number;
    unsigned int page_offset;

    char buf[256];

    int total_addr = 0;         //keeps track of how many addresses read
    int page_faults = 0;        //keeps track of total page faults
    int frameNum = 0;           //physical frame
    int TLB_size = 0;           //keeps track of TLB size
    int TLB_index = 0;
    int TLB_hits = -1;           //keeps track of how many TLB hits

    /* initializes all entries into page table to -1 */
	int i = 0;
    for( i = 0; i < PAGE_TABLE_ENTRIES; i++)
	{
        pageTable[i] = -1;
    }

    /* initializes all entries in TLB to -1 */
    for(i = 0; i < TLB_SIZE; i++)
	{
        TLBPageNum[i] = -1;
        TLBFrameNum[i] = -1;
    }

    //files pointers too be used throughout program
    FILE* addressFile;                           //addresses.txt
    FILE* backingStore;                          //BACKING_STORE.bin
    FILE* output = fopen("out.txt", "w"); ;    //out3.txt

    /* used to read in binary input from backing store */
    backingStore = fopen("C:/Users/Lenovo/Documents/Tencent Files/1113775995/FileRecv/assignment2/BACKING_STORE.bin", "rb");

    /* error checking */
    if(backingStore == NULL)
	{
        printf("error opening BACKING_STORE.bin");
        return -1;
    }

    /* used to read in input from logical addresses */
    char* readIn = argv[1]; //reads in addresses.txt from command line to be read
    addressFile = fopen("C:/Users/Lenovo/Documents/Tencent Files/1113775995/FileRecv/assignment2/addresses.txt", "rt");

    /* error checking */
    if(addressFile == NULL)
	{
        printf("error opening addresses.txt");
        return -1;
    }

    while((fscanf(addressFile, "%d", &laddr)) == 1)
	{
        total_addr++;
        laddr = laddr & 65535;   
        page_number = (laddr & PAGE_NUMBER_MASK) >> 8;
        page_offset = laddr & OFFSET_MASK;
        int TLB_hit = -1;
        for( i = 0; i < TLB_size; i++)
		{
            if(TLBPageNum[i] == page_number)   //查看TLB中是否有目标页码 
			{
                TLB_hit = TLBFrameNum[i];
                paddr = TLB_hit * PAGE_SIZE + page_offset;
            }
        }
        if(!(TLB_hit == -1))    //若TLB中有目标页码，说明命中 
		{
            TLB_hits++;     //TLB命中数+1 
        }
		else if(pageTable[page_number] == -1)   //从.bin读取整帧内容 
		{
            fseek(backingStore, page_number*256, SEEK_SET);
            fread(buf, sizeof(char), 256, backingStore);
            pageTable[page_number] = frameNum;    
            for( i = 0; i < 256; i++)
			{
                int temp = (frameNum*256) + i;
                PhyMem[temp] = buf[i];
            }
            paddr = frameNum * PAGE_SIZE + page_offset;
            frameNum++;
            page_faults++;

            if(TLB_size == 16)
			{
                TLB_size--;
            }

            for(TLB_index = TLB_size; TLB_index > 0; TLB_index--) //FIFO策略 
			{
                TLBPageNum[TLB_index] = TLBPageNum[TLB_index - 1];
                TLBFrameNum[TLB_index] = TLBFrameNum[TLB_index - 1];
            }

            if(TLB_size <= 15)
			{
                TLB_size++;
            }

            TLBPageNum[0] = page_number;
            TLBFrameNum[0] = pageTable[page_number];
            paddr = pageTable[page_number] *256 + page_offset;

        }
		else    //从页中读取内容 
		{
            paddr = pageTable[page_number] *256 + page_offset;
        }

        value = PhyMem[paddr];
    //the signed byte value stored in physical memory at the translated physical address

        fprintf(output, "%d\n", value);
    }

    //report page-fault rate

    /* rate = (page faults / total addresses) * 100 */
    float page_fault_rate = ((float)page_faults / (float)total_addr) * 100;
    printf("TLB_hits is %d\n",TLB_hits);
    printf("Page-fault rate: %.2f%%\n", page_fault_rate);

    

    //report TLB hit rate <----- do NOT expect to be high
    /* rate = (TLB hits / total addresses)* 100 */
    float TLB_hit_rate = ((float)TLB_hits / (float)total_addr) * 100;
    printf("TLB hit rate: %.2f%%\n", TLB_hit_rate);

    /* close files */

    fclose(addressFile);
    fclose(backingStore);
    fclose(output);
    return 0;
}
