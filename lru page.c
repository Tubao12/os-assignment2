#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PAGE_TABLE_ENTRIES 256//because physical addr space is smaller
#define PAGE_SIZE  256
#define FRAME_SIZE 256      
#define TLB_SIZE   16

//masks to get page number and offset
#define PAGE_NUMBER_MASK 0x0000FFFF
#define OFFSET_MASK 0x000000FF

int pageTable[PAGE_TABLE_ENTRIES][2];//second number is validity bit
int PhyMem[32768];    //physical memory (128 frames *256 bytes in each frame) and size limit

int TLBPageNum[TLB_SIZE];
int TLBFrameNum[TLB_SIZE];

int flag=-1;       //���ڱ����Ҫ�ƶ���λ�ã�ʵ��LRU
int temp_TLBPageNum=0;          //�����м��� 
int temp_TLBFrameNum=0;
int framememory[128];     // 


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
    int freepgfrm = 0;            //Keeps track of free page frames.
    int j = 0;//If 127, page table is full
    int k = 0;//For calling back into FIFOcnt
    int l = 0;//For counting number of 127 iterations reached

    int temp = 0;
    int temp2 = 0;
    int pgnumarr[1000];

    int TLB_size = 0;           //keeps track of TLB size
    int TLB_index = 0;
    int TLB_hits = -1;           //keeps track of how many TLB hits

    /* initializes all entries into page table to -1 */
    int i = 0;
    for( i = 0; i < PAGE_TABLE_ENTRIES; i++)
	{
        pageTable[i][0] = -1;
        pageTable[i][1] = 0;//0 invalid, 1 valid
    }

    /* initializes all entries in TLB to -1 */
    for( i = 0; i < TLB_SIZE; i++)
	{
        TLBPageNum[i] = -1;
        TLBFrameNum[i] = -1;
    }

    //files pointers too be used throughout program
    FILE* addressFile;                           //addresses.txt
    FILE* backingStore;                          //BACKING_STORE.bin
    FILE* output = fopen("out.txt", "w"); ;    //out3.txt

    /* used to read in binary input from backing store */
    backingStore = fopen("BACKING_STORE.bin", "rb");

    /* error checking */
    if(backingStore == NULL)
	{
        printf("error opening BACKING_STORE.bin");
        return -1;
    }

    /* used to read in input from logical addresses */
    char* readIn = argv[1]; //reads in addresses.txt from command line to be read
    addressFile = fopen(readIn, "rt");

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
        pgnumarr[j] = page_number;     //j��0��ʼ�� 
        page_offset = laddr & OFFSET_MASK;
        j++;
        int TLB_hit = -1;
         
        for( i = 0; i < TLB_size; i++)
		{ 
		    //���ж�ҳ���Ƿ���Ч  
            if((TLBPageNum[i] == page_number)&&(pageTable[page_number][1]==1))
			{
                TLB_hit = TLBFrameNum[i];
                paddr = TLB_hit * PAGE_SIZE + page_offset;
                value = PhyMem[paddr];
        		fprintf(output, "%d\n", value); 
                flag = i;
                break;
            }
        }

        if(!(TLB_hit == -1))
		{
            TLB_hits++;
            temp_TLBPageNum=TLBPageNum[flag];          
			temp_TLBFrameNum=TLBFrameNum[flag]; 
        
            for(TLB_index = flag; TLB_index > 0; TLB_index--) //��λ��i��ʼ����ǰ������κ��� 
			{
                TLBPageNum[TLB_index] = TLBPageNum[TLB_index - 1];
                TLBFrameNum[TLB_index] = TLBFrameNum[TLB_index - 1];
            }
            TLBPageNum[0] = temp_TLBPageNum;
            TLBFrameNum[0] = temp_TLBFrameNum;	
        }
        
        else
        {
        	

        /////REMEMBER TO SET FRAME NUM WHERE NEEDED.
        if(pageTable[page_number][0] == -1)
        	{
            if(l != 0)
				{//Memory is full, �������ǩ��ָʾ��Զһ����ֱ���滻���ɣ���������ʵ��ѭ������ 

                //First we need to invalidate the page in the page table that is currently holding the frame in memory we have to replace
                temp2 = framememory[frameNum];//Page number that we need to invalidate,��Ҫ���滻��Page number 
                pageTable[temp2][1] = 0;//Invalidates page, 
                

                //Now we need to load the requested page into the frame

                fseek(backingStore, page_number*256, SEEK_SET);
                fread(buf, sizeof(char), 256, backingStore);                
				int lpcnt = 0;
                for( lpcnt = 0; lpcnt < 256; lpcnt++)
				{
                    temp = (frameNum * 256) + lpcnt;      //frame�𵽴�ͷ��ʼ��������� 
                    PhyMem[temp] = buf[lpcnt];   //PhyMem�ڵ����ݿ�ʼ��0���ı� 
                }

                //Now we can store the frame number into the page table and validate the page.
                pageTable[page_number][0] = frameNum;
                pageTable[page_number][1] = 1;
                framememory[frameNum] = page_number;
                paddr = pageTable[page_number][0] * PAGE_SIZE + page_offset;
                frameNum++;
                page_faults++;
                value = PhyMem[paddr];
        		fprintf(output, "%d\n", value); 
            }

            else{//Memory isn't full, so add page to page table and insert page into frame

                //First, load page into memory
                fseek(backingStore, page_number*256, SEEK_SET);
                fread(buf, sizeof(char), 256, backingStore);                
				int lpcnt = 0;
                for( lpcnt = 0; lpcnt < 256; lpcnt++)
				{
                    temp = (frameNum * 256) + lpcnt;
                    PhyMem[temp] = buf[lpcnt];
                }

                //Then, load frame number into page table
                pageTable[page_number][0] = frameNum;
                pageTable[page_number][1] = 1;
                framememory[frameNum] = page_number;
                paddr = pageTable[page_number][0] * PAGE_SIZE + page_offset;
                frameNum++;
                page_faults++;
                value = PhyMem[paddr];
        		fprintf(output, "%d\n", value); 
            }

            if(frameNum > 127)

            {
                k = 128;   
                frameNum = 0;
                l++;     //l�仯1�����潫�仯128 
            }
        }

        else if(pageTable[page_number][0]!=-1)//If the page in the page table isn't empty
        {
            if(pageTable[page_number][1] == 0)
			{//If the page in the page table is invalid
                //First we need to load a new frame into memory

				temp2 = framememory[frameNum];//Page number that we need to invalidate,��Ҫ���滻��Page number 
                pageTable[temp2][1] = 0;//Invalidates page, 
                
                fseek(backingStore, page_number*256, SEEK_SET);
                fread(buf, sizeof(char), 256, backingStore);               
				int lpcnt = 0;
                for( lpcnt = 0; lpcnt < 256; lpcnt++)
				{
                    temp = (frameNum * 256) + lpcnt;
                    PhyMem[temp] = buf[lpcnt];
                }

                //Then, load that frame number into page table
                pageTable[page_number][0] = frameNum;
                pageTable[page_number][1] = 1;//And make the page valid
                framememory[frameNum] = page_number;
                paddr = pageTable[page_number][0] * PAGE_SIZE + page_offset;//Recalculate paddress
                frameNum++;
                page_faults++;
                value = PhyMem[paddr];
        		fprintf(output, "%d\n", value); 
        		
        		//������ҳ��ʧЧ�����ӦTLB����ʱ������ 
            }
            
            
            //LRU��ҳ�û���FIFO��ҳ�û������﷢����ͬ������Ҫ������ȡ�����µ�λ�ã�����������ƶ�һλ��
			//ͨ��ѭ������ԭ������� 
            else if(pageTable[page_number][1] == 1)
			{
			//Else the page is valid, so we retrieve the frame out of it and use it to calculate the physical address for the data we want to retrieve. This is the only outcome which results in a page hit.
				
				int current_frame = frameNum;       //�����ҳ�棬��Ҫ���滻����������ҳ��
				//First we retrieve the frame # from the page table
                temp = pageTable[page_number][0];//Temp is the frame number
				int lpcnt = 0;
				int temp_page_number;
				for (lpcnt=0; lpcnt<256;lpcnt++)
				{
					buf[lpcnt] = PhyMem[temp*256+lpcnt];
				}
				
				if (current_frame<temp)
				{
					int i=0;
					for (i=(temp*256+255);i>(current_frame*256+255);i--)
					{
						PhyMem[i] = PhyMem[i-256];
					}
					for (lpcnt=0; lpcnt<256;lpcnt++)
					{
						PhyMem[lpcnt+current_frame*256] = buf[lpcnt];
					}
					
					//��PageTable����framememory 
					for (i=current_frame;i<temp;i++)
					{
						temp_page_number = framememory[i];
						pageTable[temp_page_number][0] = i+1;
					}
				    //�ı�ҳ����Ӧ��ϵ 
					for (i=temp;i>current_frame;i--)
					{
						framememory[i] = framememory[i-1];
					}
					
				}
				
				else if (temp<current_frame)
				{
					if (l!=0)      //ҳ��full 
					{
						for (i=temp*256;i<current_frame*256;i++)
						{
							PhyMem[i] = PhyMem[i+256];
						}
						for (lpcnt=0; lpcnt<256;lpcnt++)
						{
							PhyMem[lpcnt+current_frame*256] = buf[lpcnt];
						}
						
						for (i=current_frame;i>temp;i--)
						{
							temp_page_number = framememory[i];
							pageTable[temp_page_number][0] = i-1;
						}
						for (i=temp;i<current_frame;i++)
						{
							framememory[i]=framememory[i+1];
						}
						
					}
										
					else    //ҳ�����п�λ,current_frame�ǿյ� 
					{							
						for (i=temp*256;i<(current_frame-1)*256;i++)
						{
							PhyMem[i] = PhyMem[i+256];
						}
						for (lpcnt=0; lpcnt<256;lpcnt++)
						{
							PhyMem[lpcnt+(current_frame-1)*256] = buf[lpcnt];
						}
						for (i=current_frame-1;i>temp;i--)
						{
							temp_page_number = framememory[i];
							pageTable[temp_page_number][0] = i-1;
						}
						for (i=temp;i<current_frame-1;i++)
						{
							framememory[i]=framememory[i+1];
						}
						frameNum--;
						
						
					
					}
				}				
                
                //Now we recalculate the frame number
                paddr = frameNum * PAGE_SIZE + page_offset; 
                pageTable[page_number][0]=frameNum;
                framememory[frameNum] = page_number;
                frameNum++;
                value = PhyMem[paddr];
        		fprintf(output, "%d\n", value); 
        		
        		//ע�����TLB���ڼ�¼������֡ҳ���ƶ�
        		for( i = 0; i < TLB_size; i++)
				{
					temp_page_number = TLBPageNum[i];
					TLBFrameNum[i] = pageTable[temp_page_number][0];
        		}
				 
            }

            if(frameNum > 127)
            {
               	k = 128;
                frameNum = 0;
                l++;
            }
        }

        if(TLB_size == 16)
		{
            TLB_size--;
        }

    
        for(TLB_index = TLB_size; TLB_index > 0; TLB_index--) 
		{
            TLBPageNum[TLB_index] = TLBPageNum[TLB_index - 1];
            TLBFrameNum[TLB_index] = TLBFrameNum[TLB_index - 1];
        }

        if(TLB_size <= 15)
		{
            TLB_size++;
        }

        TLBPageNum[0] = page_number;
        TLBFrameNum[0] = pageTable[page_number][0];
		
        
	}
   

	
}

    //report page-fault rate
    /* rate = (page faults / total addresses) * 100 */
    float page_fault_rate = (( (float)page_faults / (float)total_addr) * 100);
    printf("Page-fault rate: %.2f%%\n", page_fault_rate);
    //report TLB hit rate <----- do NOT expect to be high
    /* rate = (TLB hits / total addresses)* 100 */
    float TLB_hit_rate = (((float)TLB_hits / (float)total_addr) * 100);
    printf("TLB hit rate: %.2f%%\n", TLB_hit_rate);

    /* close files */
    fclose(addressFile);
    fclose(backingStore);
    fclose(output);

    return 0;
}
