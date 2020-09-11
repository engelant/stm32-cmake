/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "lwmem/lwmem.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/common.h>

extern void vApplicationStackOverflowHook(
	xTaskHandle *pxTask,
	signed portCHAR *pcTaskName);

void
vApplicationStackOverflowHook(
  xTaskHandle *pxTask __attribute((unused)),
  signed portCHAR *pcTaskName __attribute((unused))
) {
	for(;;);	// Loop forever here..
}

static void
task1(void *args __attribute((unused))) {

	volatile int foo = 0;
	foo |= 1 <<4;

	BBIO_SRAM(&foo, 2) = 1;
	for (;;) {
		gpio_toggle(GPIOC,GPIO13);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}


uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

static lwmem_region_t
regions[] = {
    { ucHeap, sizeof(ucHeap) }
};

int
main(void) {

	rcc_clock_setup_in_hse_8mhz_out_72mhz(); // For "blue pill"

	lwmem_assignmem(regions, sizeof(regions) / sizeof(regions[0]));

	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(
		GPIOC,
		GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL,
		GPIO13);

	xTaskCreate(task1,"LED",100,NULL,configMAX_PRIORITIES-1,NULL);
	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
