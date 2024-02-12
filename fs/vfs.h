#pragma once

#include "types.h"
#include "fs/filesystem.h"
#include "fs/blockdevice.h"
#include "fs/file_descriptor_table.h"

/*! \brief Virtual File System - POSIX-ähnliche Dateisystem-Schnittstelle
 *
 *  Alle Funktionen geben im Fehlerfall eine negative errno zurück.
 *
 *  Limitierungen:
 *  -Zeitstempel werden nie aktualisiert
 *  -Zugriffsrechte werden nie überprüft
 *  -setzt die UID und GID von neu erstellte Dateien auf 0 (root)
 *  -setzt die Zeitstempel von neu erstellen Dateien auf 0 (1.1.1970)
 *  -Dateideskriptoren und das aktuelle Arbeitsverzeichnis sind global
 */
class VFS {
 private:
	static Filesystem *root_fs;
	static FD_Table fd_table;
	static Inode *global_cwd;  // current working directory

 public:
	/*! \brief Siehe man 2 mount.
	 *
	 *  Limitierungen:
	 *  -target (siehe manpage) ist immer "/".
	 *  -es werden keine Flags unterstützt
	 *  -an Stelle des source Strings wird ein BlockDevice übergeben
	 */
	static int mount(const char *fstype, BlockDevice *bdev, const void *data);

	/*! \brief Siehe man 2 umount.
	 *
	 *  Gibt -EBUSY zurück, falls das Dateisystem noch verwendet wird (offene Dateien).
	 *  Limitierungen:
	 *  -target (siehe manpage) ist immer "/".
	 */
	static int umount();

	/*! \brief Schreibt Änderungen im Puffer auf die Festplatte. Siehe man 2 sync.
	 *
	 *  Zurzeit nur nötig, um Änderungen an Inodes zu schreiben.
	 */
	static void sync();

	/*! \brief Öffnet und erstellt (je nach flags) Dateien. Siehe man 2 open.
	 *
	 *  Limitierungen:
	 *  -mode ist immer 0777
	 *  -unterstützte Flags sind: O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_EXCL
	 */
	static int open(const char *pathname, int flags);

	/*! \brief Schließt einen Dateideskriptor. Siehe man 2 close.
	 *
	 *  \param fd Filedeskriptor zum Schließen.
	 *
	 *  Es gelten die "NOTES" aus der manpage.
	 */
	static int close(int fd);

	/*! \brief Lesen von einem Dateideskriptor \p fd in Speicher \p buf der Länge
	 *  \p count. Siehe man 2 read. */
	static ssize_t read(int fd, void *buf, size_t count);

	/*! \brief Schreiben von Daten aus Speicher \p buf der Länge
	 *  \p count an Dateideskriptor \p fd. Siehe man 2 write */
	static ssize_t write(int fd, const void *buf, size_t count);

	/*! \brief Ändert den aktuellen \p offset für Dateideskriptor \p fd. Siehe man 2 lseek.
	 *
	 *  Unterstützt kein SEEK_DATA oder SEEK_HOLE.
	 *  (Mit lseek(fd, 0, SEEK_END) lässt sich die Größe einer Datei herausfinden.)
	 */
	static off_t lseek(int fd, off_t offset, int whence);

	/*! \brief Setzt die Länge der Datei an \p path auf \p length. Siehe man 2 truncate. */
	static int truncate(const char *path, off_t length);

	/*! \brief Kürzt Datei für Dateideskriptor \p fd auf Länge \p length. Siehe man 2 ftruncate. */
	static int ftruncate(int fd, off_t length);

	/*! \brief Neuer Name \p newpath für Datei \p oldpath. Siehe man 2 link.
	 *
	 *  Falls oldpath ein Symlink ist wird ein Hardlink auf den Symlink erstellt,
	 *  nicht auf die Datei auf die der Symlink zeigt. (Selbes Verhalten wie Linux,
	 *  siehe NOTES in der Manpage.)
	 */
	static int link(const char *oldpath, const char *newpath);

	/*! \brief Erstellt einen symbolischen Verweis mit Inhalt target. Siehe man 2 symlink. */
	static int symlink(const char *target, const char *linkpath);

