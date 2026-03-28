# Energy Monitoring — esc_energy Module

## Amaç

TEKNOFEST Efficiency Challenge yarışmasında sıralama **Wh/km** bazında yapılıyor.
Bu modül gerçek zamanlı enerji tüketimi takibi yaparak yarış sırasında stratejik karar almayı sağlar.

**Hedef: < 110 Wh/km**

## Ne Ölçüyor?

| Metrik | Kaynak | Açıklama |
|--------|--------|----------|
| Gerilim (V) | BMS CAN (0x101) | Batarya paket gerilimi |
| Akım (A) | BMS CAN (0x102) | Batarya akımı (+deşarj, -regen) |
| Güç (W) | V × I | Anlık güç tüketimi |
| Enerji (Wh) | ∫ P dt | Trapezoidal integrasyon |
| Rejen (Wh) | ∫ P dt (P<0) | Rejeneratif frenleme enerjisi |
| Hız (km/h) | Hall RPM → tekerlek | Motor RPM × tekerlek çevresi |
| Mesafe (km) | ∫ v dt | Trapezoidal integrasyon |
| Verimlilik (Wh/km) | E_net / d | Yarış metriki |

## Mimari

```
BMS CAN ──► voltage + current ──┐
                                 ├──► ESC_Energy_Update() ──► energy_data
Hall sensors ──► RPM estimate ──┘
                                         │
                                         ├──► Wh/km (efficiency)
                                         ├──► speed (km/h)
                                         ├──► CAN frame (0x302)
                                         └──► telemetry logging
```

## CAN Telemetry Frame (0x302)

8 byte, big-endian, tüm değerler ×10 (uint16):

| Byte | Alan | Birim | Range |
|------|------|-------|-------|
| 0-1 | Verimlilik | Wh/km × 10 | 0-6553.5 |
| 2-3 | Hız | km/h × 10 | 0-655.3 |
| 4-5 | Net enerji | Wh × 10 | 0-6553.5 |
| 6-7 | Mesafe | km × 10 | 0-6553.5 |

## Yapılandırma

`esc_energy.h` içinde ayarlanacak parametreler:

```c
#define ENERGY_WHEEL_CIRCUMFERENCE_M   1.60f   // π × tekerlek çapı (m)
#define ENERGY_MOTOR_POLE_PAIRS        7U      // Motor kutup çifti sayısı
#define ENERGY_GEAR_RATIO              1.0f    // Motor:tekerlek dişli oranı
#define ENERGY_UPDATE_INTERVAL_MS      100U    // Hesaplama aralığı
```

**⚠️ Bu değerler araç ölçümlerine göre ayarlanmalı!**

## API

```c
ESC_Energy_Init();                           // Başlat
ESC_Energy_Update(HAL_GetTick());            // Main loop'ta çağır
ESC_Energy_GetEfficiency();                  // → Wh/km
ESC_Energy_GetSpeed();                       // → km/h
ESC_Energy_GetDistanceKm();                  // → km
ESC_Energy_GetNetEnergyWh();                 // → Wh
ESC_Energy_Reset(HAL_GetTick());             // Yarış başlangıcında sıfırla
ESC_Energy_PackCANFrame(data);               // CAN telemetri paketi
const EnergyData_t* ESC_Energy_GetData();    // Tüm verilere erişim
```

## Doğruluk

- Trapezoidal integrasyon kullanıyor (Euler'den daha doğru)
- 100ms aralık → 10 Hz güncelleme (yeterli doğruluk)
- Minimum 100m mesafe sonrasında Wh/km hesaplar (ilk metrelerde anlamsız)
- Hall timeout: 500ms hareketsizlik → hız = 0

## Test Planı

1. **Bench test:** Sabit V/I kaynağı ile Wh birikim doğrulama
2. **RPM test:** Bilinen hızda motor → speed_kmh doğrulama
3. **Mesafe test:** Bilinen mesafe sürüşü → distance_km karşılaştırma
4. **CAN test:** 0x302 frame decode doğrulama
5. **Reset test:** ESC_Energy_Reset sonrası tüm sayaçlar sıfır mı
