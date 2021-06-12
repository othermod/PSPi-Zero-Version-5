# PSPi Zero Version 5
![PSPi Zero](https://othermod.com/wp-content/uploads/IMG_8554.jpg)
 
Option 1 - Installation Instructions Using Image

Download the ready-to-use image from Google Drive (link coming soon)

Extract the image and use imaging software (such as Win32DiskImager or Etcher) to image your SD card

Option 2 - Manual Offline Installation Instructions

Download repository, extract and copy all subfolders to the BOOT partition of a fresh RetroPie image. You must overwrite the original config.txt

Boot the PSPi with the SD card inserted, with a USB keyboard attached.

After Emulation Station loads, press F4 on the keyboard to exit to the command line.

Type the following command to install everything:

sudo bash /boot/pspi/setup.sh

Only do this next step if the buttons aren't working. It's integrated into the button installation script, so it shouldn't be needed.

type sudo raspi-config

Select Interfacing Options

Select I2C

Select Yes and press enter

To-Do List:

