#include <stdlib.h>

enum {
	SORT_SUCCESS = 0,
	FUNC_ARG_E = 1,
	MEM_ALLOC_E,
};

int sort_errno;
const char* get_sort_error_message(void)
{
	int err_code = sort_errno;
	sort_errno = SORT_SUCCESS;
	switch (err_code) {
	case SORT_SUCCESS: return "sort success";
	case FUNC_ARG_E: return "Function argument fault";
	case MEM_ALLOC_E: return "Memory allocate fault";
	default: return "Unknown error";
	}
}


/*
* 插入排序：
*	for j = 2 to A.length
*		key = A[j]
*		i = j - 1
*		while i > 0 and A[i] > key
*			A[i+1] = A[i]
*			i = i - 1
*		A[i + 1] = key
* 时间复杂度： θ(n^2) < 最坏情况
*			   θ(n)  < 最好情况
* 
* 返回值：0, 成功；其他表示失败码
*/
int insert_sort(int* data, int cnt)
{
	int i, j, key;

	if (!data || cnt <= 0) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	for (j = 1; j < cnt; j++) {
		i = j - 1;
		key = data[j];
		while (i >= 0 && data[i] > key) {
			data[i + 1] = data[i];
			i--;
		}
		data[i + 1] = key;
	}
	return 0;
}


/*
* 选择排序
*	for i = 1 to A.length - 1
*		min = i
*		for j = i + 1 to A.length
*			if A[j] < A[min]
*				min = j
*		tmp = A[i]
*		A[i] = A[min]
*		A[min] = tmp
* 时间复杂度：θ(n^2) <- 最好情况，最坏情况
* 
* 返回值：0, 成功；其他表示失败码
*/
int select_sort(int* data, int cnt)
{
	int min, i, j, tmp;

	if (!data || cnt <= 0) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	for (i = 0; i < cnt - 1; i++) {
		min = i;
		for (j = i + 1; j < cnt; j++)
			if (data[j] < data[min])
				min = j;
		tmp = data[i];
		data[i] = data[min];
		data[min] = tmp;
	}

	return 0;
}

/*
* 分治策略：二分法排序
* 合并步骤：
*	merge(A, p, q, r)
*		n1 = q - p +1;	// 包括q
*		n2 = r - q;		// 不包括q
*		let L[1..n1] and R[1..n2] be new arrays
*		for i = 1 to n1
*			L[i] = A[p+i-1]
*		for j = 1 to n2
*			R[j] = A[q + j]
*		i = j = 1
*		for k = p to k
*			if i > n1
*				A[k] = R[j++]
*			else if j > n2
*				A[k] = L[i++]
*			else if L[i] <= R[j]
*				A[k] = L[i++]
*			else
*				A[k] = R[j++]
* 
* 完整排序：
*	merge_sort(A, p, r)
*		if p < r
*			q = (p + r) / 2
*			merge_sort(A, p, q)
*			merge_sort(A, q + 1, r)
*			merge(A, p, q, r)
* 
* 时间复杂度：θ(nlgn)
* 
* 返回值：0，成功；其他表示错误码
*/
static int merge(int* data, int p, int q, int r)
{
	int n1, n2, i, j, k;
	int *L, *R;
	int ret = 0;

	if (!data || p < 0 || q < p || r < q) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	n1 = q - p + 1;
	n2 = r - q;
	L = (int *)calloc(n1, sizeof(int));
	if (!L) {
		sort_errno = MEM_ALLOC_E;
		ret = -1;
		goto out;
	}
	R = (int *)calloc(n2, sizeof(int));
	if (!R) {
		sort_errno = MEM_ALLOC_E;
		ret = -1;
		goto out;
	}
	for (i = 0; i < n1; i++)
		L[i] = data[p + i];
	for (j = 0; j < n2; j++)
		R[j] = data[q + j + 1];

	i = j = 0;
	for (k = p; k <= r; k++) {
		if (i >= n1)
			data[k] = R[j++];
		else if (j >= n2)
			data[k] = L[i++];
		else if (L[i] > R[j])
			data[k] = R[j++];
		else
			data[k] = L[i++];
	}

out:
	if (L)
		free(L);
	if (R)
		free(R);
	return ret;
}
int merge_sort(int* data, int p, int r)
{
	int q;

	if (!data || p < 0 || r < p) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	if (p < r) {
		q = (p + r) / 2;
		merge_sort(data, p, q);
		merge_sort(data, q + 1, r);
		return merge(data, p, q, r);
	}
	return 0;
}

