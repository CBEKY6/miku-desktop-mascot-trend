Русская версия

Маскот для рабочего стола Linux, написанный на чистом C с использованием библиотеки Raylib. Никакого Wallpaper Engine или Anima Engine не требуется.

Совместимость

 * 100% работает на: KDE Plasma
 * 
 * Скорее всего, будет работать на:
 * 
   * GNOME
   * 
   * XFCE
   * 
   * Hyprland, Niri, Sway, i3
   * 
(Если честно, за пределами KDE Plasma я не тестировал, так что тут как повезёт)

Зависимости

Для Arch-подобных дистрибутивов:

sudo pacman -S gcc make raylib libx11


Для Debian/Ubuntu-подобных дистрибутивов:

sudo apt update

sudo apt install build-essential libraylib-dev libx11-dev


Сборка и запуск

 * Клонируйте репозиторий и перейдите в папку:
 * 
   git clone [https://github.com/CBEKY6/miku-desktop-mascot-trend.git](https://github.com/CBEKY6/miku-desktop-mascot-trend.git)
   
   cd miku-desktop-mascot-trend
   

 * Скомпилируйте проект с помощью Makefile:
 * 
   make
   

 * Запустите маскота:
 * 
   ./miku
   
