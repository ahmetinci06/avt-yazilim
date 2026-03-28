# 💻 AVT Racing — Yazılım Ekibi

## 📊 İlerleme & Çalışma Akışı

- [🔄 WORKFLOW.md](WORKFLOW.md) — Çalışma akışı ve PR kuralları
- [📊 PROGRESS.md](PROGRESS.md) — Görev durumları ve ilerleme
- [🐛 BUGS.md](BUGS.md) — Bug raporları ve debug takibi

> Her değişiklik PR ile yapılır. Review: @ahmetinci06 (Aİ) ☕ & Yaver (AI CTO) 🤖

---

> **avt-yazilim** — VCU firmware, telemetri sistemi, dashboard ve veri analiz araçları.

[![Ana Hub](https://img.shields.io/badge/Ana%20Hub-avt--hub--dev-blue)](https://github.com/ahmetinci06/avt-hub-dev)
[![TEKNOFEST 2026](https://img.shields.io/badge/TEKNOFEST-2026-red)](https://teknofest.org)
[![MCU](https://img.shields.io/badge/MCU-STM32G491RE-blue)]()

---

## 📂 Repo Yapısı

```
avt-yazilim/
├── vcu-firmware/              # 🚗 VCU + ESC firmware (STM32)
│   ├── Core/Inc/              # Header dosyaları
│   │   ├── vcu_pedal.h        # ✅ Pedal okuma + güvenlik
│   │   ├── vcu_state.h        # ✅ State machine
│   │   ├── vcu_can.h          # ✅ CAN haberleşme
│   │   ├── esc_commutation.h  # ✅ 6-step BLDC commutation
│   │   ├── esc_motor_control.h # ✅ Pedal→duty + ramp
│   │   ├── esc_sensors.h      # ✅ Akım + sıcaklık
│   │   ├── esc_safety.h       # ✅ Overcurrent/overtemp
│   │   ├── esc_energy.h       # ✅ Wh/km enerji izleme
│   │   └── esc_telemetry.h    # ✅ UART telemetri + SD stub
│   ├── Core/Src/              # Kaynak kodları
│   ├── Drivers/               # STM32 HAL
│   └── docs/technical/        # Teknik dökümanlar
├── docs/                      # Genel dökümanlar
├── PROGRESS.md                # Görev durumları
├── WORKFLOW.md                # PR kuralları
└── BUGS.md                    # Bug takibi
```

## ⚡ Modül Durumu

| Modül | Dosya | Durum | Açıklama |
|-------|-------|-------|----------|
| Pedal Okuma | `vcu_pedal` | ✅ Done | 16x oversampling, wire-break detection |
| State Machine | `vcu_state` | ✅ Done | OFF→READY→DRIVE→FAULT |
| CAN | `vcu_can` | ✅ Done | BMS + motor telemetri |
| ESC Commutation | `esc_commutation` | ✅ Done | 6-step hall, TIM1 PWM |
| Motor Kontrol | `esc_motor_control` | ✅ Done | Ramp limiter |
| ESC Sensörler | `esc_sensors` | ✅ Done | ADC DMA + TMP102 |
| ESC Güvenlik | `esc_safety` | ✅ Done | Overcurrent/overtemp |
| Enerji İzleme | `esc_energy` | ✅ Done | Wh/km, hız, mesafe |
| Telemetri | `esc_telemetry` | ✅ Done | UART CSV 5Hz |
| SD Kart Log | `esc_telemetry` | 🔲 Stub | FatFS gerekli |
| Dashboard | — | 📋 Planned | Web veya Python GUI |

## 🎯 Hedefler

| Metrik | Hedef |
|--------|-------|
| Enerji verimliliği | < 110 Wh/km |
| Ana döngü | 100 Hz |
| Telemetri | 5 Hz (UART CSV) |

## 👥 Yazılım Ekibi

| İsim | Rol |
|------|-----|
| Aybüke Pamukçu | Yazılım — ESC motor control |
| Ali Efe Kaya | Yazılım |
| Nisa Yegin | Yazılım |
| Ata Doğan | Yazılım |
| Ege Dönmez | Yazılım |

## 📖 Teknik Dökümanlar

- [ESC Entegrasyonu](vcu-firmware/docs/technical/ESC-INTEGRATION.md)
- [Enerji İzleme](vcu-firmware/docs/technical/ENERGY-MONITORING.md)
- [Telemetri Sistemi](vcu-firmware/docs/technical/TELEMETRY-SYSTEM.md)
- [Yazılım Mimarisi](vcu-firmware/docs/technical/SOFTWARE-ARCHITECTURE.md)
