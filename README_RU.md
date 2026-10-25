<img width="1220" height="677" alt="Без названия247_20261005113508" src="https://github.com/user-attachments/assets/f830497f-a618-4114-a20b-4a9deaeb7d68" />



# Hatsune Miku Desktop Mascot / Настольный маскот Хацунэ Мику


> *Та самая Мику из тренда «слышь ты залип, опа опа оп»!*
> > That same Miku from the "slysh ty zalip, opa opa op" trend!
> 
> [Read in English](README.md)
> ---
## Русская версия

Маскот для рабочего стола Linux, написанный на чистом C с использованием библиотеки Raylib. Никакого Wallpaper Engine или Anima Engine не требуется.

Совместимость

 * 100% работает на: KDE Plasma
   
  Скорее всего, будет работать на:
 
   * GNOME
     
   * XFCE
     
   * Hyprland, Niri, Sway, i3
     
(Если честно, за пределами KDE Plasma я не тестировал, так что тут как повезёт)

## Зависимости

> Для дистрибутивов кроме Arch Linux имена пакетов сверены с Repology, но на самих системах не тестировались.

### Arch / Manjaro / EndeavourOS
```bash
sudo pacman -S gcc raylib libx11
```

### Debian (testing/unstable) / Ubuntu / Mint / Pop!_OS
```bash
sudo apt update
sudo apt install build-essential libraylib-dev libx11-dev
```
Если `libraylib-dev` не найден (Debian stable или Старые Ubuntu), смотри раздел «Сборка Raylib из исходников»

### Fedora
```bash
sudo dnf install gcc raylib-devel libX11-devel mesa-libGL-devel
```

### OpenSUSE Tumbleweed
```bash
sudo zypper install gcc raylib-devel libX11-devel Mesa-libGL-devel
```
В Leap 15.6 старая версия raylib 4.5.0, лучше собрать из исходников.

### Alpine (должен быть включён репозиторий community)
```bash
sudo apk add build-base raylib-dev libx11-dev mesa-dev
```

### Gentoo (raylib находится в оверлее GURU)
```bash
sudo eselect repository enable guru
sudo emaint sync -r guru
sudo emerge media-libs/raylib x11-libs/libX11
```

### NixOS
```bash
nix-shell -p gcc raylib xorg.libX11 libGL
```

### Сборка Raylib из исходников (любой дистрибутив)
Если нужных зависимостей нет в репозитории, скачайте перед этим gcc, git, cmake, make,dev-пакеты X11 и OpenGL,Потом:
```bash
git clone https://github.com/raysan5/raylib.git
cd raylib && mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```
* Сборка и запуск

 * Клонируйте репозиторий и перейдите в папку:
  ```bash
  git clone https://github.com/CBEKY6/miku-desktop-mascot-trend.git
   ```
   ```bash
   cd miku-desktop-mascot-trend
   ```
   * Скомпилируйте:
   ```bash
   gcc main.c -o miku -lraylib -lX11 -lGL -lm -lpthread -ldl -lrt
   ```


 * Запустите маскота:
  ```bash
   ./miku
   ```
