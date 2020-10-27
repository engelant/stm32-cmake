/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.
 */
#include "FreeRTOS.h"
#include "task.h"
//#include "lwmem/lwmem.h"
#include "usbcdc.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/common.h>

void vApplicationMallocFailedHook( void ) {
	for(;;);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName ){
	for(;;);	// Loop forever here..
}

static void
task1(void *args __attribute((unused))) {

	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO7);

	volatile int foo = 0;
//	foo |= 1 <<4;

//	BBIO_SRAM(&foo, 2) = 1;
	for (;;) {
		gpio_toggle(GPIOB,GPIO7);
		foo++;
		vTaskDelay(1000);
		foo++;
	}

}

/*
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

static lwmem_region_t
regions[] = {
    { ucHeap, sizeof(ucHeap) }
};
*/

int
main(void) {

	rcc_clock_setup_hse(&rcc_3v3[RCC_CLOCK_3V3_216MHZ], 8);

	//lwmem_assignmem(regions, sizeof(regions) / sizeof(regions[0]));

	xTaskCreate(task1,"LED", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-1, NULL);
	//xTaskCreate(usb_task, "USB", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-1, NULL);
	vTaskStartScheduler();

	for (;;);
	return 0;
}

// End