/*
* 堆排序：
*	堆是一个数组，它可以被看成一个近似的完全二叉树。
*	表示堆的数组A有两个属性：
*		A.length给出数组元素的个数，
*		A.heap-size表示有多少个堆元素存储在该数组中。
*		0 <= A.heap-size <= A.length
*
*	如果树的根结点下标为1，则有：
*		PARENT(i) = i / 2
*		LEFT(i) = 2 * i
*		RIGHT(i) = 2 * i + 1
*	如果树的根结点下标为0，则有：
*		PARENT(i) = (i-1) / 2
*		LEFT(i) = 2 * i + 1
*		RIGHT(i) = 2 * i + 2
*
*	最大堆：除根结点外满足A[PARENT(i)] >= A[i]
*	最小堆：除根结点外满足A[PARENT(i)] <= A[i]
*
*	维护堆的性质：下标从0开始
*		max_heapify(A, i) 前提：A[i+1, ...]已经是一个大堆
*			l = LEFT(i)
*			r = RIGHT(i)
*			largest = i
*			if l < A.heap_size and A[l] > A[i]
*				largest = l
*			if r < A.heap_size and A[r] > A[i]
*				largest = r
*			if largest != i
*				exchange A[i] with A[largest]
*				max_heapify(A, largest)
*		时间复杂度：θ(lgn)
*
*	建堆：下标从0开始
*		build_max_heap(A)
*			A.heap_size = A.length
*			for i = A.length / 2 - 1 downto 0
*				max_heapify(A, i)
*		时间复杂度：θ(n)
*
*	堆排序：下标从0开始
*		heap_sort(A)
*			build_max_heap(A)
*			for i = A.length - 1 downto 0
*				exchange A[1] with A[i]
*				A.heap_size--;
*				max_heapify(A, 0)
*
*	堆的其他操作：
*		heap_maximum:	返回最大值
*			return A[0]
*		heap_extract_max: 返回最大值，并删除最大值
*			max = A[0]
*			A[0] = A[A.heap_size - 1]
*			A.heap_size--
*			max_heapify(A, 0)
*		heap_increase_key(A, i, key): 将A[i]增大至key
*			A[i] = key
*			while i >= 0 and A[PARENT(i)] < A[i]
*				exchange A[i] and A[PARENT(i)]
*				i = PARENT(i)
*/
#define PARENT(i) ((i-1)/2)
#define LEFT(i) (2*i+1)
#define RIGHT(i) (2*i+2)

typedef struct heap_s {
	int* h_data;
	int h_length;
	int h_size;
} heap_t;

int max_heapify(heap_t* heap, int i)
{
	int l = LEFT(i);
	int r = RIGHT(i);
	int largest = i;
	int* data = heap->h_data;
	
	if (!heap || !heap->h_data || i < 0) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	if (l < heap->h_size && data[l] > data[largest])
		largest = l;
	if (r < heap->h_size && data[r] > data[largest])
		largest = r;
	if (largest != i) {
		int tmp = data[largest];
		data[largest] = data[i];
		data[i] = tmp;
		return max_heapify(heap, largest);
	}
	return 0;
}

int build_max_heap(heap_t* heap)
{
	int i, ret;

	if (!heap || !heap->h_data || heap->h_length <= 0) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	heap->h_size = heap->h_length;
	for (i = heap->h_length / 2 - 1; i >= 0; i--) {
		ret = max_heapify(heap, i);
		if (ret != 0)
			return ret;
	}
	return 0;
}

heap_t* heap_new(int* data, int cnt)
{
	int i;
	heap_t* new_heap;

	if (!data || cnt <= 0) {
		sort_errno = FUNC_ARG_E;
		return NULL;
	}

	new_heap = (heap_t*)malloc(sizeof(heap_t));
	if (!new_heap) {
		sort_errno = MEM_ALLOC_E;
		return NULL;
	}

	new_heap->h_data = data;
	new_heap->h_length = cnt;

	build_max_heap(new_heap);

	return new_heap;
}

void heap_free(heap_t* heap)
{
	if (!heap)
		return;

	if (heap->h_data)
		free(heap->h_data);

	free(heap);
}

int heap_sort(int* data, int cnt)
{
	heap_t* heap;
	int i, tmp;

	if (!data || cnt <= 0) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	heap = heap_new(data, cnt);
	if (!heap)
		return -1;
	for(i = heap->h_size - 1; i > 0; i--) {
		tmp = heap->h_data[i];
		heap->h_data[i] = heap->h_data[0];
		heap->h_data[0] = tmp;
		heap->h_size--;
		max_heapify(heap, 0);
	}
	
	return 0;
}


