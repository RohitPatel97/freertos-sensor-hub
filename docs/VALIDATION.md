# Validation record and hardware checklist

This document separates reproducible repository checks from work that requires
the named hardware. Do not mark a hardware item complete without attaching the
artifact (build log, capture, or test note) to a release or pull request.

## Automated host checks

Recorded September 4, 2026 on Windows: fresh Zig 0.16.0 / Clang 21.1.0
compilation passed with strict C99 warnings. All 5 C groups, 11 simulator
scenarios, 69 CLI rejection cases, and 5 Python tool tests passed. Sample
dashboard replay also succeeded. These direct native builds use current source;
old generated executables are not publication artifacts. Physical target
validation remains pending below.

| Check | Command / CI test | Pass criterion |
|---|---|---|
| Strict C build | `cmake --build build` | GCC/Clang completes with `-Wall -Wextra -Wpedantic -Werror` |
| Portable unit tests | `core_unit_tests` | five groups cover retry/mutex accounting, BMP280 vector, conversion/formatting, sensor transitions/saturation, and heartbeat deadlines across clock wrap |
| Nominal schedule | `simulator_integration_tests` | 200 samples and 20 frames in 2 s; no errors/drops; two watchdog feeds |
| Bus/sensor fault | same integration test | BMP goes offline, MPU remains online, retry/error/disconnect counters rise |
| Queue pressure | same integration test | raw queue drops are nonzero and simulation remains bounded |
| Task stall | same integration test | watchdog refresh is withheld and deadline misses rise |
| Recovery | same integration test | MPU, BMP, both-device, and startup-absence payloads recover; one-cycle faults do not flap status |
| Invalid CLI inputs | same integration test | 69 malformed and out-of-range inputs exit 2 promptly without telemetry |
| Python tooling | `python -m unittest ...` | parser, noise handling, faults, and renderer pass |

CI is evidence for host behavior only; it does not compile the STM32 adapter
against Cube-generated sources.

Integration tests use subprocess timeouts so a regression in argument handling
cannot leave a CI runner stuck in an unbounded simulation. The complete
problem / solution mapping is in the root README.

## Physical bring-up checklist (currently unverified)

- [ ] Record exact NUCLEO-F401RE revision, sensor breakout part markings, HAL,
  CMSIS, FreeRTOS, compiler, and CubeIDE versions.
- [ ] Save a clean target build log with warnings enabled.
- [ ] Verify 3.3 V rail and idle-high SCL/SDA before attaching sensors.
- [ ] Capture 400 kHz I2C identity and burst-read transactions on a logic analyzer.
- [ ] Confirm TIM2 interrupt frequency is 100.00 Hz and ISR priority satisfies
  FreeRTOS syscall constraints.
- [ ] Capture at least 60 seconds of 10 Hz NDJSON with zero malformed lines.
- [ ] Report DWT min/mean/p95/p99/max for acquisition and processing over at
  least 10 minutes; verify acquisition max remains safely below 10 ms.
- [ ] Disconnect MPU6050 and BMP280 separately; attach status/counter captures.
- [ ] Inject recoverable NACKs or bus contention and confirm retry accounting.
- [ ] Create queue pressure in a debug build and confirm drop accounting.
- [ ] Suspend each task separately and demonstrate IWDG reset after refresh is
  withheld. Record reset cause from RCC flags on the following boot.
- [ ] Capture `uxTaskGetStackHighWaterMark` for every task and right-size stacks
  with at least the project's required safety margin.
- [ ] Complete an application-appropriate soak test and power-cycle/brownout test.

## Suggested evidence naming

Use `validation/YYYY-MM-DD/` in a release artifact (large binary captures do not
need to live in Git) with files such as `build.log`, `uart.ndjson`,
`i2c-nominal.png`, `bmp-disconnect.ndjson`, and `timing-summary.md`. Record the
Git commit SHA in `timing-summary.md` so results remain traceable.
