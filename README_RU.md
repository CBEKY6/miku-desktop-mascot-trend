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
   gcc main.c widget.c -o miku -lraylib -lX11 -lGL -lm -lpthread -ldl -lrt
   ```

   Проще через `make`, он сам определит наличие заголовков X11 и добавит `-lX11`, если они
   установлены:

   ```bash
   make
   ```

   Подсказки оконному менеджеру (поверх всех окон, скрыт из таскара и переключателя окон, не
   забирает фокус, не мешает полноэкранным окнам) требуют X11; а кликабельность насквозь — по
   клику под маскотом — работает и под X11, и под Wayland. Нужен raylib 4.2 или новее.


 * Запустите маскота:
  ```bash
   ./miku
   ```
