/*
* created by wushengbang at 2023/7/19 15:01
* 
* 此程序用于测试各种排序算法，并比较各种算法之前的性能
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>

#include "sort.h"

#define CNT 10000000	// 定义数组长度

static int debug;
static int cnt;

void help(const char *appname)
{
	printf("%s [-c number] -d\n", appname);
	printf("\t-c\tspecific the number of data sorted\n");
	printf("\t-d\tdebug, print the data value before and after sorting\n");
	exit(EXIT_SUCCESS);
}

void parse_arg(int argc, char** argv)
{
	int c, option_index;
	struct option long_opt[] = {
		{"debug", no_argument, 0, 1},
		{"cnt", required_argument, 0, 2 }
	};

	while (1) {
		option_index = 0;
		c = getopt_long(argc, argv, "dc:", long_opt, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'd':
		case 1: debug = 1; break;
		case 'c':
		case 2: cnt = atoi(optarg); break;
		default:
			help(argv[0]);
		}
	}
}

void disp(int* data, int c)
{
	int i;

	for (i = 0; i < c; i++) {
		if (i > 0 && i % 16 == 0) printf("\n");
		printf("%d ", data[i]);
	}
	printf("\n");

}

void disp_tv(struct timeval* tv1, struct timeval* tv2)
{
	long usec_delta = (long)(tv2->tv_usec - tv1->tv_usec);
	long sec_delta = (long)(tv2->tv_sec - tv1->tv_sec);
	if (usec_delta < 0) {
		usec_delta += 1000000;
		sec_delta--;
	}
	if (sec_delta > 3600 * 24) {
		printf("%ld 天 ", sec_delta / (3600 * 24));
		sec_delta %= (3600 * 24);
	}
	if (sec_delta > 3600) {
		printf("%ld 小时 ", sec_delta / 3600);
		sec_delta %= 3600;
	}
	if (sec_delta > 60) {
		printf("%ld 分钟 ", sec_delta / 60);
		sec_delta %= 60;
	}
	printf("%ld 秒 %ld 微秒\n", sec_delta, usec_delta);
}

int main(int argc, char **argv)
{
	int* data, *data1;
	int i;
	struct timeval tv1, tv2;

	parse_arg(argc, argv);
	if (cnt <= 0)
		cnt = CNT;

	data = (int*)calloc(cnt, sizeof(int));
	if (!data) {
		perror("calloc");
		return EXIT_FAILURE;
	}

	srand((unsigned int)time(NULL));
	for (i = 0; i < cnt; i++) 
		data[i] = rand() % cnt;
	
	if (debug)
		disp(data, cnt);
	
	data1 = (int*)calloc(cnt, sizeof(int));
	if (!data1) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}

	printf("各种排序算法对%d个随机整数进行排序所花费的时间如下（1秒 == 1000000微秒）\n", cnt);
	
	
	printf("插入排序耗时：\t");
	memcpy(data1, data, cnt * sizeof(int));
	gettimeofday(&tv1, NULL);
	insert_sort(data1, cnt);
	gettimeofday(&tv2, NULL);
	disp_tv(&tv1, &tv2);
	if (debug)
		disp(data1, cnt);

	printf("选择排序耗时：\t");
	memcpy(data1, data, cnt * sizeof(int));
	gettimeofday(&tv1, NULL);
	select_sort(data1, cnt);
	gettimeofday(&tv2, NULL);
	disp_tv(&tv1, &tv2);
	if (debug)
		disp(data1, cnt);
	

	printf("归并排序耗时：\t");
	memcpy(data1, data, cnt * sizeof(int));
	gettimeofday(&tv1, NULL);
	merge_sort(data1, 0, cnt - 1);
	gettimeofday(&tv2, NULL);
	disp_tv(&tv1, &tv2);
	if (debug)
		disp(data1, cnt);

	printf("堆排序耗时：\t");
	memcpy(data1, data, cnt * sizeof(int));
	gettimeofday(&tv1, NULL);
	heap_sort(data1, cnt);
	gettimeofday(&tv2, NULL);
	disp_tv(&tv1, &tv2);
	if (debug)
		disp(data1, cnt);

	printf("快速排序耗时：\t");
	memcpy(data1, data, cnt * sizeof(int));
	gettimeofday(&tv1, NULL);
	quick_sort(data1, 0, cnt - 1);
	gettimeofday(&tv2, NULL);
	disp_tv(&tv1, &tv2);
	if (debug)
		disp(data1, cnt);

	printf("计数排序耗时：\t");
//	memcpy(data1, data, cnt * sizeof(int));
	gettimeofday(&tv1, NULL);
	counting_sort(data, data1,  cnt - 1, cnt);
	gettimeofday(&tv2, NULL);
	disp_tv(&tv1, &tv2);
	if (debug)
		disp(data1, cnt);


	free(data);
	free(data1);

	return EXIT_SUCCESS;
}
