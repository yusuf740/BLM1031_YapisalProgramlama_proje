# Kütüphane Otomasyon Sistemi

Bu proje, Yıldız Teknik Üniversitesi Bilgisayar Mühendisliği bölümü **BLM1031 Yapısal Programlama** dersi dönem projesi kapsamında geliştirilmiştir. Amaç, bir kütüphanede kitap, öğrenci, yazar ve işlem yönetimini gerçekleştiren bir terminal uygulaması oluşturmaktır.

## Geliştirici Bilgileri

- **İsim:** Yusuf İBİN
- **Numara:** 24011074
- **E-posta:** yusuf.ibin@std.yildiz.edu.tr

---

## Özellikler

Uygulama aşağıdaki işlemleri destekler:

### Öğrenci İşlemleri
- Öğrenci ekleme, silme, güncelleme
- Öğrenci bilgisi görüntüleme
- Tüm öğrencileri listeleme
- Cezalı (puanı negatif) öğrencileri listeleme
- Kitap teslim etmeyen öğrencileri listeleme

### Kitap İşlemleri
- Kitap ekleme, silme, güncelleme
- Kitap bilgisi arama ve görüntüleme (isim/ISBN ile)
- Kitap-yazar eşleştirme
- Kitap yazarını güncelleme
- Raftaki kitapları listeleme
- Zamanında teslim edilmeyen kitapları listeleme
- Kitap ödünç alma ve teslim etme işlemleri

### Yazar İşlemleri
- Yazar ekleme, silme, güncelleme
- Yazara ait kitapları görüntüleme

---

## Dosya Yapısı ve Veri Yönetimi

Uygulama CSV dosyaları üzerinden veri okuma/yazma işlemleri yapar:

- `Ogrenciler.csv` — Öğrenci bilgileri
- `Yazarlar.csv` — Yazar bilgileri
- `Kitaplar.csv` — Kitap bilgileri
- `KitapOrnekleri.csv` — Kitap örnekleri ve rafta durumu
- `KitapYazar.csv` — Kitap-yazar eşleşmeleri
- `KitapOdunc.csv` — Ödünç alma ve teslim işlemleri

---

## Derleme ve Çalıştırma

Aşağıdaki komutla programı derleyebilir ve çalıştırabilirsiniz:

```bash
gcc main.c -o kutuphane -std=c99
./kutuphane
```

> Not: `main.c` yerine kaynak dosyanızın adını yazmalısınız.

---

## Notlar

- Kullanıcı dostu bir terminal arayüzü sağlar.
- Bellek yönetimi dikkatli yapılmış ve tüm kaynaklar program sonunda serbest bırakılır.
- Tarih işlemleri `dd.mm.yyyy` formatında yapılır ve geç teslimlerde otomatik puan düşümü uygulanır.

---

Bu proje, C programlama dilinde yapı, dosya işlemleri, bağlı listeler, tarih işlemleri ve kullanıcı arayüzü konularında kapsamlı bir pratik sağlar.
