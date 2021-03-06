/*
 * dev_tools.c
 *
 *  Created on: Sep 23, 2013
 *      Author: rasmus
 */
#include "dev_tools.h"

char unload_8_bit(char* data, int *i)
{
	char b = data[*i] & 0xFF;
	*i += 1;
	return b;
}

long unload_16_bit(char* data, int *i, int swap)
{
	long h, rval;
	long l;
	if(swap) {
		l = *(data + *i) & 0x00FF;
		h = *(data + *i + 1) << 8;
	}
	else {
		l = *(data + *i + 1) & 0x00FF;
		h = *(data + *i) << 8;
	}
	*i += 2;
	rval = (h + l) & 0xFFFF;
	return rval;
}

char *unload_mac_addr(char* data, int *i)
{
	int j;
	char *mac;
	mac = (char*)malloc(6);

	for(j=5; j>=0; j--) {
		mac[j] = data[*i++];
	}

	return mac;
}

char compareMAC(char* MAC1, char* MAC2) 
{ 
	int i; 
	for(i=0; i<6; i++) { 
		if( *MAC1++ != *MAC2++ ) return 0; 
	} 
	return 1; 
} 

BLE_Peripheral_t* findDeviceByMAC(BLE_Central_t *central, char *MAC) 
{ 
	int i; 
	for(i=0; i<MAX_PERIPHERAL_DEV; i++) { 
		if(central->devices[i]._defined) { 
			if(compareMAC(central->devices[i].connMAC, MAC)) { 
				return &(central->devices[i]); 
			} 
		} 
	} 
	return NULL; 
} 

BLE_Peripheral_t* findDeviceByConnHandle(BLE_Central_t *central, long connHandle) 
{ 
	int i; 
	for(i=0; i<MAX_PERIPHERAL_DEV; i++) { 
		if(central->devices[i]._defined) { 
			if(central->devices[i].connHandle == connHandle) { 
				return &(central->devices[i]); 
			} 
		} 
	} 
	return NULL; 
} 

BLE_Peripheral_t* getNextAvailableDevice(BLE_Central_t *central, char *MAC) 
{
	BLE_Peripheral_t* device = findDeviceByMAC(central, MAC); 
	if (device != NULL) return device; 
 
	int i;       
	for(i=0; i<MAX_PERIPHERAL_DEV; i++) { 
		if(central->devices[i]._defined == 0) { 
			return &(central->devices[i]); 
		} 
	} 
	return NULL; 
}


void format_time_of_day(char* str, struct timeval tv)
{
	char tmbuf[100];
	struct tm *tmp;
	
	time_t t = (time_t)tv.tv_sec;
	tmp = localtime(&t);
	if(tmp == NULL) {
		sprintf(str, "ERROR: localtime\n");
		return;
	}
	strftime(tmbuf, sizeof(tmbuf),  "%Y-%m-%d %H:%M:%S", tmp);
	sprintf(str, "%s.%06d", tmbuf, tv.tv_usec);
}

int check_size_available(char *file_memory, int size) {
	int mem_ptr = 0;

	/* Place write lock on file. */
	//printf("Check size at: %X\n", file_memory);

	mem_ptr = *file_memory;
	mem_ptr = (mem_ptr<<8) + *(file_memory+1);

	if((mem_ptr+size+2) > FILE_LENGTH) {
		memset(file_memory,'\0', FILE_LENGTH);		
	}

	/* Release the lock. */
	
	return mem_ptr;
}

void append_mm_XMLfile(int runtime_count,char *content, char *file_memory) {
	int strlen = 0;	
	int mem_ptr = 0;
	
	//if((fd = lock_file(file)) < 0) {
	//	printf("File locked.\n");
	//	return 0;
	//}

	check_size_available(file_memory, 550);

	mem_ptr = *file_memory;
	mem_ptr = (mem_ptr<<8) + *(file_memory+1);

	//if((mem_ptr+MAX_STREAM_SIZE) < FILE_LENGTH+2) {
		strlen = sprintf((char*)(file_memory+2+mem_ptr),"\n<ble id=%d>\n\
<stw>\n\
<milliseconds>%s</milliseconds>\n\
</stw>\n\
</ble>\n",runtime_count,content);

	*file_memory = (char)(((strlen+mem_ptr) & 0xFF00)>>8);
	*(file_memory+1) = (char)((strlen+mem_ptr) & 0xFF);

	//unlock_file(fd);
}

/* 
 * Prepare a memory mapped file
 */
int preparemappedMem(char *file) {
	FILE *fd;
	char *file_memory;

	/* Prepare a file large enough.*/
	if((fd = open (file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
		fprintf(stderr, "Unable to open file: %s\n",file);
		return -1;	
	}

	/* Initialize the flock structure. */

	/* go to the location corresponding to the last byte */
	if (lseek (fd, FILE_LENGTH-1, SEEK_SET) == -1) {
		fprintf(stderr, "lseek error");
		return -1;
	}

	/* write a dummy byte at the last location */
	if (write (fd, "", 1) != 1) {
		fprintf(stderr, "write error");
		return -1;
	}

	//Go to the start of the file again
	lseek (fd, 0, SEEK_SET);

	file_memory = (char*)mmap (0, FILE_LENGTH, PROT_WRITE, MAP_SHARED, fd, 0);

	if(file_memory == ((caddr_t) -1))
      	{
        	fprintf(stderr, "%s: mmap error for file %s \n",strerror(errno),file);
		return -1;
        }

	memset(file_memory,0,FILE_LENGTH);

	printf("%s daemon: Buffer memory mapped at %02X [%d Bytes]\n",file ,file_memory, FILE_LENGTH);
	
	/* Release the lock. */

	close(fd);
	
	return (int)file_memory;		
}








