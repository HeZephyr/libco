#include "co.h"
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>

enum co_status {
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，还未释放资源
};

#define K 1024
#define STACK_SIZE (64 * K) // 栈大小

// 协程结构体
struct co{
    const char* name; 
    void (*func)(void *);   // co_start 指定的入口地址和参数
    void* (arg);

    enum co_status status;              // 协程的状态
    struct co*     waiter;              // 是否还有其他协程在等待当前协程
    jmp_buf        context;             // 寄存器现场(setjmp.h)
    unsigned char  stack[STACK_SIZE];   // 协程的堆栈
};

// 双向链表
typedef struct CONODE {
    struct co* coroutine;

    struct CONODE* prev;    // 前一个结点
    struct CONODE* next;    // 下一个结点
}CoNode;

static CoNode* co_node = NULL;

/*
 * 如果co_node == NULL，则创建一个新的双向循环链表即可，并返回
 * 如果co_node != NULL，则在co_node和co_node->prev之间插入，仍然返回co_node的值
 */
static void co_node_insert(struct co* coroutine) {
    CoNode* victim = (CoNode*)malloc(sizeof(CoNode));

    assert(victim);

    victim->coroutine = coroutine;
    if (co_node == NULL) {
        victim->prev = victim->next = victim;   // 循环
    } else {
        victim->next = co_node->next;
        victim->prev = co_node;
        victim->next->prev = victim;
        victim->prev->next = victim;
    }
}

/*
 * 如果当前只剩node一个，则返回这个
 * 否则，拉去当前co_node对应的协程，并沿着next方向移动
 */
static CoNode* co_node_remove() {
    CoNode* victim = NULL;
    if (co_node == NULL) {
        return NULL;
    } else if (co_node->next == co_node) {
        victim = co_node;
        co_node = NULL;
    } else {
        victim = co_node;   // 拿走当前协程
        
        co_node = co_node->prev;
        co_node->next = victim->next;
        co_node->next->prev = co_node;
    }
    return victim;
}

struct co* co_start(const char *name, void (*func)(void *), void *arg) {
    struct co* new_co = (struct co*)malloc(sizeof(struct co));  // 创建新协程
    
    assert(new_co); // 判断是否创建成功
    
    // 赋值
    new_co->name   = name;
    new_co->func   = func;
    new_co->arg    = arg;
    new_co->status = CO_NEW;
    new_co->waiter = NULL;

    co_node_insert(new_co);
    return new_co;
}

void co_wait(struct co *co) {
}

void co_yield() {
}
