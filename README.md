# SimulIDE 2.0 STM32F103 Patch (Win64)

[🇹🇷 Türkçe](#-türkçe) | [🇬🇧 English](#-english) | [🙏 Original Project / Orijinal Proje](https://github.com/Arcachofo/SimulIDE-dev)

> [!NOTE]
> **Orijinal Proje / Original Project:** [**Arcachofo/SimulIDE-dev**](https://github.com/Arcachofo/SimulIDE-dev) (by Santiago González — [`qemu_simulide`](https://github.com/Arcachofo/qemu_simulide))  
> Bu depo **yalnızca saf STM32 ek yamasını (~464 KB)** içerir; resmi `SimulIDE 2.0 (R260501 Win64)` ikili dosyalarını veya herhangi bir özel projeye ait firmware dosyasını içermez.  
> This repository contains **only the pure STM32 add-on patch (~464 KB)** — no upstream `SimulIDE` binaries and no private project firmware are redistributed here.

---

## 🇹🇷 Türkçe

### 1. Bu Yama Nedir?

Standart **SimulIDE 2.0 (`SimulIDE_2-R260501_Win64`)** sürümü yalnızca ESP32 QEMU arka ucunu (`qemu-system-xtensa.exe`) içerir; STM32 bileşen tanımları, pin paketleri ve ARM QEMU köprüsü paket içinde yer almaz.

Bu yama, resmi `SimulIDE_2-R260501_Win64` kurulumuna doğrudan **STM32F101 / STM32F103 (`QemuDevice`)** desteğini kazandırır:

* **Özel ARM QEMU Paylaşımlı Bellek Köprüsü (`data/bin/qemu-system-arm.exe`):** Yalnızca standart Windows sistem kütüphanelerini (`KERNEL32.dll`, `msvcrt.dll`) kullanan `100 KB`'lık bağımsız emülasyon köprüsü.
* **Kütüphane ve Paket Tanımları (`data/stm32.xml`, `data/STM32/*.package`):**
  * `STM32F103C8`, `STM32F103CB` (`LQFP48`, 64/128 KB Flash, 20 KB SRAM)
  * `STM32F103RB`, `STM32F103RC` (`LQFP64`, 128/256 KB Flash, 20/48 KB SRAM)
  * `STM32F103VC`, `STM32F103VE` (`LQFP100`, 256/512 KB Flash, 48/64 KB SRAM)
  * `STM32F103T6`, `STM32F103T8` (`QFN36`, 32/64 KB Flash, 10/20 KB SRAM)
  * **Pin ID Düzeltmesi Dahil:** `stm32_dip48.package` (ve familya paketleri) sıfır dolgulu pin kimlikleriyle (`PD00..PD01`, `PA00..PA09`, `PB00..PB09`) güncellenmiştir; böylece 48 pinin tamamı eksiksiz görünür ve `PB8/PB9` I2C hatları çakışmaz.
* **Desteklenen Çevre Birimleri ([`data/help/stm32.txt`](data/help/stm32.txt)):**
  * **GPIO:** Atomik `BSRR/BRR` ve pin yönü yapılandırması
  * **EXTI / AFIO:** Tüm pinlerde harici kesme ve `MAPR` yeniden yönlendirme desteği
  * **USART:** `115200` baud'a kadar seri haberleşme
  * **I2C:** `I2C1` master/slave (EEPROM, `SSD1306` OLED vb.)
  * **Timers:** `TIM1`, `TIM2`, `TIM3`, `TIM4` donanım PWM üretimi
  * **Firmware Formatı:** Ham `.bin` (`0x00000000` SP, `0x00000004` `Reset_Handler`)
* **Hazır Örnek Projeler (`examples/STM32/`):** `blink`, `button`, `i2c_eeprom`, `uart_echo` ve `stm32f103.ld` linker scripti.

### 2. Kurulum

1. Orijinal projeden ([Arcachofo/SimulIDE-dev](https://github.com/Arcachofo/SimulIDE-dev)) resmi **SimulIDE 2.0 (`SimulIDE_2-R260501_Win64`)** paketini indirin ve çıkarın.
2. Bu depoyu indirin veya klonlayın:
   ```bash
   git clone https://github.com/kurekciyusuf/SimulIDE-STM32F103-Patch-v1.0.0.git
   ```
3. **Otomatik Kurulum:** [`install_stm32_patch.bat`](install_stm32_patch.bat) dosyasını çalıştırıp SimulIDE klasör yolunu verin:
   ```bat
   install_stm32_patch.bat "C:\SimulIDE_2-R260501_Win64"
   ```
   *(Alternatif Manuel Kurulum: Bu depodaki `data` ve `examples` klasörlerini doğrudan `SimulIDE_2-R260501_Win64` klasörünün üzerine kopyalayın.)*
4. `simulide.exe` uygulamasını açın; sol menüde **Micro → STM32** kategorisinin geldiğini göreceksiniz. İlk test için `examples/STM32/blink/blink.sim2` dosyasını açıp simülasyonu başlatabilirsiniz.

> [!TIP]
> Yoğun zamanlayıcı / ekran kullanan firmware'lerde QEMU ile gerçek zamanlı eşzamanlama kararlılığı için `.sim2` devresindeki `stepsPS` hızını (ör. `stepsPS="30000"`) düşürmeniz önerilir.

---

## 🇬🇧 English

### 1. Overview

Stock **SimulIDE 2.0 (`SimulIDE_2-R260501_Win64`)** only ships with the ESP32 QEMU backend (`qemu-system-xtensa.exe`) and omits STM32 component definitions, pin packages, and the ARM QEMU backend.

This patch enables **STM32F101 / STM32F103 (`QemuDevice`)** simulation directly on top of an official `SimulIDE_2-R260501_Win64` installation:

* **Lightweight ARM QEMU Shared-Memory Bridge (`data/bin/qemu-system-arm.exe`):** Standalone `100 KB` executable requiring only standard Windows system DLLs (`KERNEL32.dll`, `msvcrt.dll`).
* **Device Library & Fixed Packages (`data/stm32.xml`, `data/STM32/*.package`):**
  * `STM32F103C8`, `STM32F103CB` (`LQFP48`, 64/128 KB Flash, 20 KB SRAM)
  * `STM32F103RB`, `STM32F103RC` (`LQFP64`, 128/256 KB Flash, 20/48 KB SRAM)
  * `STM32F103VC`, `STM32F103VE` (`LQFP100`, 256/512 KB Flash, 48/64 KB SRAM)
  * `STM32F103T6`, `STM32F103T8` (`QFN36`, 32/64 KB Flash, 10/20 KB SRAM)
  * **Fixed 48-Pin Package IDs:** Includes zero-padded IDs (`PD00..PD01`, `PA00..PA09`, `PB00..PB09`) in `stm32_dip48.package` so all 48 pins and `PB8/PB9` I2C lines map properly with no missing pins.
* **Supported Peripherals ([`data/help/stm32.txt`](data/help/stm32.txt)):**
  * **GPIO:** Full 16-bit ports with atomic `BSRR/BRR`
  * **EXTI / AFIO:** External interrupts on any pin with `MAPR` remapping
  * **USART:** Up to `115200` baud with Serial Monitor
  * **I2C:** `I2C1` master/slave (EEPROMs, `SSD1306` OLED, etc.)
  * **Timers:** Hardware PWM on `TIM1–TIM4` channels
  * **Firmware Format:** Raw `.bin` (`0x00000000` stack pointer, `0x00000004` `Reset_Handler`)
* **Included Examples (`examples/STM32/`):** `blink`, `button`, `i2c_eeprom`, `uart_echo`, and `stm32f103.ld`.

### 2. Installation

1. Download and extract official **SimulIDE 2.0 (`SimulIDE_2-R260501_Win64`)** from the upstream repository ([Arcachofo/SimulIDE-dev](https://github.com/Arcachofo/SimulIDE-dev)).
2. Clone or download this repository:
   ```bash
   git clone https://github.com/kurekciyusuf/SimulIDE-STM32F103-Patch-v1.0.0.git
   ```
3. **Automated Install:** Run [`install_stm32_patch.bat`](install_stm32_patch.bat) targeting your SimulIDE folder:
   ```bat
   install_stm32_patch.bat "C:\SimulIDE_2-R260501_Win64"
   ```
   *(Manual option: Copy the `data` and `examples` folders from this repo directly into your `SimulIDE_2-R260501_Win64` directory.)*
4. Launch `simulide.exe`. **Micro → STM32** will appear in the component tree, and you can test immediately with `examples/STM32/blink/blink.sim2`.

---

### 📁 Repository Structure / Depo Yapısı

```text
├── data/
│   ├── bin/qemu-system-arm.exe    # Shared-memory ARM QEMU bridge for SimulIDE
│   ├── help/stm32.txt             # STM32 component documentation
│   ├── STM32/stm32_*.package      # Fixed DIP/LQFP/QFN package definitions
│   └── stm32.xml                  # Component library catalog entry
├── examples/STM32/
│   ├── blink/
│   ├── button/
│   ├── i2c_eeprom/
│   ├── uart_echo/
│   └── stm32f103.ld
├── install_stm32_patch.bat        # One-step Windows installer script
├── .gitattributes
├── .gitignore
├── LICENSE                        # GNU General Public License v3 (GPL-3.0)
└── README.md
```

---

### 🙏 Orijinal Projeye Atıf ve Lisans / Attribution & License

Bu çalışma, **Santiago González (`Arcachofo`)** tarafından geliştirilen açık kaynaklı **SimulIDE 2.0** (`SimulIDE_2-R260501`) üzerine kurulmuş bir ek yama paketidir.  
This work is an add-on patch built for the open-source **SimulIDE 2.0** (`SimulIDE_2-R260501`) project created by **Santiago González (`Arcachofo`)**.

* **Upstream SimulIDE Repository:** [Arcachofo/SimulIDE-dev](https://github.com/Arcachofo/SimulIDE-dev) — `Copyright (C) 2012–2025 Santiago González`
* **Upstream QEMU Fork:** [Arcachofo/qemu_simulide](https://github.com/Arcachofo/qemu_simulide)
* **Licenses / Lisanslar:**
  * Bu yama deposu, SimulIDE `copyright.txt` (üçüncü taraf katkılar için **GPL-3.0**) ve `examples/STM32` (`GPL-2.0-or-later`) ile uyumlu olarak **GNU General Public License v3 ([`LICENSE`](LICENSE))** altında sunulmuştur.
  * This patch repository is distributed under **GNU General Public License v3 ([`LICENSE`](LICENSE))**, matching SimulIDE's `copyright.txt` (`GPL-3.0` for third-party contributions) and `examples/STM32` (`GPL-2.0-or-later`).
  * SimulIDE 2.0 core is licensed under **GNU Affero General Public License v3 (AGPL-3.0)**; `qemu_simulide` is licensed under **GNU General Public License v2 (GPL-2.0)**.
* **Disclaimer / Kapsam Notu:** Bu depo yalnızca resmi `SimulIDE_2-R260501_Win64` dağıtımı üzerine STM32 bileşenlerini/örneklerini ve paylaşımlı bellek köprüsünü ekler; resmi `simulide.exe` ikilisi burada yeniden dağıtılmaz (`This repository only provides the STM32 add-on patch; the upstream simulide.exe binary is not redistributed and all upstream rights remain with the original author`).


