from enum import Enum
from . import helper

class Arch(Enum):
    X86 = 0
    X86_64 = 1

class PageLevel(Enum):
    PML4 = 0
    DirectoryPtr = 1
    Directory = 2
    Table = 3

class PageTableEntry:
    def __init__(self, base, idx, mmu, value):
        self.base = base
        self.idx = idx
        self.raw = value
        self.faulty = False

        self.present = helper.bits(value, 0, 1)
        self.rw = helper.bits(value, 1, 1)
        self.us = helper.bits(value, 2, 1)
        self.pwt = helper.bits(value, 3, 1)
        self.pcd = helper.bits(value, 4, 1)
        self.a = helper.bits(value, 5, 1)
        self.ps = helper.bits(value, 7, 1)

        self.r = helper.bits(value, 11, 1)
        self.xd = helper.bits(value, 63, 1)
        self.reference = helper.bits(value, 12, mmu.M)
        self.rsvd = helper.bits(value, mmu.M, 64 - mmu.M)
    pass

    def addr(self):
        return self.reference << 12

    def name(self):
        raise Exception("must be implemented by inheriting class")

    def description(self):
        raise Exception("must be implemented by inheriting class")

    def __str__(self):
        return f"<{self.name()}> {self.description()}"

class PML4Entry(PageTableEntry):
    def __init__(self, base, idx, mmu, value):
        super().__init__(base, idx, mmu, value)
        if (self.ps != 0):
            self.faulty = True
    pass

    def name(self):
        return f"PML4 (0x{self.base:x}[0x{self.idx:x}])"

    def description(self):
        if not self.present:
            return f"[p:0|raw: {self.raw:x}]"
        return "[" + str.join('|', [
            "p:1", f"rw:{self.rw}", f"us:{self.us}", f"pwt:{self.pwt}",
            f"pcd:{self.pcd}", f"a:{self.a}", f"rsvd:{self.ps}",
            f"r:{self.r}", f"addr:0x{self.reference:x}",
            f"rsvd:{self.rsvd}"
        ]) + f"] = {self.raw:x}"
        pass

class PDPEntry(PageTableEntry):

    def __init__(self, base, idx, mmu, value):
        super().__init__(base, idx, mmu, value)
        self.d = helper.bits(value, 6, 1)
        self.g = helper.bits(value, 8, 1)
        self.pat = helper.bits(value, 12, 1)
        self.xd = helper.bits(value, 63, 1)
        if self.ps == 1:
            self.reference = helper.bits(value, 30, mmu.M - (30 - 12))

    def name(self):
        return f"PDP  (0x{self.base:x}[0x{self.idx:x}])"

    def addr(self):
        if self.ps == 1:
            return self.reference << (12 + 18)
        return self.reference << 12

    def description(self):
        if not self.present:
            return f"[p:0|raw: {self.raw:x}]"
        return "[" + str.join('|', [
            "p:1", f"rw:{self.rw}", f"us:{self.us}", f"pwt:{self.pwt}",
            f"pcd:{self.pcd}", f"a:{self.a}", f"d:{self.d}", f"ps:{self.ps}",
            f"g:{self.g}", f"r:{self.r}", f"addr:0x{self.reference:x}",
            f"rsvd:{self.rsvd}"
        ]) + f"] = {self.raw:x}"
        pass

class PDEntry(PageTableEntry):

    def __init__(self, base, idx, mmu, value):
        super().__init__(base, idx, mmu, value)
        self.d = helper.bits(value, 6, 1)
        self.g = helper.bits(value, 8, 1)
        self.pat = helper.bits(value, 12, 1)
        self.xd = helper.bits(value, 63, 1)
        if self.ps == 1:
            self.reference = helper.bits(value, 21, mmu.M - (21 - 12))

    def addr(self):
        if self.ps == 1:
            return self.reference << (12 + 9)
        return self.reference << 12

    def name(self):
        return f"PD   (0x{self.base:x}[0x{self.idx:x}])"

    def description(self):
        if not self.present:
            return f"[p:0|raw: {self.raw:x}]"
        desc = [
            "p:1", f"rw:{self.rw}", f"us:{self.us}", f"pwt:{self.pwt}",
            f"pcd:{self.pcd}", f"a:{self.a}", f"d:{self.d}", f"ps:{self.ps}",
            f"g:{self.g}", f"r:{self.r}", f"r:{self.r}"
        ]
        if self.ps == 1:
            desc.append(f"pat:{self.pat}")
        desc = desc + [
            f"addr:0x{self.reference:x}", f"rsvd:{self.rsvd}", f"xd:{self.xd}"
        ]
        return "[" + str.join('|', desc) + f"] = {self.raw:x}"
    pass

class PTEntry(PageTableEntry):

    def __init__(self, base, idx, mmu, value):
        super().__init__(base, idx, mmu, value)
        self.d = helper.bits(value, 6, 1)
        self.g = helper.bits(value, 8, 1)
        self.xd = helper.bits(value, 63, 1)

    def name(self):
        return f"PT   (0x{self.base:x}[0x{self.idx:x}])"

    def description(self):
        if not self.present:
            return f"[p:0|raw: {self.raw:x}]"
        desc = [
            "p:1", f"rw:{self.rw}", f"us:{self.us}", f"pwt:{self.pwt}",
            f"pcd:{self.pcd}", f"a:{self.a}", f"d:{self.d}", f"ps:{self.ps}",
            f"g:{self.g}", f"r:{self.r}", f"r:{self.r}",
            f"addr:0x{self.reference:x}", f"rsvd:{self.rsvd}", f"xd:{self.xd}"
        ]
        return "[" + str.join('|', desc) + f"] = {self.raw:x}"

