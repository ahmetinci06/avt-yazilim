# 💻 AVT Racing — Yazılım Ekibi

> **avt-yazilim** — Yazılım projeleri, telemetri sistemi, araç dashboard'u ve veri analiz araçlarının deposu.

[![Ana Hub](https://img.shields.io/badge/Ana%20Hub-avt--hub--dev-blue)](https://github.com/ahmetinci06/avt-hub-dev)
[![TEKNOFEST 2026](https://img.shields.io/badge/TEKNOFEST-2026-red)](https://teknofest.org)

---

## 📋 Hakkında

Bu repo, AVT Racing aracının yazılım altyapısını kapsar: gerçek zamanlı telemetri, araç durum monitörü, veri loglama, dashboard arayüzü ve analiz araçları.

**Merkezi Hub:** [avt-hub-dev](https://github.com/ahmetinci06/avt-hub-dev)

---

## 👥 Yazılım Ekibi

| İsim | Rol |
|------|-----|
| Aybüke Pamukçu | Yazılım |
| Ali Efe Kaya | Yazılım |
| Nisa Yegin | Yazılım |
| Ata Doğan | Yazılım |
| Ege Dönmez | Yazılım |

---

## 📁 Klasör Yapısı

```
avt-yazilim/
├── telemetry/                  # Telemetri sistemi
│   ├── receiver/               # Veri alıcı (Python/Node.js)
│   ├── parser/                 # CAN/UART veri ayrıştırıcı
│   └── logger/                 # Veri kayıt sistemi
├── dashboard/                  # Web dashboard (React)
│   ├── src/
│   │   ├── components/         # UI bileşenleri
│   │   ├── pages/              # Sayfalar
│   │   └── hooks/              # Custom hooks
│   ├── public/
│   └── package.json
├── api/                        # Backend API (Node.js/Express)
│   ├── src/
│   │   ├── routes/             # API rotaları
│   │   ├── controllers/        # Controller'lar
│   │   └── models/             # Veri modelleri
│   └── package.json
├── analysis/                   # Veri analiz araçları (Python)
│   ├── notebooks/              # Jupyter Notebooks
│   └── scripts/                # Analiz scriptleri
├── simulation/                 # Yazılım simülasyonu
│   └── vehicle-model/          # Araç modeli simülasyonu
├── tests/                      # Test dosyaları
│   ├── unit/
│   └── integration/
├── scripts/                    # Yardımcı scriptler
└── docs/
    ├── GITHUB-GUIDE.md
    ├── api-docs.md             # API dökümanları
    └── telemetry-protocol.md   # Telemetri protokolü
```

---

## 🔧 Tech Stack

| Teknoloji | Kullanım |
|-----------|----------|
| **React** | Dashboard arayüzü |
| **Node.js / Express** | Backend API |
| **Python** | Telemetri, veri analiz |
| **WebSocket** | Gerçek zamanlı veri |
| **SQLite / InfluxDB** | Veri saklama |
| **Chart.js / Recharts** | Grafikler |

---

## ⚙️ Kurulum

### Gereksinimler
- Node.js 18+
- Python 3.10+
- npm veya yarn

### Dashboard

```bash
cd dashboard
npm install
npm run dev        # Geliştirme
npm run build      # Production build
```

### API

```bash
cd api
npm install
cp .env.example .env   # .env dosyasını düzenle
npm run dev
```

### Telemetri (Python)

```bash
cd telemetry
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
python receiver/main.py
```

---

## 🌿 Branch Yapısı

```
main          ← Production-ready, test edilmiş kod
  └── develop ← Aktif geliştirme
        ├── feature/telemetri-dashboard
        ├── feature/can-parser
        ├── feature/data-logger
        └── fix/websocket-baglanti
```

### Branch İsimlendirme
```
feature/<özellik>     # Yeni özellik
fix/<konu>            # Bug düzeltme
refactor/<konu>       # Yeniden yapılandırma
test/<konu>           # Test ekleme
docs/<konu>           # Döküman
```

---

## 🧪 Testler

```bash
# Node.js testler
npm test
npm run test:coverage

# Python testler
pytest
pytest --cov=. --cov-report=html
```

---

## 🚀 Hızlı Başlangıç

```bash
git clone git@github.com:ahmetinci06/avt-yazilim.git
cd avt-yazilim
git checkout develop
git checkout -b feature/yaptığın-özellik
```

Detaylı rehber: [docs/GITHUB-GUIDE.md](docs/GITHUB-GUIDE.md)

---

## 📚 Kaynaklar

- [CONTRIBUTING.md](CONTRIBUTING.md)
- [docs/GITHUB-GUIDE.md](docs/GITHUB-GUIDE.md)
- [Ana Hub — avt-hub-dev](https://github.com/ahmetinci06/avt-hub-dev)

---

*🏎️ AVT Racing — TEKNOFEST 2026 | Yazılım Ekibi*
