// RPC.c - UltimateOS Self-Contained RPC with Memory-Region Lock/Unlock

typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned long u64;
typedef unsigned long uintptr_t;

// ------------------------
// RPC Function IDs
// ------------------------
typedef enum {
    RPC_COPY_MEMORY,
    RPC_GET_POINTER,
    RPC_LOCK_MEMORY,
    RPC_UNLOCK_MEMORY,
    RPC_INVOKE_PROCEDURE
} RPC_FunctionID;

// ------------------------
// Generic Request/Response
// ------------------------
typedef struct {
    RPC_FunctionID func_id;
    void* params;
    u32 param_size;
} RPC_Request;

typedef struct {
    u32 status;      // 0 = OK, non-zero = error
    void* result;
    u32 result_size;
} RPC_Response;

// ------------------------
// Parameter Structures
// ------------------------
typedef struct {
    u32 src_pid;
    void* src_addr;
    u32 dest_pid;
    void* dest_addr;
    u32 size;
} RPC_CopyMemoryParams;

typedef struct {
    u32 pid;
    void* base_addr;
    u32 offset;
} RPC_GetPointerParams;

typedef struct {
    u32 pid;
    void* addr;
    u32 size;
} RPC_LockUnlockMemoryParams;

typedef struct {
    u32 pid;
    void* func_ptr;
    void* arguments;
    u32 arg_size;
} RPC_InvokeProcedureParams;

// ------------------------
// Process Memory Lock Table
// ------------------------
#define MAX_LOCKS 128

typedef struct {
    void* addr;
    u32 size;
    u32 locked;   // 1 = locked, 0 = unlocked
} MemRegionLock;

typedef struct {
    u32 pid;
    MemRegionLock locks[MAX_LOCKS];
} ProcessLockTable;

// For simplicity, max 64 processes tracked
ProcessLockTable process_locks[64];
u32 num_processes = 0;

// ------------------------
// Helpers
// ------------------------
int find_process_index(u32 pid) {
    for(u32 i = 0; i < num_processes; i++) {
        if(process_locks[i].pid == pid) return i;
    }
    // new process entry
    if(num_processes < 64) {
        process_locks[num_processes].pid = pid;
        for(u32 j = 0; j < MAX_LOCKS; j++) process_locks[num_processes].locks[j].locked = 0;
        num_processes++;
        return num_processes - 1;
    }
    return -1; // too many processes
}

int lock_memory_region(u32 pid, void* addr, u32 size) {
    int idx = find_process_index(pid);
    if(idx < 0) return 0;

    ProcessLockTable* table = &process_locks[idx];

    // Find free lock slot
    for(u32 i = 0; i < MAX_LOCKS; i++) {
        if(table->locks[i].locked == 0) {
            table->locks[i].addr = addr;
            table->locks[i].size = size;
            table->locks[i].locked = 1;
            return 1; // success
        }
    }
    return 0; // no free slots
}

int unlock_memory_region(u32 pid, void* addr, u32 size) {
    int idx = find_process_index(pid);
    if(idx < 0) return 0;

    ProcessLockTable* table = &process_locks[idx];

    // Find the matching region
    for(u32 i = 0; i < MAX_LOCKS; i++) {
        if(table->locks[i].locked &&
           table->locks[i].addr == addr &&
           table->locks[i].size == size) {
            table->locks[i].locked = 0;
            return 1; // success
        }
    }
    return 0; // region not found
}

int validate_process(u32 pid) { return 1; } // assume all PIDs valid

void memcopy_process(u32 dest_pid, void* dest, u32 src_pid, void* src, u32 size) {
    u8* s = (u8*)src;
    u8* d = (u8*)dest;
    for(u32 i = 0; i < size; i++) d[i] = s[i];
}

void* resolve_pointer(u32 pid, void* base, u32 offset) {
    return (void*)((u8*)base + offset);
}

void* call_in_process(u32 pid, void* func_ptr, void* args, u32 arg_size) {
    // Placeholder for low-level call
    return 0;
}

// ------------------------
// Core RPC Functions
// ------------------------
RPC_Response rpc_copy_memory(RPC_CopyMemoryParams* p) {
    if(!validate_process(p->src_pid) || !validate_process(p->dest_pid))
        return (RPC_Response){1, 0, 0};
    memcopy_process(p->dest_pid, p->dest_addr, p->src_pid, p->src_addr, p->size);
    return (RPC_Response){0, 0, 0};
}

RPC_Response rpc_get_pointer(RPC_GetPointerParams* p) {
    void* ptr = resolve_pointer(p->pid, p->base_addr, p->offset);
    return (RPC_Response){0, ptr, sizeof(void*)};
}

RPC_Response rpc_lock_memory(RPC_LockUnlockMemoryParams* p) {
    if(!validate_process(p->pid)) return (RPC_Response){1, 0, 0};
    if(!lock_memory_region(p->pid, p->addr, p->size)) return (RPC_Response){1, 0, 0};
    return (RPC_Response){0, 0, 0};
}

RPC_Response rpc_unlock_memory(RPC_LockUnlockMemoryParams* p) {
    if(!validate_process(p->pid)) return (RPC_Response){1, 0, 0};
    if(!unlock_memory_region(p->pid, p->addr, p->size)) return (RPC_Response){1, 0, 0};
    return (RPC_Response){0, 0, 0};
}

RPC_Response rpc_invoke_procedure(RPC_InvokeProcedureParams* p) {
    void* result = call_in_process(p->pid, p->func_ptr, p->arguments, p->arg_size);
    return (RPC_Response){0, result, sizeof(void*)};
}

// ------------------------
// RPC Dispatcher
// ------------------------
RPC_Response handle_rpc(RPC_Request* req) {
    switch(req->func_id) {
        case RPC_COPY_MEMORY:
            return rpc_copy_memory((RPC_CopyMemoryParams*)req->params);
        case RPC_GET_POINTER:
            return rpc_get_pointer((RPC_GetPointerParams*)req->params);
        case RPC_LOCK_MEMORY:
            return rpc_lock_memory((RPC_LockUnlockMemoryParams*)req->params);
        case RPC_UNLOCK_MEMORY:
            return rpc_unlock_memory((RPC_LockUnlockMemoryParams*)req->params);
        case RPC_INVOKE_PROCEDURE:
            return rpc_invoke_procedure((RPC_InvokeProcedureParams*)req->params);
        default:
            return (RPC_Response){1, 0, 0};
    }
}

