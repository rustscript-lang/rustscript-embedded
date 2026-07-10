#include <stdint.h>
#include <stddef.h>

#define UART0_BASE 0x20201000u
#define UART_DR    (*(volatile uint32_t *)(UART0_BASE + 0x00u))
#define UART_FR    (*(volatile uint32_t *)(UART0_BASE + 0x18u))
#define UART_IBRD  (*(volatile uint32_t *)(UART0_BASE + 0x24u))
#define UART_FBRD  (*(volatile uint32_t *)(UART0_BASE + 0x28u))
#define UART_LCRH  (*(volatile uint32_t *)(UART0_BASE + 0x2cu))
#define UART_CR    (*(volatile uint32_t *)(UART0_BASE + 0x30u))
#define UART_IMSC  (*(volatile uint32_t *)(UART0_BASE + 0x38u))
#define UART_ICR   (*(volatile uint32_t *)(UART0_BASE + 0x44u))

static void uart_init(void) {
    UART_CR = 0;
    UART_ICR = 0x7ff;
    UART_IBRD = 1;
    UART_FBRD = 40;
    UART_LCRH = (3u << 5);
    UART_IMSC = 0;
    UART_CR = (1u << 0) | (1u << 8) | (1u << 9);
}

static void uart_putc(char ch) {
    while ((UART_FR & (1u << 5)) != 0) {
    }
    UART_DR = (uint32_t)ch;
}

static void uart_puts(const char *s) {
    while (*s != '\0') {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}

typedef enum {
    OP_PUSH_I32,
    OP_PUSH_BOOL,
    OP_LOAD,
    OP_STORE,
    OP_ADD,
    OP_LT,
    OP_NOT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_PRINT_TEXT,
    OP_HALT,
} OpCode;

typedef struct {
    OpCode op;
    int32_t arg;
} Op;

enum {
    LOCAL_LED = 0,
    LOCAL_TICKS = 1,
    TEXT_LED_ON = 0,
    TEXT_LED_OFF = 1,
};

static const char *const TEXTS[] = {
    "led:on\n",
    "led:off\n",
};

/* Frozen form of programs/blinky.rss for Raspberry Pi Zero BCM2835 bare metal. */
static const Op PROGRAM[] = {
    {OP_PUSH_BOOL, 0}, {OP_STORE, LOCAL_LED},
    {OP_PUSH_I32, 0}, {OP_STORE, LOCAL_TICKS},
    {OP_LOAD, LOCAL_TICKS}, {OP_PUSH_I32, 4}, {OP_LT, 0}, {OP_JUMP_IF_FALSE, 16},
    {OP_LOAD, LOCAL_LED}, {OP_NOT, 0}, {OP_STORE, LOCAL_LED},
    {OP_LOAD, LOCAL_TICKS}, {OP_PUSH_I32, 1}, {OP_ADD, 0}, {OP_STORE, LOCAL_TICKS},
    {OP_JUMP, 4},
    {OP_LOAD, LOCAL_LED}, {OP_JUMP_IF_FALSE, 20},
    {OP_PRINT_TEXT, TEXT_LED_ON}, {OP_HALT, 0},
    {OP_PRINT_TEXT, TEXT_LED_OFF}, {OP_HALT, 0},
};

typedef struct {
    int32_t stack[16];
    size_t sp;
    int32_t locals[2];
} TinyVm;

static void push(TinyVm *vm, int32_t value) {
    vm->stack[vm->sp++] = value;
}

static int32_t pop(TinyVm *vm) {
    return vm->stack[--vm->sp];
}

static void run_frozen_program(void) {
    TinyVm vm;
    for (size_t i = 0; i < sizeof(vm.stack) / sizeof(vm.stack[0]); ++i) {
        vm.stack[i] = 0;
    }
    for (size_t i = 0; i < sizeof(vm.locals) / sizeof(vm.locals[0]); ++i) {
        vm.locals[i] = 0;
    }
    vm.sp = 0;
    size_t ip = 0;

    for (;;) {
        Op op = PROGRAM[ip++];
        switch (op.op) {
        case OP_PUSH_I32:
        case OP_PUSH_BOOL:
            push(&vm, op.arg);
            break;
        case OP_LOAD:
            push(&vm, vm.locals[op.arg]);
            break;
        case OP_STORE:
            vm.locals[op.arg] = pop(&vm);
            break;
        case OP_ADD: {
            int32_t rhs = pop(&vm);
            int32_t lhs = pop(&vm);
            push(&vm, lhs + rhs);
            break;
        }
        case OP_LT: {
            int32_t rhs = pop(&vm);
            int32_t lhs = pop(&vm);
            push(&vm, lhs < rhs);
            break;
        }
        case OP_NOT:
            push(&vm, !pop(&vm));
            break;
        case OP_JUMP:
            ip = (size_t)op.arg;
            break;
        case OP_JUMP_IF_FALSE:
            if (!pop(&vm)) {
                ip = (size_t)op.arg;
            }
            break;
        case OP_PRINT_TEXT:
            uart_puts(TEXTS[op.arg]);
            break;
        case OP_HALT:
            return;
        }
    }
}

void kernel_main(void) {
    uart_init();
    uart_puts("RustScript Pi Zero bare-metal demo\n");
    uart_puts("SoC: BCM2835, CPU: ARM1176JZF-S, OS: none\n");
    uart_puts("JIT: off, program: frozen blinky.rss\n");
    run_frozen_program();
    uart_puts("done\n");
}
