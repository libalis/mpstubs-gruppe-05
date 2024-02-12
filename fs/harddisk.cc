#include "fs/harddisk.h"

#include "utils/alloc.h"

#include "debug/output.h"

//
// Public
//

Block Harddisk::fix(uint64_t block_number) {
	Block block(this, block_number);

	void *dest_buf = malloc(blocksize);
	if (dest_buf == nullptr) {
		block.flags = -ENOMEM;
		return block;
	}

	int retval = read_sectors(block_number, 1, dest_buf);
	if (retval != 0) {
		block.flags = retval;
		free(dest_buf);
		return block;
	}

	block.data = dest_buf;
	return block;
}

void Harddisk::unfix(Block *block) {
	if(!block->is_dirty()) {
		free(block->data);
		block->data = nullptr;
		return;
	}
	// dirty → write block to disk

	sync(block);  // ignore errors is intended
	free(block->data);
	block->data = nullptr;
}

int Harddisk::sync() {
	// no caching, so no op
	return 0;
}

int Harddisk::sync(Block *block) {
	if(!block->is_dirty()) {
		return 0;
	}

	int error = write_sectors(block->block_number, 1, block->data);
	if (error != 0) {
		return error;
	}

	block->clear_dirty();

	return 0;
}

//
// Private
//

int Harddisk::read_sectors(uint64_t lba, uint8_t sectors, void *dest_buf) {
	uint64_t lba_start = lba;
	uint8_t sectors_512_byte = sectors;
	int error = convert_lba(&lba_start, &sectors_512_byte);
	if (error != 0) {
		return error;
	}
#ifdef VERBOSE
	DBG_VERBOSE << "Harddisk::read_sectors: lba_start "
	            << (long unsigned int)lba_start
	            << " sectors " << sectors << endl;
#endif
	drive_select(lba_start);

	const uint8_t read_command = 0x20;
	transmit_command(lba_start, sectors_512_byte, read_command);

	if (!wait_for_harddisk()) {
#ifdef VERBOSE
		DBG_VERBOSE << "Harddisk::read_sectors 3rd waiting failed" << endl;
#endif
		return -EIO;
	}

	// receive data
	uint16_t *dest_buf_u16 = reinterpret_cast<uint16_t *>(dest_buf);
	// with block_factor=1, the for reads 256 words = 1 sector = 512 byte
	for (unsigned int i = 0; i < 256 * sectors_512_byte; i++) {
		dest_buf_u16[i] = ports.data.inw();

		wait_until_bsy_clear();
	}

	if (!wait_for_harddisk(true)) {
		return -EIO;
	}

	return 0;
}

int Harddisk::write_sectors(uint64_t lba, uint8_t sectors, void *src_buf) {
	uint64_t lba_start = lba;
	uint8_t sectors_512_byte = sectors;
	int error = convert_lba(&lba_start, &sectors_512_byte);
	if (error != 0) {
		return error;
	}
#ifdef VERBOSE
	DBG_VERBOSE << "Harddisk::write_sectors: lba_start "
	            << (long unsigned int)lba_start
	            << " sectors " << sectors << endl;
#endif
	drive_select(lba_start);

	const uint8_t write_command = 0x30;
	transmit_command(lba_start, sectors_512_byte, write_command);

	// write data
	uint16_t *src_buf_u16 = reinterpret_cast<uint16_t *>(src_buf);
	// with block_factor=1, the for writes 256 words = 1 sector = 512 byte
	for (unsigned int i = 0; i < 256 * sectors_512_byte; i++) {
		ports.data.outw(src_buf_u16[i]);

		wait_until_bsy_clear();  // „delay“
	}

	const uint8_t flush_cache_command = 0xE7;
	ports.command.outb(flush_cache_command);
	wait_until_bsy_clear();

	return 0;
}

bool Harddisk::wait_for_harddisk(bool delay) const {
	if (delay) {
		ports.command.inb();
		ports.command.inb();
		ports.command.inb();
		ports.command.inb();
	}

	bool bsy_set;
	bool drq_unset;
	do {
		uint8_t ret = ports.command.inb();

		bsy_set = (ret & busy_bit) != 0;
		if (!bsy_set && (ret & error_bits) != 0) {
			// Fehler nur beachten, sobald Festplatte untätig
			return false;
		}
		drq_unset = (ret & dma_request_bit) == 0;
	} while (bsy_set && drq_unset);

	return true;
}

void Harddisk::wait_until_bsy_clear(bool delay) {
	wait_for_harddisk(delay);
}

void Harddisk::drive_select(uint64_t lba_start) {
	const uint8_t lba_mode_bit = 0x40;  // otherwise CHS mode
	const uint8_t legacy_bits = 0xA0;

	uint8_t drive_bits;
	switch(drive) {
		case primary_bus_master:
		case secondary_bus_master:
		case third_bus_master:
		case fourth_bus_master:
			// master disk
			drive_bits = 0x0;
			break;
		default:
			// slave
			drive_bits = 0x10;
			break;
	}

	uint8_t lba_highest_4bits = (lba_start >> 24) & 0xF;
	uint8_t new_drive_select_bits = drive_bits | lba_mode_bit | legacy_bits | lba_highest_4bits;

	if (new_drive_select_bits == last_drive_select_bits) {
		return;
	}

	wait_until_bsy_clear(true);
	ports.drive_select.outb(new_drive_select_bits);
	last_drive_select_bits = new_drive_select_bits;
}

bool Harddisk::is_attached() {
	drive_select();

	const uint8_t identify_command = 0xEC;
	transmit_command(0, 0, identify_command);

	if (ports.command.inb() == 0) {
		return false;
	}

	wait_until_bsy_clear();

	if (ports.lba_mid.inb() != 0 && ports.lba_high.inb() != 0) {
		// no ATA drive
		return false;
	}

	if (!wait_for_harddisk()) {
		return false;
	}

	for (unsigned int i = 0; i < 256; i++) {
		uint16_t buf = ports.data.inw();

		if (i == 60) {
			max_lba = buf;
		} else if (i == 61) {
			max_lba |= (buf << 16);
		}
	}

	return true;
}

int Harddisk::convert_lba(uint64_t *lba, uint8_t *sectors) {
	unsigned int block_factor = blocksize / 512;
	*lba *= block_factor;

	if (*sectors * block_factor > 255) {
		return -EINVAL;
	}
	*sectors *= block_factor;

	if (*lba > max_lba || *lba + *sectors > max_lba) {
		return -EINVAL;
	}
	*lba &= 0xfffffff;  // truncate all bits after bit 28

	return 0;
}

void Harddisk::transmit_command(uint64_t lba, uint8_t sectors, uint8_t command) const {
	ports.sector_count.outb(sectors);
	ports.lba_low.outb(static_cast<uint8_t>(lba));
	ports.lba_mid.outb(static_cast<uint8_t>(lba >> 8));
	ports.lba_high.outb(static_cast<uint8_t>(lba >> 16));

	// no polling here needed, as it's always waited until BSY clears in read/write

	ports.command.outb(command);
}
