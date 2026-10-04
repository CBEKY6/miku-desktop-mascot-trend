# Hatsune Miku Desktop Mascot / Настольный маскот Хацунэ Мику


> That same Miku from the "slysh ty zalip, opa opa op" trend!
> 
> *Та самая Мику из тренда «слышь ты залип, опа опа оп»!*
> 

---

## English Version


Desktop mascot for Linux written in pure C using the Raylib library. No Wallpaper Engine or Anima Engine required.


### Compatibility

* 100% Working on: KDE Plasma
* 
* Likely to work on:
* 
  * GNOME
  * XFCE
  * Hyprland, Niri, Sway, i3
  * 

*(To be honest, I haven't tested this outside of KDE Plasma, so good luck!)*


### Dependencies


For Arch-based distros:

sudo pacman -S gcc make raylib libx11

For Debian/Ubuntu-based distros:

sudo apt update

sudo apt install build-essential libraylib-dev libx11-dev


Building and Running

 * Clone the repository and enter the directory:
 * 
 *git clone https://github.com/CBEKY6/miku-desktop-mascot-trend.git
 *cd miku-desktop-mascot-trend


 * Compile the project using Makefile:
 * 
   make
   

 * Run the mascot:
 * 
   ./miku
