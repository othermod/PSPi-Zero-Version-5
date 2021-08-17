# PSPi Zero Version 5
![PSPi Zero](https://othermod.com/wp-content/uploads/IMG_8727.jpg)
 
### Option 1 - Installation Instructions Using Image

Download the ready-to-use image from [Google Drive](https://drive.google.com/file/d/1LdgOSTCEvPwvksIEMmHCxauEN8INBU-z/view?usp=sharing) or Torrent Magnet Link at the bottom of the page.

Extract the image and use imaging software (such as Win32DiskImager, Etcher, or Raspberry Pi Imager) to image your SD card

### Option 2 - Manual Offline Installation Instructions

Download repository, extract and copy the PSPi folder and config.txt file to the BOOT partition of a fresh RetroPie microSD (v4.7.1 currently). You must overwrite the original config.txt file from the RetroPie image.

Next, copy the appropriate lcd.txt file (from the 480x272 or 800x480 folder depending on which LCD you are using) to the BOOT partition. This goes into the same BOOT folder as the config.txt file

Boot the PSPi with the SD card inserted, with a USB keyboard attached.

After Emulation Station loads, press F4 on the keyboard to exit to the command line.

Type the following command to install everything:
```
sudo bash /boot/PSPi/setup.sh
```
The system will reboot after installation completes.



Torrent Magnet Link

magnet:?xt=urn:btih:a9f6d58c56663d980b317419adf9a53b9920c7f7&dn=PSPi%20Zero%20v5%208.17.21.7z&tr=udp%3a%2f%2ftracker.opentrackr.org%3a1337%2fannounce&tr=udp%3a%2f%2ftracker.openbittorrent.com%3a1337%2fannounce
