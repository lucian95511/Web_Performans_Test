## Özellikler
- **Çok Kanallı Yapı:** Aynı anda yüzlerce istek gönderebilir.
- **Dinamik Yük:** Test sırasında yükü otomatik olarak artırır.
- **İki Dil Desteği:** Türkçe ve İngilizce arayüz seçeneği.
- **Detaylı Rapor:** Gecikme süreleri (Min/Ort/Max) ve saniyelik istek (RPS) verilerini sunar.

## 🛠️ Kurulum
1. Visual Studio'da yeni bir **C++ Console App** projesi açın.
2. Yukarıdaki kodu `main.cpp` dosyasına yapıştırın.
3. Proje özelliklerinden `Release` modunu seçin.
4. Derleyin ve çalıştırın.

## 📊 Örnek Çıktı
```text
================ ANALIZ RAPORU ================
Hedef URL           : [https://example.com](https://example.com)
Toplam Istek        : 1000
Sure                : 12.45 s
Istek/Saniye        : 80.32
-----------------------------------------------
Status 200          : 1000 adet
-----------------------------------------------
Min Gecikme         : 45 ms
Ort. Gecikme        : 120 ms
Max Gecikme         : 450 ms
===============================================
