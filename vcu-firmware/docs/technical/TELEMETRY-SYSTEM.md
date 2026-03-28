# Telemetri Sistemi — esc_telemetry

## Genel Bakış

Yarış sırasında ve sonrasında veri analizi için tüm araç verilerini loglar.
İki çıkış kanalı:

1. **UART (aktif)** — CSV formatında seri çıkış, RF modülü veya kablolu dashboard
2. **SD Kart (stub)** — FatFS + SPI/SDIO ile 60 dakikalık circular buffer

## UART CSV Formatı

5 Hz (200ms aralık), 115200 baud, USART2 (ST-LINK VCP veya RF modülü)

```
tick,state,pedal,brake,duty,mA,mT,bV,bA,soc,W,Wh,rWh,kmh,km,whkm,hall,fault
12500,2,45.3,0,4500,12.5,42.1,48.2,8.5,85,409.7,1.25,0.05,32.5,0.450,2.8,3,0x00000000
```

| Sütun | Açıklama | Birim |
|-------|----------|-------|
| tick | Zaman damgası | ms |
| state | VCU durumu (0=OFF, 1=READY, 2=DRIVE, 3=FAULT) | enum |
| pedal | Pedal pozisyonu | % |
| brake | Fren durumu | 0/1 |
| duty | Motor PWM duty | 0-9999 |
| mA | Motor akımı (filtrelenmiş) | A |
| mT | Motor/ESC sıcaklığı | °C |
| bV | Batarya gerilimi | V |
| bA | Batarya akımı | A |
| soc | Şarj durumu | % |
| W | Anlık güç | W |
| Wh | Toplam tüketim | Wh |
| rWh | Rejen enerji | Wh |
| kmh | Hız | km/h |
| km | Mesafe | km |
| whkm | Verimlilik | Wh/km |
| hall | Hall sensör kodu | 1-6 |
| fault | Hata bayrakları | hex |

## SD Kart (Planlanıyor)

CubeMX'te yapılacak:
1. SPI2 veya SDMMC1 peripheral ekle
2. FatFS middleware ekle
3. `esc_telemetry.c` içindeki `WriteSD()` ve `FlushSD()` fonksiyonlarını implement et

Kapasite: 18000 frame × 200ms = 60 dakika circular buffer

## Dashboard Bağlantısı

UART çıkışını almak için:
- **Kablolu:** USB-UART → laptop, Python/serial ile parse
- **Kablosuz:** HC-12 veya LoRa modülü → alıcı → dashboard
- **CAN:** 0x302 frame'i de dashboard'a enerji verileri gönderir

## Kullanım

```c
// main.c init bölümüne:
ESC_Telemetry_Init();

// main loop'a:
ESC_Telemetry_Update(tick);

// Power-off öncesi (SD kart varsa):
ESC_Telemetry_FlushSD();
```
