# 🛵 W-Smart Rider Dasboard Motor (ESP32-C3)

**W-Smart Rider** adalah *firmware* untuk *Smart Dashboard* kustom sepeda motor berbasis mikrokontroler ESP32-C3. Perangkat ini terhubung secara nirkabel dengan smartphone Android melalui Bluetooth Low Energy (BLE) menggunakan pustaka ChronosESP32 untuk menampilkan waktu, notifikasi, kontrol musik, navigasi, hingga informasi cuaca secara *real-time*.

---

## 🛠️ Komponen & Perangkat Keras (Hardware)

* **Mikrokontroler:** ESP32-C3 Super Mini / Board ESP32-C3 lainnya
* **Layar:** OLED 0.96 inci I2C (128x64 piksel, Driver SSD1306)
* **Input / Sensor:** Sensor Sentuh TTP223 (Touch Sensor)
* **Koneksi Pin:**
  * **SDA OLED:** GPIO 20
  * **SCL OLED:** GPIO 21
  * **Touch Sensor:** GPIO 1

---

## 📚 Pustaka (Libraries) yang Dibutuhkan

Sebelum melakukan proses *compile* & *upload* via Arduino IDE, pastikan pustaka berikut sudah terpasang melalui **Library Manager**:

1. **Adafruit SSD1306**
2. **Adafruit GFX Library**
3. **NimBLE-Arduino** (Oleh h2zero) — *Wajib untuk fondasi koneksi BLE*
4. **ChronosESP32** (Oleh febiego)
5. **WiFiManager** (Oleh tzapu)

---

## 📲 Aplikasi Pendamping HP & Cara Menghubungkan

1. Unduh dan buka aplikasi pendamping **Chronos** (tersedia di Play Store) pada smartphone Android Anda.
2. Aktifkan Bluetooth dan GPS/Lokasi di HP.
3. Hubungkan aplikasi ke perangkat Bluetooth bertuliskan **`W-Smart Rider`**.

---

## ✍️ Cara Mengubah Running Text dari HP

Untuk mengganti teks berjalan (*Running Text*) yang tampil di layar OLED melalui aplikasi Chronos, ikuti langkah berikut:

1. Buka aplikasi **Chronos** di HP yang sudah terhubung ke ESP32.
2. Masuk ke menu pengiriman pesan uji (*Test Notification* / *Media*).
3. Ubah atau pilih ikon aplikasi ke **Tencent Penguin** (logo penguin 🐧).
4. Tulis teks atau pesan yang Anda inginkan di kolom teks.
5. Tekan tombol **Kirim / Send**.
6. Teks pada layar OLED (*Running Text*) akan otomatis berubah sesuai pesan yang baru saja Anda kirimkan dan tersimpan secara permanen di memori internal ESP32.

---

## 💡 Navigasi Mode & Sensor Sentuh

* **Sentuh 1x (Tap 1x):** Berpindah antar Menu (Home $\rightarrow$ Music $\rightarrow$ Maps $\rightarrow$ Weather $\rightarrow$ WhatsApp $\rightarrow$ Running Text $\rightarrow$ GIF Intro $\rightarrow$ Menu OTA).
* **Mode Home (Jam):**
  * **Tahan (>0.8 Detik):** Mengubah sub-mode Home (Tampilan Jam / Animasi GIF).
  * **Ketuk 2x:** Mengubah tema warna layar (Invert / Negative Display).
* **Mode Musik:**
  * **Ketuk 2x:** Mengganti lagu selanjutnya (Next).
  * **Ketuk 3x:** Mengulang lagu sebelumnya (Previous).
  * **Tahan (>0.8 Detik):** Play / Pause musik.
  * **Ketuk 4x:** Membuka Popup Pengatur Volume.
    * *Saat Popup Volume Aktif:*
      * **Ketuk 1x:** Menambah volume (+10%).
      * **Tahan (>0.8 Detik):** Menurunkan volume (-10%).
      * **Keluar Otomatis:** Popup akan tertutup otomatis setelah 5 detik tidak ada aktivitas (atau ketuk 4x lagi).
* **Mode Cuaca (Weather):**
  * Menampilkan informasi suhu (°C) dan nama kota secara *real-time* dengan tampilan penuh tanpa header jam. 
  * *Catatan:* Membutuhkan koneksi Bluetooth yang aktif ke aplikasi Chronos di HP serta izin GPS/Lokasi agar data cuaca dari internet dapat disinkronkan ke perangkat.
* **Mode WhatsApp:**
  * **Ketuk 3x:** Mengosongkan/menghapus tampilan pesan di layar.
* **Mode Running Text:**
  * **Tahan (>0.8 Detik):** Berganti tampilan antara Teks Berjalan dan Info Popup.
* **Mode OTA Update (Mode 7):**
  * **Ketuk 3x:** Mengaktifkan Access Point Wi-Fi OTA (`W-Smart Rider`) untuk pembaruan *firmware* secara nirkabel via browser HP.
  * **Ketuk 2x / Tahan:** Merestart ESP32.

---

## 🚀 Cara Instalasi Firmware

1. Unduh dan buka repositori ini di **Arduino IDE**.
2. Pastikan file `daichi_intro.h` berada dalam satu folder yang sama dengan `W-Smart-Rider.ino`.
3. Pilih board **ESP32C3 Dev Module** pada opsi Board Manager.
4. Lakukan *Upload* kode ke board ESP32-C3 Anda.

---

## ☕ Dukung & Donasi

Jika proyek *open-source* ini bermanfaat bagi Anda dan ingin mendukung pengembangan fitur-fitur selanjutnya, Anda dapat berdonasi melalui:

* **Bank BCA:** 4140364237 (a.n. Wahyu)

Dukungan sekecil apa pun sangat berarti untuk kemajuan proyek ini. Terima kasih!

---

## 📝 Lisensi & Penggunaan

Proyek ini dibuat open-source agar bisa dikembangkan dan digunakan oleh komunitas perakit/modifikator elektronik kendaraan. Silakan kembangkan lebih lanjut sesuai kebutuhan!