class FourLevelPagingTable:
    entries = 512
    pass

    def __init__(self, mmu, base):
        self.mmu = mmu
        self.base = base
        self.descriptor_size = 8

    def _create(self, base, idx, mmu, val):
        pass

    def __iter__(self):
        table = self

        class TableIterator:
            def __init__(self):
                self.idx = 0

            def __next__(self):
                if self.idx < FourLevelPagingTable.entries:
                    idx = self.idx
                    e = table.entry(self.idx)
                    self.idx = self.idx + 1
                    return idx, e
                raise StopIteration
        return TableIterator()

    def name(self):
        return f"table_{hex(self.base)}"

    def entry(self, idx):
        offset = idx * self.descriptor_size
        val = self.mmu.monitor.physical_memory(
            self.base + offset, self.descriptor_size)
        return self._create(self.base, idx, self.mmu, val)
    pass

class PML4Table(FourLevelPagingTable):

    def _create(self, base, idx, mmu, val):
        return PML4Entry(base, idx, self.mmu, val)
        pass

class PageDirectoryPointerTable(FourLevelPagingTable):

    def _create(self, base, idx, mmu, val):
        return PDPEntry(base, idx, self.mmu, val)
        pass

class PageDirectory(FourLevelPagingTable):

    def _create(self, base, idx, mmu, val):
        return PDEntry(base, idx, self.mmu, val)
        pass
    pass

class PageTable(FourLevelPagingTable):

    def _create(self, base, idx, mmu, val):
        return PTEntry(base, idx, self.mmu, val)
        pass
    pass

class MMU:
    def __init__(self, monitor, arch):
        self.monitor = monitor
        if arch == Arch.X86:
            self.bits = 32
            self.M = 20
            pass
        else:
            self.bits = 64
            self.M = 48  # todo check
            pass
        assert (self.bits == 64)  # TODO x86
        pass

    def assert_width(self, addr):
        assert ((1 << self.bits) > addr)
        pass

    def split_addr(self, addr: int) -> list[int]:
        parts = []
        # 4Ki pages
        parts.append([
            (addr >> 39) & 0x1ff,
            (addr >> 30) & 0x1ff,
            (addr >> 21) & 0x1ff,
            (addr >> 12) & 0x1ff,
            addr & 0xfff
        ])
        # 2Mi pages
        parts.append([
            (addr >> 39) & 0x1ff,
            (addr >> 30) & 0x1ff,
            (addr >> 21) & 0x1ff,
            addr & 0x1fffff
        ])

        # 1Gi pages
        parts.append([
            (addr >> 39) & 0x1ff,
            (addr >> 30) & 0x1ff,
            addr & 0x3fffffff
        ])
        return parts

    def resolve(self, base: int, addr: int):

        self.assert_width(addr)

        entries = list()
        parts = self.split_addr(addr)

        pml4_tbl = PML4Table(self, base)
        pml4_idx = parts[0][0]
        pml4_entry = pml4_tbl.entry(pml4_idx)
        entries.append(pml4_entry)

        if not pml4_entry.present:
            return (None, None, None, entries)

        pdp_tbl = PageDirectoryPointerTable(self, pml4_entry.addr())
        pdp_idx = parts[0][1]
        pdp_entry = pdp_tbl.entry(pdp_idx)
        entries.append(pdp_entry)

        if not pdp_entry.present:
            return (None, None, None, entries)

        if (pdp_entry.ps):
            return pdp_entry.addr(), (4096 << 18), parts[2][2], entries

        pd_tbl = PageDirectory(self, pdp_entry.addr())
        pd_idx = parts[0][2]
        pd_entry = pd_tbl.entry(pd_idx)
        entries.append(pd_entry)

        if not pd_entry.present:
            return (None, None, None, entries)

        if (pd_entry.ps):
            return pd_entry.addr(), (4096 << 9), parts[1][3], entries

        pt_tbl = PageTable(self, pd_entry.addr())
        pt_idx = parts[0][3]
        pt_entry = pt_tbl.entry(pt_idx)
        entries.append(pt_entry)

        if not pt_entry.present:
            return (None, None, None, entries)

        physical = pt_entry.addr()
        return physical, 4096, parts[0][4], entries

    def linear_bytes(self, mapping, addr, len):
        chunks = []
        while len > 0:
            page, size, offset, _ = self.resolve(mapping, addr)
            # read larger 8 byte blocks
            if offset % 8 == 0:
                while offset + 8 < size and len >= 8:
                    b = self.monitor.physical_memory(page + offset, 8)
                    chunks.append(int.to_bytes(b, 8, 'little'))
                    len = len - 8
                    offset = offset + 8
                pass

            # read single bytes
            while offset < size and len > 0:
                b = self.monitor.physical_memory(page + offset, 1)
                chunks.append(int.to_bytes(b, 1, 'little'))
                len = len - 1
                offset = offset + 1
                pass
            addr = page + size
            pass
        return bytes().join(chunks)
