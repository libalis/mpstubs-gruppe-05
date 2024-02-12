# Minix V3 Dateisystemlayout

Auf einer Platte mit Minix V3 steht jeweils nacheinander

 1. Boot block (1024 Byte)
 2. Superblock (1024 Byte), siehe `struct Minix_Super_Block` in `minix/minix.h`
 3. Bitmap inodes
 4. Bitmap zonemap (Zonen sind hier genauso groß wie ein Block)
 5. Inodes, für den Aufbau einer Inode siehe `struct Minix_Disk_Inode` in `minix/minix.h`
 6. Daten in Blöcken. Bei `mkfs.minix` von Linux sind Blöcke jeweils 1024 Byte groß.
    * Ein Datenblock kann alternativ auch Verzeichniseinträge enthalten, siehe `struct Minix_Dirent` in `minix/minix.h`

# Minix V3 Dateisystemabbild erstellen

Unter einem Linux:

    $ dd if=/dev/zero of=~/file.img bs=1MiB count=1
    $ mkfs.minix -3 ~/file.img  # optional --inodes <number>
    352 Inodes
    1024 Blöcke
    Erste Datenzone=26 (26)
    Zonengröße=1024
    Maximalgröße=2147483647

## Zugriff als eingebundenes Dateisystem

Als Root unter Linux:

    $ losetup --find --show ~/file.img
    $ mount /dev/loop0 /mnt/tmp
    # Jetzt Dateien und Co anlegen
    $ umount /dev/loop0
    $ losetup --detach /dev/loop0

## Zugriff mit dem fstool

Da nicht immer Rootzugriff vorhanden ist (z.B. im CIP) gibt es eine Kommandozeilenanwendung, welche mit FTP-artigen Befehlen einen direkten Zugriff auf das Abbild erlaubt

    $ cd tool
    $ make
    $ fstool ~/file.img
    # Jetzt Dateien und Co bearbeiten.
    # "help" für weitere Informationen

# Dateisystemabbild in OOStuBS einbinden

## ATA Treiber
Ermöglicht das Schreiben aus OOStuBS auf das Abbild, so dass diese Daten dann in z.B. Linux später gelesen werden können.

In `common.mk` bei `QEMU` `-drive file=./test_imgs/minix_sample_data.img,format=raw` hinzufügen. Dann ist das Abbild – sollte es das Einzige sein – Master am ersten Port. Es reicht dann in der App

	#include "fs/harddisk.h"

	Harddisk harddisk;

	int error = VFS::mount("minix", &harddisk, "");

Möchte man einen anderen Port nutzen, muss man . Z.B. `-drive file=./test_imgs/minix_sample_data.img,format=raw,bus=1,unit=1` bei `QEMU` anhängen (siehe `man qemu`). Dementsprechend `Harddisk harddisk(secondary_bus_slave);` in der App verwenden. Alle weiteren unterstützten Ports finden sich in `enum Drive` unter `harddisk.h`.

## Initrd

Ermöglicht kein Schreiben. Dazu via `QEMUINITRD` Dateisystemabbild als initrd übergeben.

In der App

    #include "memory/multiboot.h"
    extern void *multiboot_addr;

    multiboot_info_t *mb_info = (multiboot_info_t*)multiboot_addr;
	if (mb_info->mods_count <= 0) {
		GuardedScheduler::exit();
	}
	multiboot_module_t *mod = (multiboot_module_t*)mb_info->mods_addr;
        void *initrd = (void *)mod->mod_start;
        size_t initrdsize = ((size_t)mod->mod_end) - ((size_t)mod->mod_start);
	Ramdisk ramdisk(initrd, initrdsize);
	int error = VFS::mount("minix", &ramdisk, "");

## Headerdatei (nicht empfohlen)

Ermöglicht kein Schreiben. Das Erstellen einer Headerdatei, wodurch das Abbild dann über eine Variable genutzt werden kann, geht folgendermaßen:

    xxd -i minix_mkfs.img > minix_img.h

In der App

	#include "minix_img.h"
	#include "fs/ramdisk.h"

    static Ramdisk ramdisk(test_imgs_minix_sample_data_img, test_imgs_minix_sample_data_img_len);

    int error = VFS::mount("minix", &ramdisk, "");

# Mit Dateien arbeiten

Einige Ordner- und Dateioperationen bietet das Virtualfilesystem (kurz VFS). Beispielsweise finden sich dort Operationen wie `VFS::open(…)`, `VFS::lseek(…)` oder `VFS::stat(…)`. Genauere Beschreibungen der Methoden finden sich in den Kommentaren von `vfs.h`.
