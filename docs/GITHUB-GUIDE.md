# 📘 GitHub Kullanım Rehberi — AVT Racing

> Bu rehber, Git ve GitHub'a yeni başlayanlar için adım adım hazırlanmıştır. Sabırla oku, bir kez öğrenince her şey çok daha kolay!

---

## 📋 İçindekiler

1. [Git Kurulumu](#1-git-kurulumu)
2. [SSH Key Kurulumu](#2-ssh-key-kurulumu)
3. [Repo Klonlama](#3-repo-klonlama)
4. [Branch Oluşturma](#4-branch-oluşturma)
5. [Commit ve Push](#5-commit-ve-push)
6. [Pull Request Açma](#6-pull-request-açma)
7. [Code Review](#7-code-review)
8. [Conflict Çözme](#8-conflict-çözme)
9. [GitHub Desktop Alternatifi](#9-github-desktop-alternatifi)
10. [Komut Cheat Sheet](#10-komut-cheat-sheet)

---

## 1. Git Kurulumu

### Windows
1. [https://git-scm.com/download/win](https://git-scm.com/download/win) adresine git
2. İndir ve kur (varsayılan ayarlar yeterli)
3. "Git Bash" uygulamasını aç

### macOS
```bash
# Homebrew varsa:
brew install git

# Yoksa Xcode Command Line Tools:
xcode-select --install
```

### Linux (Ubuntu/Debian)
```bash
sudo apt update && sudo apt install git
```

### Kurulumu Doğrula
```bash
git --version
# Çıktı: git version 2.x.x
```

### İlk Kez Kimliğini Tanıt
```bash
git config --global user.name "Adın Soyadın"
git config --global user.email "email@ornek.com"
```

---

## 2. SSH Key Kurulumu

SSH key, GitHub'a şifre girmeden bağlanmanı sağlar. **Bir kez kurulunca sonsuza kadar çalışır.**

### Adım 1: SSH Key Oluştur
```bash
ssh-keygen -t ed25519 -C "email@ornek.com"
# Tüm sorulara Enter bas (varsayılan)
```

### Adım 2: Public Key'i Kopyala
```bash
# macOS:
cat ~/.ssh/id_ed25519.pub | pbcopy

# Linux:
cat ~/.ssh/id_ed25519.pub
# Çıkan metni kopyala

# Windows (Git Bash):
cat ~/.ssh/id_ed25519.pub | clip
```

### Adım 3: GitHub'a Ekle
1. GitHub'a giriş yap → sağ üst profil fotoğrafı → **Settings**
2. Sol menüde **SSH and GPG keys**
3. **New SSH key** butonuna tıkla
4. Title: "Laptop" veya "PC" gibi bir isim
5. Key: kopyaladığın metni yapıştır
6. **Add SSH key** tıkla

### Adım 4: Test Et
```bash
ssh -T git@github.com
# Başarılı: "Hi kullanicin! You've successfully authenticated..."
```

---

## 3. Repo Klonlama

```bash
# SSH ile (önerilen — SSH key kurduysan):
git clone git@github.com:ahmetinci06/REPO_ADI.git

# HTTPS ile (SSH kurmadan):
git clone https://github.com/ahmetinci06/REPO_ADI.git

# Klasöre gir:
cd REPO_ADI
```

---

## 4. Branch Oluşturma

**Her yeni özellik veya düzeltme için ayrı branch açılır!**

```bash
# Önce develop branch'ine geç:
git checkout develop

# En güncel hali çek:
git pull origin develop

# Yeni branch oluştur ve geç:
git checkout -b feature/ozellik-adi
# Örnek: git checkout -b feature/motor-kontrol
# Örnek: git checkout -b fix/pcb-hata-duzeltme

# Branch'leri listele:
git branch
```

### Branch İsimlendirme Kuralları
| Tür | Format | Örnek |
|-----|--------|-------|
| Yeni özellik | `feature/kisa-aciklama` | `feature/telemetri-modulu` |
| Bug düzeltme | `fix/kisa-aciklama` | `fix/sensor-okuma-hatasi` |
| Döküman | `docs/kisa-aciklama` | `docs/kurulum-rehberi` |
| Tasarım | `design/kisa-aciklama` | `design/pcb-v2` |

---

## 5. Commit ve Push

```bash
# Değişiklikleri gör:
git status

# Tüm değişiklikleri stage'e ekle:
git add .

# Belirli dosya ekle:
git add dosya_adi.py

# Commit oluştur:
git commit -m "feat: motor kontrol algoritması eklendi"

# Branch'i GitHub'a gönder:
git push origin feature/ozellik-adi

# İlk push ise -u ile:
git push -u origin feature/ozellik-adi
```

### Commit Mesaj Formatı
```
<tip>: <kısa açıklama>

Örnek:
feat: yeni özellik eklendi
fix: hata düzeltildi
docs: döküman güncellendi
style: kod formatı düzenlendi
refactor: kod yeniden yapılandırıldı
test: test eklendi
```

---

## 6. Pull Request Açma

Branch'ini main/develop'a merge etmek için PR açarsın.

### Adım Adım:
1. GitHub'da repoya git
2. **"Compare & pull request"** butonuna tıkla (push sonrası çıkar)
   - Yoksa **Pull requests** → **New pull request**
3. **base:** `develop`, **compare:** kendi branch'in
4. Başlık yaz: `feat: Motor kontrol algoritması`
5. Açıklama yaz: Ne yaptın, neden, nasıl test ettiler
6. **Create pull request** tıkla
7. Reviewer ekle (ekip arkadaşın)

---

## 7. Code Review

### Reviewer Olarak:
1. PR'a git → **Files changed** sekmesi
2. Satırın üzerine hover → **+** işaretine tıkla
3. Yorum yaz
4. **Review changes** → Approve veya Request changes

### Yazar Olarak:
1. Yorumları oku
2. Değişiklikeri yap, commit et, push et
3. PR otomatik güncellenir
4. "Resolved" işaretle

---

## 8. Conflict Çözme

Conflict, iki kişi aynı dosyanın aynı satırını değiştirince olur.

```bash
# develop'u kendi branch'ine çek:
git checkout feature/benim-branch
git merge develop

# Conflict varsa git söyler:
# CONFLICT (content): Merge conflict in dosya.py

# Dosyayı aç, şunları görürsün:
<<<<<<< HEAD
Senin değişikliğin
=======
Develop'taki değişiklik
>>>>>>> develop

# İstediğini seç, diğerini sil, marker satırları da sil
# Sonra:
git add dosya.py
git commit -m "fix: merge conflict çözüldü"
git push origin feature/benim-branch
```

**VS Code** conflict'leri görsel olarak çözmeni sağlar — "Resolve in Merge Editor" diyebilirsin.

---

## 9. GitHub Desktop Alternatifi

Terminal kullanmak istemiyorsan **GitHub Desktop** mükemmel bir seçenek:

1. [desktop.github.com](https://desktop.github.com) adresinden indir
2. GitHub hesabınla giriş yap
3. **Clone a repository** ile repo'yu indir
4. Değişiklikler sol panelde görünür
5. Commit mesajı yaz → **Commit to branch**
6. **Push origin** tıkla

**VS Code** de Git entegrasyonu var — sol paneldeki "Source Control" sekmesi.

---

## 10. Komut Cheat Sheet

```bash
# ─── TEMEL KOMUTLAR ───────────────────────────────
git status                    # Değişiklikleri gör
git log --oneline             # Commit geçmişi (kısa)
git diff                      # Farkları gör

# ─── BRANCH ───────────────────────────────────────
git branch                    # Branch listesi
git checkout develop          # Develop'a geç
git checkout -b yeni-branch   # Yeni branch oluştur
git branch -d eski-branch     # Branch sil (local)

# ─── SYNC ─────────────────────────────────────────
git fetch origin              # GitHub'dan güncelle (merge etme)
git pull origin develop       # Develop'u çek ve merge et
git push origin branch-adi    # Branch'i gönder

# ─── GERI AL ──────────────────────────────────────
git restore dosya.py          # Değişikliği geri al (stage öncesi)
git reset HEAD dosya.py       # Stage'den çıkar
git revert COMMIT_HASH        # Commit'i geri al (yeni commit ile)

# ─── STASH (geçici kaydet) ────────────────────────
git stash                     # Değişiklikleri geçici sakla
git stash pop                 # Geri getir

# ─── TAGLEme ──────────────────────────────────────
git tag v1.0.0                # Tag oluştur
git push origin --tags        # Tag'leri gönder
```

---

## 🆘 Yardım

- Bir şey çalışmıyorsa → ekip kanalında sor!
- GitHub Docs: [docs.github.com/tr](https://docs.github.com/tr)
- Türkçe Git rehberi: [git-scm.com/book/tr](https://git-scm.com/book/tr)

---

*AVT Racing — TEKNOFEST 2026 🏎️*
