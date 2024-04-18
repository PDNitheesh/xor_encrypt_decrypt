/*****************************

Compile using 
gcc -O2  -o out xor_encrypt_decrypt.c

*****************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

char *filename = "xor_encrypt_decrypt.c";
char *key = "123456";
#define SIZE  80
typedef struct msg {
	char BUFF[4096];
	int len;
} msg;

msg MSG[SIZE];
int tail = 0;
static int msg_index = 0;
pthread_mutex_t lock;

void msg_push(char *message, int len)
{
	if(len > 4096) len = 4096;
	if(len > 0){
		if(msg_index == SIZE) msg_index = 0;
		pthread_mutex_lock(&lock);
		strncpy(MSG[msg_index].BUFF, message, len);
		MSG[msg_index].len = len;
		msg_index++;
		tail++;
		pthread_mutex_unlock(&lock);
	}
}

void *tprint(void *args)
{
	static int index  = 0;
	
	//Allow buffer to be filled
	sleep(1);
	
	while(tail > 0){
		usleep(100000);
		if(index == SIZE) break;
		pthread_mutex_lock(&lock);
		write(1, MSG[index].BUFF, MSG[index].len);
		index++;
		tail--;
		pthread_mutex_unlock(&lock);
	}

}

int xor(const char *buff, int len, char *data)
{
	int count = 0;
	//zero padding
	int ksize = sizeof(key);
	len = len + (ksize - (len % ksize));
	for(count = 0; len; len = len -ksize){
		for(int j = 0; j < ksize; j++, count++){
			data[count] = buff[count] ^ key[j];
		}
	}
	
	return count;
}

int main()
{
	int fp = -1;
	char buff[4096] = {0};
	char data[4096] = {0};
	pthread_t thread_id;
	int len = 0;
	int flag = 1;
	
	fp = open(filename, O_RDONLY);
	if(fp == -1) goto end;
	
	
	pthread_mutex_init(&lock, NULL);
	if(pthread_create(&thread_id, NULL, tprint, NULL) != 0 ) goto end;
	
	while(flag) {
		memset(buff, 0, 4096);
		memset(data, 0, 4096);
		len = read(fp, buff, 4096);
		flag = (len < 4096) ? 0:1;
		if(len == -1) break;
		//encrypt
		len = xor(buff, len, data);
		//decrypt
		len = xor(data, len, buff);
		msg_push(buff, len);
	}
	
	pthread_join(thread_id, NULL);

end: if(fp == -1) close(fp);


}