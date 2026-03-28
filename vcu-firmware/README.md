# 🚗 VCU Firmware — Yerli ESC + Enerji İzleme

AVT Racing aracının VCU (Vehicle Control Unit) firmware'i.
STM32G491RET üzerinde çalışır. Yerli ESC ile doğrudan motor sürüşü.

## Kaynak Repo
Ana geliştirme: [AVT-VCU-DEV](https://github.com/ahmetinci06/AVT-VCU-DEV) (private)

Bu kopya avt-yazilim ekibinin erişimi için senkronize tutulur.

## Modüller

### VCU (Araç Kontrol)
| Modül | Dosya | Durum |
|-------|-------|-------|
| Pedal Okuma | `vcu_pedal.c/h` | ✅ |
| State Machine | `vcu_state.c/h` | ✅ |
| CAN Haberleşme | `vcu_can.c/h` | ✅ |

### ESC (Motor Sürüş)
| Modül | Dosya | Durum |
|-------|-------|-------|
| 6-Step Commutation | `esc_commutation.c/h` | ✅ |
| Motor Kontrol | `esc_motor_control.c/h` | ✅ |
| Akım + Sıcaklık | `esc_sensors.c/h` | ✅ |
| Güvenlik | `esc_safety.c/h` | ✅ |
| Enerji İzleme | `esc_energy.c/h` | ✅ |

## Teknik Dökümanlar
- [ESC Entegrasyonu](docs/technical/ESC-INTEGRATION.md)
- [Enerji İzleme](docs/technical/ENERGY-MONITORING.md)
- [Yazılım Mimarisi](docs/technical/SOFTWARE-ARCHITECTURE.md)

## Hedef
- Enerji verimliliği: **< 110 Wh/km**
- Ana döngü: 100 Hz (10ms)
- Güvenlik tepkisi: < 1ms
