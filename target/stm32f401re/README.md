# STM32F401RE integration adapter

This folder is source-level integration code for an STM32CubeIDE project. It is
not a generated CubeMX project and has not been compiled against a specific HAL
or FreeRTOS release in this repository.

## Expected CubeMX configuration

- Board/MCU: NUCLEO-F401RE / STM32F401RETx, 84 MHz system clock.
- I2C1: PB8 SCL and PB9 SDA, 400 kHz, external 4.7 kOhm pull-ups to 3.3 V.
- USART2: PA2 TX and PA3 RX, 115200 8-N-1.
- TIM2: update interrupt at exactly 100 Hz. Give it an interrupt priority that
  is numerically at or below `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` so
  calling `xSemaphoreGiveFromISR` is legal.
- IWDG: approximately 2 s timeout. Start it only after the task objects exist.
- FreeRTOS: 1 kHz tick, mutexes enabled, `configASSERT` enabled, and at least
  20 KiB heap for the example's conservative dynamic task stacks. Measure stack
  high-water marks on hardware before reducing them.
- Linker: include `-u _printf_float` when using newlib-nano; telemetry uses
  bounded `snprintf` float formatting.

Copy `Core/Inc/*` and `Core/Src/*` into the generated project, add this
repository's `include/` directory and portable `src/*.c` files to the build,
then call `HubRtos_Init()` after peripheral initialization and before the
scheduler starts. Forward the timer callback:

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    HubRtos_TimerPeriodElapsedFromISR(htim);
}
```

The names `hi2c1`, `huart2`, `hiwdg`, and `htim2` are deliberate integration
seams. Update the four `extern` declarations in `app_freertos.c` if the CubeMX
project uses different handles.

## Bring-up gates

1. Keep IWDG disabled and confirm WHO_AM_I (`0x68`) and chip ID (`0x58`) reads.
2. Verify TIM2 with a logic-analyzer GPIO toggle before enabling the task.
3. Confirm 10 Hz NDJSON on UART and no increasing queue/error counters.
4. Measure DWT maxima for at least 10 minutes; acquisition must remain below the
   10 ms sample period with margin.
5. Disconnect each sensor independently and verify `offline` after three failed
   acquisitions, continued telemetry, and watchdog feeding while tasks remain
   alive.
6. Suspend a task in a debug-only build and verify the health task withholds the
   watchdog refresh.

See the root README for wiring, fault tests, and the distinction between host
simulation evidence and physical-board validation still to be performed.
