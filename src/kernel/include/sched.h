#ifndef _SCHED_H
#define _SCHED_H

#define THREAD_CPU_CONTEXT 0

/*
 * PSR bits
 */
#define PSR_MODE_EL0t	0x00000000
#define PSR_MODE_EL1t	0x00000004
#define PSR_MODE_EL1h	0x00000005
#define PSR_MODE_EL2t	0x00000008
#define PSR_MODE_EL2h	0x00000009
#define PSR_MODE_EL3t	0x0000000c
#define PSR_MODE_EL3h	0x0000000d






#ifndef __ASSEMBLER__

struct pt_regs {
	unsigned long regs[31];
	unsigned long sp;
	unsigned long pc;
	unsigned long pstate;
};



#define THREAD_SIZE 4096

#define NR_TASKS    16
#define NR_ISR		126
enum TASK_STATE {
    TASK_RUNNING,
	TASK_ZOMBIE,
	TASK_WAITING
};

extern struct task_struct *current;
extern struct task_struct ** task;
extern int nr_tasks;

struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

#define MAX_PROCESS_PAGES			48	// 128 kB process

struct user_page {
	unsigned long phys_addr;
	unsigned long virt_addr;
};

struct mm_struct {
	unsigned long pgd;
	int user_pages_count;
	struct user_page user_pages[MAX_PROCESS_PAGES];
	int kernel_pages_count;
	unsigned long kernel_pages[MAX_PROCESS_PAGES];
};

#define PF_KTHREAD		            	0x00000002	

struct task_struct {
	struct cpu_context cpu_context;
	enum TASK_STATE state;	
	long counter;
	long priority;
	long id;
	long preempt_count; // if > 0, then critical section
	unsigned long stack;
	unsigned long flags;
	struct mm_struct mm;
};

struct isr_wait_struct {
	unsigned int ticks_to_wait;
	struct task_struct* task_ptr;
	struct isr_wait_struct* next;
};


extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void switch_to(struct task_struct* next);
extern void cpu_switch_to(struct task_struct* prev, struct task_struct* next);
int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg, unsigned long stack);
int move_to_user_mode(unsigned long start, unsigned long size,unsigned long pc);
struct pt_regs * task_pt_regs(struct task_struct *tsk);
void exit_process();
void set_task_prio(int prio);

void delay_ticks(long ticks);
void handle_isr_wake_up(int isr_num);
void register_for_isr(int isr_num);
unsigned long get_ticks_since_start();

#define INIT_TASK { \
    /* cpu_context */ { 0,0,0,0,0,0,0,0,0,0,0,0,0 }, \
    /* state */       TASK_RUNNING, \
    /* counter */     0, \
    /* priority */    15, \
	/* process id*/   0, \
    /* preempt_count */ 0, \
    /* stack */       0, \
    /* flags */       PF_KTHREAD, \
    /* mm */          { 0, 0, {{0}}, 0, {0} } \
}

#endif
#endif