/*
* 快速排序：
*	分解：	数组A[p..r]被划分为两个（可能为空）子数组A[p..q-1]和A[q+1..r]，
*			使得A[p..q-1]中的每一个元素都小于等于A[q]，而A[q]也小于等于A[q+1..r]
*			中的每个元素，其中计算下标q也是划分过程的一部分
*	解决：	通过递归调用快速排序，对子数组A[p..q-1]和A[q+1..r]进行排序
*	合并：	因为子数组都是原址排序的，所以不需要合并操作，数组A[p..r]已经有序
* 
*	伪代码：
*		quick_sort(A, p, r):
*			if p < r
*				q = partition(A, p, r)
*				quick_sort(A, p, q - 1)
*				quick_sort(A, q + 1, r)
* 
*		partition(A, p, r):
*			x = A[r]
*			i = p - 1
*			for j = p to r - 1
*				if A[j] <= x
*					i++
*					exchange A[i] with A[j]
*			exchange A[i + 1] with A[r]
*			return i + 1
* 
* 性能：
*	最好情况：θ(nlgn)
*	最好情况：θ(n^2)
*	期望情况：θ(nlgn)
*/
int partition(int* data, int p, int r)
{
	int x = data[r];
	int tmp, j, i = p - 1;

	for (j = p; j <= r - 1; j++) {
		if (data[j] <= x) {
			tmp = data[++i];
			data[i] = data[j];
			data[j] = tmp;
		}
	}
	tmp = data[++i];
	data[i] = data[r];
	data[r] = tmp;

	return i;
}
int quick_sort(int* data, int p, int r)
{
	int q;

	if (!data || p < 0 || r < p) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}
	if (p < r) {
		q = partition(data, p, r);
		quick_sort(data, p, q - 1);
		quick_sort(data, q + 1, r);
	}

	return 0;
}


/*
* 计数排序：只能用于排序元素在[0, .. k]之间的整数数组，
			如最小值小于0，则应该把先把数组的所有元素加上最小值的绝对值，使其范围在[0,..k]之间。
*			对于每一个输入元素x，确定小于x的元素的个数。利用这一信息，就可以直接把x放到它在输出数组中的位置上。
* 
* 伪代码：
*	counting_sort(A, B, k): A为输入数组，B为输出数组， k为数组中最大元素的值
*		let C[0..k] to be a new array
*		for i = 0 to k
*			C[i] = 0
*		for j = 1 to A.length
*			C[A[j]]++
*		for i = 1 to k
*			C[i] += C[i - 1]
*		for j = A.length downto 1
*			B[C[A[j]]] = A[j]
*			C[A[j]]--
* 
* 性能：θ(n)
*/
int counting_sort(int* in, int* out, int max, int cnt)
{
	int* C;
	int i;

	if (!in || !out || max < 0 || cnt < 1) {
		sort_errno = FUNC_ARG_E;
		return -1;
	}

	C = (int*)calloc(max + 1, sizeof(int));
	if (!C) {
		sort_errno = MEM_ALLOC_E;
		return -1;
	}
//	for (i = 0; i <= max; i++)
//		C[i] = 0;
	for (i = 0; i < cnt; i++)
		C[in[i]]++;
	for (i = 1; i <= max; i++)
		C[i] += C[i - 1];
	for (i = cnt - 1; i >= 0; i--) {
		out[C[in[i]] - 1] = in[i];
		C[in[i]]--;
	}

	free(C);

	return 0;
}

/*
* 基数排序：
*	将数组元素分成若干组，对每组进行排序，直到最后一组排序完后，整个数组也排序
* 
* 如何根据某一组来交换整个元素呢？
* typedef struct element_s {
*	void *data;
*	int d;	// d 字节为单位的宽度
* } element_t;
*/
typedef struct element_s {
	void* data;
	int d;
} element_t;

void comp_swap(element_t* e1, element_t* e2, int idx)
{
	
}


/*
* 桶排序：
*	确定桶的个数：总共cnt个数，如果桶的个数为10，则令d=(cnt/10) + 1，ele/d = ele / cnt * 10为ele所属桶的序号，例如：
*		cnt = 23, ele = [0..22], d = 3, 桶的序号为:0..9, ele的序号为[0..22]/3 = [0..7], 此时还多出两个桶了。
*		cnt = 29，ele = [0..28], d = 3, 桶的序号为:0..9, ele的序号为[0..29]/3 = [0..9], 此时刚好
*	桶中元素个数不能太大，因为元素个数不固定，所以桶的个数不能固定。
*	假设每个桶中元素个数平均为10个，则桶的个数为cnt/10 + 1，
*/

