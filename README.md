<img width="1220" height="677" alt="Без названия247_20261005113508" src="https://github.com/user-attachments/assets/d4a60f81-9a78-4d10-873d-0384aa64311a" />



# Hatsune Miku Desktop Mascot / Настольный маскот Хацунэ Мику


> That same Miku from the "slysh ty zalip, opa opa op" trend!
> 
> *Та самая Мику из тренда «слышь ты залип, опа опа оп»!*
> 
> [Переход на Русскую версию](README_RU.md)

---

## English Version


Desktop mascot for Linux written in pure C using the Raylib library. No Wallpaper Engine or Anima Engine required.


### Compatibility

* 100% Working on: KDE Plasma
  
  Likely to work on:
  
  * GNOME
  * XFCE
  * Hyprland, Niri, Sway, i3
    

(To be honest, I haven't tested this outside of KDE Plasma, so good luck!)*


### Dependencies


* For Arch-based distros:
```bash

sudo pacman -S gcc make raylib libx11
```

* For Debian/Ubuntu-based distros:
```bash

sudo apt update
```
```bash

sudo apt install build-essential libraylib-dev libx11-dev
```


*Building and Running

 * Clone the repository and enter the directory:
   
```bash
 git clone https://github.com/CBEKY6/miku-desktop-mascot-trend.git
 ```
```bash
 cd miku-desktop-mascot-trend
```

 * Compile:
   ```bash
   make
   ```
   Or without `make`:
   ```bash
   gcc main.c widget.c -o miku -lraylib -lX11 -lGL -lm -lpthread -ldl -lrt
   ```
   `make` detects whether the X11 headers are installed and only then adds `-lX11`. Window
   manager hints (always on top, hidden from the taskbar and window switcher, no focus
   stealing, staying below fullscreen windows) need X11; clicks passing through to whatever is
   underneath work on both X11 and Wayland. Requires raylib 4.2 or newer.
 * Run the mascot:
   ```bash
   ./miku
```
