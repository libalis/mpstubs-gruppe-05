#pragma once

#include "fs/blockdevice.h"
#include "fs/errno.h"
#include "machine/ioport.h"

enum Drive {
	primary_bus_master,
	primary_bus_slave,
	secondary_bus_master,
	secondary_bus_slave,
	third_bus_master,
	third_bus_slave,
	fourth_bus_master,
	fourth_bus_slave
};

class HarddiskPorts {
 private:
	static uint16_t base_port(Drive drive) {
		switch (drive) {
		case primary_bus_master:
		case primary_bus_slave:
			return 0x1F0;
		case secondary_bus_master:
		case secondary_bus_slave:
			return 0x170;
		case third_bus_master:
		case third_bus_slave:
			return 0x1E8;
		case fourth_bus_master:
		case fourth_bus_slave:
			return 0x168;
		}
		// bug
		return 0;
	}

 public:
	IOPort data;
	IOPort sector_count;
	IOPort lba_low;
	IOPort lba_mid;
	IOPort lba_high;
	IOPort drive_select;
	IOPort command;

	explicit HarddiskPorts(Drive drive)
		: data(base_port(drive)),
		  sector_count(base_port(drive) + 2),
		  lba_low(base_port(drive) + 3),
		  lba_mid(base_port(drive) + 4),
		  lba_high(base_port(drive) + 5),
		  drive_select(base_port(drive) + 6),
		  command(base_port(drive) + 7)
		{}
};

/**
 *
 * \brief Simpler ATA Treiber, der 28bit Adressierung und polling über IOPorts verwendet.
 *
 * Hinweis für block_number und sector:
 * Die block_number von fix() verwendet stets die Blockgröße, die über set_blocksize festgelegt wurde.
 * Intern entspricht ein Sektor 512 byte, da die IOPorts diese Adressierung verwenden.
 * block_number wird in read_sectors() oder write_sectors() per convert_lba() abhängig von this->blocksize umgewandelt.
 *
 * Siehe https://wiki.osdev.org/ATA_PIO_Mode
 */
class Harddisk: public BlockDevice {
 public:
	bool attached;
	uint32_t max_lba;

 private:
	Drive drive;
	HarddiskPorts ports;
	uint8_t last_drive_select_bits = 0;

	// status bits https://wiki.osdev.org/ATA_PIO_Mode#Status_Byte
	const uint8_t dma_request_bit = 0x8;   // DMA wird zwar nicht verwendet, zeigt aber an, ob bereit Daten zu übertragen
	const uint8_t error_bit       = 0x1;   // Fehler, ggf. Kommando nochmal senden
	const uint8_t drive_fault_bit = 0x20;  // error_bit wird nicht gleichzeitig gesetzt
	const uint8_t busy_bit        = 0x80;  // Festplatte bereitet Daten vor.
	                                       // Wenn gesetzt, sollten anderen Zustände ignoriert werden.
	const uint8_t error_bits = error_bit | drive_fault_bit;

 public:
	explicit Harddisk(Drive drive = primary_bus_master) : ports(drive) {
		this->drive = drive;

		attached = is_attached();
	}

	/* \brief Siehe BlockDevice::fix */
	Block fix(uint64_t block_number);
	/* \brief Siehe BlockDevice::unfix */
	void unfix(Block *block);
	/* \brief Siehe BlockDevice::sync */
	int sync();
	/* \brief Siehe BlockDevice::sync */
	int sync(Block *block);

 private:
	/* \brief Liest Sektoren mit 28bit PIO
	 *
	 * Siehe https://wiki.osdev.org/ATA_PIO_Mode#28_bit_PIO für Details
	 *
	 * \param lba logical block address. Ein Sektor hat die Größe von this->blocksize.
	 * \param sectors Anzahl an Sektoren, die ab \p lba gelesen werden. Wenn 0, werden 128kB gelesen (256 Sektoren mit Blockgröße von 512byte)
	 * \param dest_buf Zielspeicher, in den die gelesenen Daten geschrieben werden
	 *
	 * \return Im Fehlerfall negativer Wert aus errno.h, sonst 0.
	 */
	int read_sectors(uint64_t lba, uint8_t sectors, void *dest_buf);

	/**
	 *
	 * \brief Schreibt Sektoren mit 28bit PIO
	 *
	 * Sehr ähnlich zu read_sectors. Allerdings wird nach jedem Schreiben noch der Cache geleert.
	 *
	 * \param lba logical block address. Ein Sektor hat die Größe von this->blocksize.
	 * \param sectors Anzahl an Sektoren, die ab \p lba geschrieben werden. Wenn 0, werden 128kB geschrieben (256 Sektoren mit Blockgröße von 512byte)
	 * \param src_buf Speicheradresse, von der die Daten zum Schreiben gelesen werden
	 *
	 * \return Im Fehlerfall negativer Wert aus errno.h, sonst 0.
	 */
	int write_sectors(uint64_t lba, uint8_t sectors, void *src_buf);

	/**
	 *  \brief Blockiert bis Festplatte bereit ist.
	 *
	 * \param delay: Wenn true, wird das status register mehrmals gelesen, um der Festplatte genügend Zeit zum Aktualisieren der register zu geben.
	 *
	 *  \return false im Fehlerfall, sonst true.
	 */
	bool wait_for_harddisk(bool delay = false) const;

	/**
	 * \brief Identisch zu wait_for_harddisk – nur ohne Rückgabewert im Fehlerfall.
	 */
	void wait_until_bsy_clear(bool delay = false);

	/**
	 * \brief Selektiert die Festplatte, die nachfolgende Kommandos erhält.
	 *
	 * \param lba_start Nötig um die höchsten 4bits der 28bit-Adresse zu übermitteln.
	 */
	void drive_select(uint64_t lba_start = 0);

	/**
	 *  \brief Testet mit dem IDENTIFY-Kommando, ob diese Festplatte angeschlossen ist
	 *
	 *  Siehe https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
	 *
	 *  \return false, falls keine Festplatte an `drive` angeschlossen oder ein Fehler aufgetreten ist. Sonst true.
	 */
	bool is_attached();

	/**
	 * \brief Konvertiert die gegebene \p lba und \p sectors in deren Äquivalent mit einer Sektorgröße von 512 byte.
	 * Das ist notwendig, da dies die Adressierung der IOPorts ist.
	 *
	 * Das Ergebnis wird in die übergebenen Pointer geschrieben!
	 *
	 * Beispiel mit this->blocksize von 1024:
	 *  - eine lba von 1 wird zu 2
	 *  - sectors=2 wird zu sectors=4
	 *
	 * \return Im Fehlerfall negativer Wert aus errno.h, sonst 0.
	 */
	int convert_lba(uint64_t *lba, uint8_t *sectors);

	/**
	 * \brief Übermittelt via IOPort das Kommando \p command
	 *
	 * \param lba logical block address
	 * \param sectors Sektoren in 512 byte
	 * \param command zu übermittelnde Kommandobits
	 */
	void transmit_command(uint64_t lba, uint8_t sectors, uint8_t command) const;
};
