Русская версия

Маскот для рабочего стола Linux, написанный на чистом C с использованием библиотеки Raylib. Никакого Wallpaper Engine или Anima Engine не требуется.

Совместимость

 * 100% работает на: KDE Plasma
   
  Скорее всего, будет работать на:
 
   * GNOME
     
   * XFCE
     
   * Hyprland, Niri, Sway, i3
     
(Если честно, за пределами KDE Plasma я не тестировал, так что тут как повезёт)

Зависимости

* Для Arch-подобных дистрибутивов:
```bash
sudo pacman -S gcc raylib libx11
```

* Для Debian/Ubuntu-подобных дистрибутивов:
```bash
sudo apt update
```
```bash
sudo apt install build-essential libraylib-dev libx11-dev

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