	/*! \brief Löscht Namen und bei keinem verbleibenden Verweis mehr die Datei \p pathname. Siehe man 2 unlink. */
	static int unlink(const char *pathname);

	/*! \brief Löscht Verzeichnis \p pathname. Siehe man 2 rmdir. */
	static int rmdir(const char *pathname);

	/*! \brief Verschieben von Dateien/Verzeichnissen. Siehe man 2 rename. */
	static int rename(const char *oldpath, const char *newpath);

	/*! \brief Liefert Dateistatus für \p pathname. Siehe man 2 stat.
	 *
	 *  Limitierungen:
	 *  -st_dev und st_rdev sind immer 0
	 */
	static int stat(const char *pathname, struct stat *statbuf);

	/*! \brief Liefert Dateistatus für \p pathname. Siehe man 2 lstat.
	 *
	 *  Es gelten dieselben Limitierungen wie für stat.
	 *  Falls pathname ein Symlink ist liefert lstat im Gegensatz zu stat Informationen
	 *  über den Symlink, nicht über die Datei auf die der Symlink zeigt.
	 */
	static int lstat(const char *pathname, struct stat *statbuf);

	/*! \brief Liefert Dateistatus für Dateideskriptor \p fd. Siehe man 2 fstat.
	 *
	 *  Es gelten dieselben Limitierungen wie für stat.
	 */
	static int fstat(int fd, struct stat *statbuf);

	/*! \brief Liest den Inhalt eines symbolischen Verweises. Siehe man 2 readlink.
	 *
	 *  Achtung: Der Buffer wird nicht null-terminiert.
	 */
	static ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);

	/*! \brief Liefert Verzeichniseinträge. Siehe man 2 getdents.
	 *
	 *  Wird verwendet um readdir zu implementieren.
	 *  Das Dirent struct ähnelt dem linux_dirent64 struct aus der manpage,
	 *  allerdings sind d_ino und d_off nicht unbedingt 64-bit Werte.
	 *  Limitierungen:
	 *  -d_off ist immer 0
	 */
	static int getdents(int fd, Dirent *dirp, int count);

	/*! \brief Öffnet ein Verzeichnis \p name. Siehe man 3 opendir. */
	static DIR * opendir(const char *name);

	/*! \brief Liest einen Verzeichniseintrag. Siehe man 3 readdir. */
	static struct Dirent * readdir(DIR *dirp);

	/*! \brief Setzt die Position im Verzeichnis zurück. Siehe man 3 rewinddir. */
	static void rewinddir(DIR *dirp);

	/*! \brief Schließt ein Verzeichnis. Siehe man 3 closedir. */
	static int closedir(DIR *dirp);

	/*! \brief Erstellt Verzeichnis \p pathname. Siehe man 2 mkdir.
	 *
	 *  Limitierungen:
	 *  -pathname darf nicht mit einem '/' enden.
	 */
	static int mkdir(const char *pathname);

	/*! \brief Wechselt in Verzeichnis \p path. Siehe man 2 chdir. */
	static int chdir(const char *path);

	/*! \brief Wechselt in Verzeichnis \p fd. Siehe man 2 fchdir. */
	static int fchdir(int fd);

 private:
	static int pathwalk_step1(struct path *path, const char *pathname, Inode *cwd);
	static int pathwalk_step2(struct path *path, int depth);
	static Inode *pathwalk_step3(struct path *path, bool follow_symlink,
	                             int depth, int *error);
	static inline int pathwalk_step12(struct path *path, const char *pathname, Inode *cwd,
	                                  int depth);
	static inline Inode *pathwalk_step23(struct path *path, bool follow_final_symlink,
	                                     int depth, int *error);
	static inline Inode *pathwalk_step123(const char *pathname, Inode *cwd,
	                                      bool follow_final_symlink, int depth, int *error);
	static inline Inode *resolve_symlink(Inode *symlink, Inode *cur_dir, int depth, int *error);
	static int stat(Inode *inode, struct stat *statbuf);
	static int chdir(Inode *inode);
	static int truncate(Inode *inode, off_t length);
	static int sync_fs(Filesystem *fs);
};
