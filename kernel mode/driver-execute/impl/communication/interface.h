
#define DEVICE_NAME L"\\Device\\{50c8c082-d595-4c65-b72b-c1ad4fb520fd}"
#define DOS_NAME L"\\DosDevices\\{50c8c082-d595-4c65-b72b-c1ad4fb520fd}"

typedef enum _requests
{
	invoke_start,
	invoke_base,
	invoke_read,
	invoke_write,
	invoke_success,
	invoke_unique,
	invoke_translate,
	invoke_read_kernel,
	invoke_dtb,
	invoke_protect_virtual

}requests, * prequests;

typedef struct _read_invoke {
	uint32_t pid;
	uintptr_t address;
	uintptr_t dtb;
	void* buffer;
	size_t size;
} read_invoke, * pread_invoke;

typedef struct _write_invoke {
	uint32_t pid;
	uintptr_t address;
	void* buffer;
	size_t size;
} write_invoke, * pwrite_invoke;

typedef struct _base_invoke {
	uint32_t pid;
	uintptr_t handle;
	const char* name;
	size_t size;
} base_invoke, * pbase_invoke;

typedef struct _allocate_invoke {
	uintptr_t address;
	SIZE_T dwSize;
	DWORD flAllocationType;
} allocate_invoke, * palloc_invoke;

typedef struct _read_kernel_invoke {
	uintptr_t address;
	void* buffer;
	size_t size;
	uint32_t memory_type;
} read_kernel_invoke, * pread_kernel_invoke;

typedef struct _translate_invoke {
	uintptr_t virtual_address;
	uintptr_t directory_base;
	void* physical_address;
} translate_invoke, * ptranslate_invoke;

typedef struct _dtb_invoke {
	uint32_t pid;
	uintptr_t dtb;
} dtb_invoke, * pdtb_invoke;

typedef struct _invoke_data
{
	uint32_t unique;
	requests code;
	void* data;
}invoke_data, * pinvoke_data;