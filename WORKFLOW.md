# 🔄 Çalışma Akışı — avt-yazilim

> Bu dosya ekibin GitHub üzerinden nasıl çalışacağını açıklar.

## 📋 Genel Akış

```
1. Branch Oluştur    →  develop'tan yeni branch aç
2. Çalış & Commit    →  değişikliklerini yap, commit at
3. Push & PR Aç      →  push et, Pull Request oluştur
4. Review            →  Ahmet & Yaver review eder
5. Merge             →  onay sonrası develop'a merge
6. Release           →  develop → main (stabil sürüm)
```

## 🌿 Branch Kuralları

| Branch | Amaç | Kim Push Eder? |
|--------|-------|----------------|
| `main` | Stabil, son sürüm | Sadece PR ile (korumalı) |
| `develop` | Aktif geliştirme | Sadece PR ile |
| `feature/xxx` | Yeni özellik | Herkes kendi branch'inde |
| `fix/xxx` | Bug fix | Herkes kendi branch'inde |
| `docs/xxx` | Döküman güncellemesi | Herkes kendi branch'inde |

## 🚀 Adım Adım

### 1. Yeni Branch Oluştur
```bash
git checkout develop
git pull origin develop
git checkout -b feature/görev-adı
```

### 2. Çalış & Commit At
```bash
git add .
git commit -m "feat: görev açıklaması"
```

### 3. Push Et
```bash
git push -u origin feature/görev-adı
```

### 4. Pull Request (PR) Aç
- GitHub'a git → "Compare & pull request" butonuna tıkla
- **Base branch:** `develop`
- **Compare branch:** `feature/görev-adı`
- Açıklama yaz, reviewer olarak @ahmetinci06 ekle
- PR açıklamasında PROGRESS.md'deki ilgili görevi referans ver

### 5. Review Süreci
- Ahmet ve Yaver (AI CTO) PR'ı review eder
- Değişiklik istenirse → düzelt → tekrar push et (aynı branch)
- Onay gelince → merge edilir

### 6. Merge Sonrası
```bash
git checkout develop
git pull origin develop
```
Eski branch'i sil:
```bash
git branch -d feature/görev-adı
```

## 📊 İlerleme Güncelleme

Her PR ile birlikte `PROGRESS.md` ve/veya `BUGS.md` güncellenmeli:
1. Görevin durumunu değiştir (💡 → 🔄 → 🔍 → ✅)
2. Tarih ve notlar kısmını güncelle
3. Bug fix ise BUGS.md'ye de ekle

## ⚠️ Önemli Kurallar

- ❌ `main` veya `develop`'a direkt push YAPMA
- ✅ Her zaman PR aç
- ✅ Commit mesajlarını açıklayıcı yaz
- ✅ PR açmadan önce `develop`'tan son değişiklikleri çek
- ✅ Conflict varsa kendi branch'inde çöz
- ✅ Her PR'da en az 1 review gerekli

## 🤝 Review Yapanlar

| Reviewer | Rol |
|----------|-----|
| @ahmetinci06 | Proje Lideri — Final onay |
| Yaver (AI) | AI CTO — Kod kalitesi & teknik review |

> Sorularınız varsa BUGS.md'ye yazın veya PR comment'inde sorun.
