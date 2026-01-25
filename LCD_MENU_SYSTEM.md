# 🖥️ Interactive LCD Menu System - Implementation

## Overview
İnteraktif LCD menü sistemi başarıyla implemente edildi! Sistem artık kullanıcı ile LCD ekran üzerinden interaktif olarak iletişim kurabiliyor.

## ✅ Yapılan İşlemler

### 1. LCD Driver Implementation
**Dosyalar:** `i2c_lcd.h`, `i2c_lcd.cpp`

- I2C LCD (HD44780 + PCF8574) sürücüsü yazıldı
- 16x2 LCD display desteği
- I2C Address: 0x27 (veya 0x3F)
- Bağlantılar: PB8 (SCL), PB9 (SDA)

### 2. Menü Sistemi
**Dosya:** `my_main.cpp` (güncellenmiş)

Üç menü durumu (state machine):

#### **MENU_IDLE** - Bekleme Ekranı
```
+------------------+
| Say: ON or OFF   |
| Ready...         |
+------------------+
```
- Kullanıcı "ON" veya "OFF" söyleyebilir
- "ON" → Süre seçim menüsüne geç
- "OFF" → Eğer priz açıksa kapat

#### **MENU_DURATION** - Süre Seçimi
```
+------------------+
| 1:Forever        |
| 2:10 Minutes     |
+------------------+
```
- "ONE" söylenirse → Priz süresiz açılır (Forever mode)
- "TWO" söylenirse → Priz 10 dakika sonra otomatik kapanır
- Hatalı giriş → Menü tekrar gösterilir

#### **MENU_SOCKET_ON** - Priz Açık
**Forever Mode:**
```
+------------------+
| Socket: ON       |
| Mode: Forever    |
+------------------+
```

**10 Minute Mode:**
```
+------------------+
| Socket: ON       |
| Time: 09:45      |
+------------------+
```
- Geri sayım her saniye güncellenir (MM:SS formatında)
- 10 dakika bitince priz otomatik kapanır
- "OFF" söylenirse manuel olarak kapatılabilir

## 🎮 Kullanım Senaryosu

### Örnek 1: Forever Mode
1. **Kullanıcı:** "ON" der
   - LCD: Süre seçim menüsü gösterir
   
2. **Kullanıcı:** "ONE" der
   - Priz açılır (süresiz)
   - LCD: "Socket: ON / Mode: Forever"
   - LED'ler 3 kez yanıp söner (görsel feedback)

3. **Kullanıcı:** "OFF" der
   - Priz kapanır
   - LCD: "Socket: OFF / Say: ON"

### Örnek 2: 10 Minute Timer
1. **Kullanıcı:** "ON" der
   - LCD: Süre seçim menüsü

2. **Kullanıcı:** "TWO" der
   - Priz açılır (10 dakikalık timer başlar)
   - LCD: "Socket: ON / Time: 10:00"
   - Geri sayım başlar: 9:59, 9:58, 9:57...

3. **10 dakika sonra:**
   - Priz otomatik kapanır
   - LCD: "Socket: OFF"
   - Ardından idle ekrana dönülür

4. **VEYA kullanıcı erkenden "OFF" derse:**
   - Timer iptal edilir
   - Priz kapanır

## 🔧 Teknik Detaylar

### LCD Fonksiyonları
```cpp
LCD_Init(&hi2c1);              // LCD'yi başlat
LCD_Clear();                    // Ekranı temizle
LCD_PrintAt(row, col, "text"); // Belirli pozisyona yaz
LCD_Backlight(true);            // Arka ışığı aç
```

### Timer Mantığı
```cpp
// 10 dakikalık timer
socketOnTime = HAL_GetTick();  // Başlangıç zamanı kaydet
uint32_t elapsed = (HAL_GetTick() - socketOnTime) / 1000; // Geçen süre (saniye)
uint32_t remaining = (10 * 60) - elapsed; // Kalan süre

// Her saniye ekranı güncelle
if (HAL_GetTick() - lastTimerUpdate >= 1000) {
    UpdateTimerDisplay(remaining);
    lastTimerUpdate = HAL_GetTick();
}

// 10 dakika doldu mu kontrol et
if (elapsed >= 10 * 60) {
    // Prizi kapat
}
```

### State Machine
```cpp
enum MenuState {
    MENU_IDLE,           // Bekleme
    MENU_DURATION,       // Süre seçimi
    MENU_SOCKET_ON       // Priz açık
};

switch (currentMenu) {
    case MENU_IDLE:
        // ON/OFF komutlarını bekle
        break;
    case MENU_DURATION:
        // ONE/TWO komutlarını bekle
        break;
    case MENU_SOCKET_ON:
        // OFF komutu veya timer bitişini bekle
        break;
}
```

## 🎨 Görsel Feedback

### LED Göstergeleri
- **2 kez yanıp sönme:** ON komutu alındı
- **3 kez yanıp sönme:** Süre seçildi, priz açıldı
- **1 kez uzun yanma:** OFF komutu alındı, priz kapandı
- **Sürekli animasyon:** Ses seviyesi gösterimi

### LCD Mesajları
- Tüm durumlar LCD'de net şekilde gösteriliyor
- Hatalı girişlerde kullanıcı bilgilendiriliyor
- Timer modunda geri sayım canlı gösteriliyor

## 🚀 Avantajlar

1. **Kullanıcı Dostu:** 
   - Tüm durumlar LCD'de görülebiliyor
   - Ne yapılması gerektiği her zaman açık

2. **Esneklik:**
   - Süresiz veya zamanlı çalışma seçeneği
   - İstediğin zaman manuel kapatma

3. **Güvenlik:**
   - 10 dakikalık timer ile otomatik kapanma
   - Unutulan prizlerin enerji israfını önler

4. **Hata Toleransı:**
   - Yanlış komut algılandığında kullanıcı bilgilendiriliyor
   - Sistem kilitlenmiyor, menüde kalıyor

## 📝 Geliştirme Önerileri

İleride eklenebilecek özellikler:
- Farklı timer süreleri (5 dk, 30 dk, 1 saat)
- Programlanabilir açılma/kapanma
- Haftalık zamanlayıcı
- Enerji tüketimi gösterimi
- Birden fazla priz kontrolü

## 🔌 I2C Bağlantısı

LCD bağlantıları:
```
STM32F429           LCD Module
---------           ----------
PB8 (SCL)    ----> SCL
PB9 (SDA)    ----> SDA
GND          ----> GND
5V           ----> VCC
```

I2C Address:
- Genellikle `0x27` (bazı modüllerde `0x3F`)
- Driver otomatik olarak deneyecek

## ✨ Sonuç

Sistem başarıyla interaktif hale getirildi! Artık sadece ses komutlarıyla değil, LCD ekran üzerinden de kullanıcı ile etkileşim kuruluyor. Timer fonksiyonu ile priz güvenli ve verimli şekilde kullanılabiliyor.

**Tebrikler! 🎉** İnteraktif menü sistemi tam çalışır durumda!
