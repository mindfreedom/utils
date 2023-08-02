#ifndef __MY_SORT__
#define __MY_SORT__

#ifdef __cplusplus
extern "C" {
#endif

	const char* get_sort_error_message(int ret);

	int insert_sort(int* data, int cnt);

	int  select_sort(int* data, int cnt);
	
	int merge_sort(int* data, int p, int r);
	
	int heap_sort(int* data, int cnt);

	int quick_sort(int* data, int p, int r);

	int counting_sort(int* in, int* out, int max, int cnt);


#ifdef __cplusplus
}
#endif

#endif
