# ESC Integration — motor-control-esc → AVT-VCU-DEV

## Kaynak
- **Repo:** [aybukepamukcu/motor-control-esc](https://github.com/aybukepamukcu/motor-control-esc)
- **Lisans:** Apache License 2.0
- **MCU:** STM32G491RET (aynı MCU ✅)

## Ne Entegre Edildi

Aybüke'nin ESC projesindeki 4 modül, AVT VCU mimarisine adapte edildi:

| Kaynak (motor-control-esc) | Hedef (AVT-VCU-DEV) | Değişiklikler |
|---|---|---|
| `commutation.c/h` | `esc_commutation.c/h` | Hall pin PB0-2 → PA0-2, proper CCER manipulation, MOE control |
| `sensors.c/h` | `esc_sensors.c/h` | Throttle ADC kaldırıldı (vcu_pedal handles it), sadece current + TMP102 |
| `motor_control.c/h` | `esc_motor_control.c/h` | Raw ADC throttle → pedal_percent input, vcu_pedal'a bağlandı |
| `safety.c/h` | `esc_safety.c/h` | Brake GPIO kaldırıldı (vcu_state handles it), fault flag range 0x1000+ |

## Mimari

```
                    VCU Katmanı (mevcut)          ESC Katmanı (yeni)
                    ────────────────────          ──────────────────
Pedal ADC ──► vcu_pedal ──► pedal_percent ──► esc_motor_control ──► duty
                  │                                    │
Brake GPIO ──► vcu_state ◄── esc_fault_flags ◄── esc_safety
                  │                                    │
BMS CAN ──► vcu_can                           esc_sensors (current + temp)
                  │                                    │
                  ▼                                    ▼
             State Machine                    esc_commutation (TIM1 PWM)
          (OFF→READY→DRIVE→FAULT)            (6-step hall commutation)
```

## Donanım Eşlemesi

| Sinyal | Pin | Peripheral | Notlar |
|--------|-----|------------|--------|
| Hall A | PA0 | GPIO Input (EXTI) | Rising + Falling edge |
| Hall B | PA1 | GPIO Input (EXTI) | Rising + Falling edge |
| Hall C | PA2 | GPIO Input (EXTI) | Rising + Falling edge |
| Phase U High | PA8 | TIM1_CH1 | PWM |
| Phase V High | PA9 | TIM1_CH2 | PWM |
| Phase W High | PA10 | TIM1_CH3 | PWM |
| Phase U Low | PB13 | TIM1_CH1N | Complementary |
| Phase V Low | PB14 | TIM1_CH2N | Complementary |
| Phase W Low | PB15 | TIM1_CH3N | Complementary |
| Motor Current | PC1 | ADC1_IN7 | DMA |
| Motor Temp | I2C1 | TMP102 (0x48) | SDA/SCL |
| Brake | PC5 | GPIO Input | Mevcut vcu_state |

## ⚠️ CubeMX Güncellemesi Gerekiyor

Bu entegrasyon `main.c`'ye TIM1, I2C1 ve DMA init fonksiyonları ekledi.
**AVT-VCU-DEV.ioc** dosyasının CubeMX'te güncellenmesi gerekiyor:

1. **TIM1:** PWM Generation CH1/CH2/CH3 + CH1N/CH2N/CH3N (complementary)
   - Period: 9999, Prescaler: 0, Dead Time: 170
2. **I2C1:** Standard mode, 7-bit addressing
3. **DMA1 Channel 1:** ADC1 circular mode
4. **ADC1:** Scan mode + 2 channels (throttle CH6/PC0 + current CH7/PC1)
5. **GPIO:** PA0/PA1/PA2 as EXTI Rising+Falling (Hall sensors)

## ⚠️ Motor Doğrulama (İLK TEST ÖNCESİ)

Commutation tablosu (hall code → phase mapping) doğrulanmalı:
1. Motor doğru yönde dönmüyor → tablo swap'ı gerekebilir
2. Motor titriyor → hall sensor A/B/C sıralaması kontrol et
3. Commutation tablosunu test etmek için: düşük duty (%5-10) ile başla

## Test Planı

1. **Bench test:** Motor bağlı değilken TIM1 PWM sinyallerini osiloskopla kontrol et
2. **Hall test:** Motoru elle çevir, hall code okumalarını UART'tan izle
3. **Low-duty test:** %5 duty ile motor dönüşünü kontrol et
4. **Ramp test:** Pedal ile kademeli hızlanma testi
5. **Safety test:** Overcurrent ve overtemp threshold'larını doğrula

## Dosya Listesi

### Yeni dosyalar (8)
- `Core/Inc/esc_commutation.h`
- `Core/Inc/esc_motor_control.h`
- `Core/Inc/esc_sensors.h`
- `Core/Inc/esc_safety.h`
- `Core/Src/esc_commutation.c`
- `Core/Src/esc_motor_control.c`
- `Core/Src/esc_sensors.c`
- `Core/Src/esc_safety.c`

### Değiştirilen dosyalar (1)
- `Core/Src/main.c` — ESC init + main loop entegrasyonu
