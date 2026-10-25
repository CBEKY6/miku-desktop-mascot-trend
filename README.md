<img width="1220" height="677" alt="Без названия247_20261005113508" src="https://github.com/user-attachments/assets/f830497f-a618-4114-a20b-4a9deaeb7d68" />

# Hatsune Miku Desktop Mascot / Настольный маскот Хацунэ Мику

> That same Miku from the "slysh ty zalip, opa opa op" trend!
>
> *Та самая Мику из тренда «слышь ты залип, опа опа оп»!*
>
> [Читать на русском](README_RU.md)

---

## English Version

A desktop mascot for Linux written in pure C using the Raylib library. No Wallpaper Engine or Anima Engine required.

### Compatibility

- 100% working on: KDE Plasma

Likely to work on:

- GNOME
- XFCE
- Hyprland, Niri, Sway, i3

(To be honest, I haven't tested it outside of KDE Plasma, so good luck!)

## Dependencies

> For distros other than Arch Linux, package names were checked against Repology, but not tested on the actual systems.

### Arch / Manjaro / EndeavourOS
```bash
sudo pacman -S gcc raylib libx11
```

### Debian (testing/unstable) / Ubuntu / Mint / Pop!_OS
```bash
sudo apt update
sudo apt install build-essential libraylib-dev libx11-dev
```
If `libraylib-dev` is not found (Debian stable or older Ubuntu), see the "Building Raylib from source" section.

### Fedora
```bash
sudo dnf install gcc raylib-devel libX11-devel mesa-libGL-devel
```

### openSUSE Tumbleweed
```bash
sudo zypper install gcc raylib-devel libX11-devel Mesa-libGL-devel
```
Leap 15.6 ships an old raylib 4.5.0, so it's better to build from source.

### Alpine (the community repository must be enabled)
```bash
sudo apk add build-base raylib-dev libx11-dev mesa-dev
```

### Gentoo (raylib is in the GURU overlay)
```bash
sudo eselect repository enable guru
sudo emaint sync -r guru
sudo emerge media-libs/raylib x11-libs/libX11
```

### NixOS
```bash
nix-shell -p gcc raylib xorg.libX11 libGL
```

### Building Raylib from source (any distro)
If the required dependencies are not in your repositories, first install gcc, git, cmake, make, and the X11 and OpenGL dev packages. Then:
```bash
git clone https://github.com/raysan5/raylib.git
cd raylib && mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

## Building and Running

1. Clone the repository and enter the directory:
```bash
git clone https://github.com/CBEKY6/miku-desktop-mascot-trend.git
cd miku-desktop-mascot-trend
```

2. Compile:
```bash
gcc main.c -o miku -lraylib -lX11 -lGL -lm -lpthread -ldl -lrt
```

3. Run the mascot:
```bash
./miku
```

 
