# 🤝 Katkı Rehberi — AVT Racing

Bu repoya katkıda bulunmak için aşağıdaki kuralları takip et.

---

## 🌿 Branch Kuralları

| Branch | Açıklama | Kim kullanır |
|--------|----------|--------------|
| `main` | Stabil, production-ready | Sadece merge ile güncellenir |
| `develop` | Aktif geliştirme | Tüm ekip buraya PR açar |
| `feature/*` | Yeni özellik | Özellik geliştirirken |
| `fix/*` | Hata düzeltme | Bug fix yaparken |
| `docs/*` | Döküman güncellemeleri | Döküman yazarken |

**⚠️ Kural:** `main` branch'ine direkt push YASAK. Her zaman PR aç!

---

## 📝 Commit Mesaj Formatı

```
<tip>(<kapsam>): <kısa açıklama>

[isteğe bağlı uzun açıklama]

[isteğe bağlı: closes #issue_no]
```

### Tip Seçenekleri
| Tip | Ne zaman |
|-----|----------|
| `feat` | Yeni özellik eklendi |
| `fix` | Bug düzeltildi |
| `docs` | Sadece döküman değişikliği |
| `style` | Kod formatı (logic değişmedi) |
| `refactor` | Yeniden yapılandırma |
| `test` | Test eklendi/düzenlendi |
| `chore` | Build, bağımlılık güncellemeleri |
| `design` | CAD/PCB/Tasarım dosyası değişikliği |
| `sim` | Simülasyon dosyası değişikliği |

### Örnekler
```
feat(motor): PWM kontrol algoritması eklendi
fix(sensor): IMU okuma hatası düzeltildi
docs: kurulum adımları güncellendi
design(pcb): ana kart v2.1 revize edildi
sim(fea): ön süspansiyon analizi eklendi
```

---

## 🔄 Katkı Süreci

### 1. Repo'yu Hazırla
```bash
git clone git@github.com:ahmetinci06/REPO_ADI.git
cd REPO_ADI
git checkout develop
git pull origin develop
```

### 2. Branch Oluştur
```bash
git checkout -b feature/yaptığın-şey
# Örnek: git checkout -b feature/hiz-kontrol-pid
```

### 3. Değişikliklerini Yap

Kodunu/dosyalarını ekle/düzenle.

### 4. Commit Et
```bash
git add .
git commit -m "feat: açıklayıcı mesaj"
```

### 5. Push Et
```bash
git push origin feature/yaptığın-şey
```

### 6. Pull Request Aç
- GitHub'da repoya git
- **"Compare & pull request"** butonuna tıkla
- **Base:** `develop`, **Compare:** kendi branch'in
- Başlık ve açıklama yaz
- Reviewer olarak bir ekip arkadaşı ekle
- **Create pull request** tıkla

---

## ✅ PR Kuralları

- [ ] PR başlığı açıklayıcı olmalı
- [ ] Ne yaptığını, neden yaptığını açıklama bölümüne yaz
- [ ] En az 1 reviewer ekle
- [ ] CI/test varsa yeşil olduğuna emin ol
- [ ] Büyük değişiklikleri küçük PR'lara böl (500 satır max ideal)

---

## ❌ Yapılmaması Gerekenler

- `main`'e direkt push yapma
- Büyük binary dosyaları (>100MB) commit etme
- API key, şifre gibi hassas bilgileri commit etme
- Conflict'li kod push etme

---

## 📁 Dosya Boyutu Kuralları

| Dosya Tipi | Max Boyut | Alternatif |
|------------|-----------|------------|
| Kaynak kod | Sınırsız | — |
| Döküman/PDF | 50MB | Google Drive link |
| CAD dosyaları | 100MB | Git LFS veya Drive |
| Simülasyon sonuçları | 50MB | Drive link + özet |

Büyük dosyalar için Google Drive/OneDrive'a yükle, linki `docs/` klasörüne ekle.

---

## 🆘 Yardım

Takıldığında:
1. `docs/GITHUB-GUIDE.md`'e bak
2. Ekip kanalında sor
3. [GitHub Docs TR](https://docs.github.com/tr) kontrol et

---

*AVT Racing — TEKNOFEST 2026 🏎️*
