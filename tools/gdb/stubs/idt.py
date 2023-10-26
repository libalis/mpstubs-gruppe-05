
import gdb
from . import helper

class IDT:
    class InterruptTrapGate:
        def __init__(self, raw):
            self.raw = raw  # 128-bit type
            self.offset = helper.bits(self.raw, 0, 16) \
                | helper.bits(self.raw, 32 + 16, 16) << 16\
                | helper.bits(self.raw, 64, 32) << 32
            self.segment = helper.bits(self.raw, 16, 16)
            self.ist = helper.bits(self.raw, 32, 4)
            self.type = helper.bits(self.raw, 32 + 8, 4)
            self.dpl = helper.bits(self.raw, 32 + 13, 2)
            self.p = helper.bits(self.raw, 32 + 15, 1)

        def _get_symbol(self):
            block = gdb.block_for_pc(self.offset)
            if block is None:
                return "?"
            if block.function is None:
                return "?"
            return block.function

        def __str__(self):
            symbol = self._get_symbol()
            addr = f"0x{self.segment:x}:0x{self.offset:x}"
            bits = "|".join([
                f"off:{self.offset:x}", f"p:{self.p}",
                f"dpl:{self.dpl}", f"type:{self.type:x}",
                f"ist:{self.ist:x}", f"ss:{self.segment:x}"
            ])
            return f"{addr} <{symbol}> [{bits}] raw={self.raw:032x}"

    def __init__(self, mmu, mapping, base, limit):
        self.mmu = mmu
        self.mapping = mapping
        self.base = base
        self.limit = limit
        self.desc_size = 16
        self.nentry = (limit + 1) // self.desc_size
        pass

    def print(self):
        entries = []
        for i in range(0, self.nentry):
            offset = i * self.desc_size
            desc_bytes = self.mmu.linear_bytes(
                self.mapping,
                self.base + offset,
                self.desc_size
            )
            desc = int.from_bytes(desc_bytes, 'little')
            gate = self.InterruptTrapGate(desc)
            entries.append((i, gate))

        def cmp_eq(a, b):
            return a.offset == b.offset and a.segment == b.segment

        compact = helper.compact_entries(entries, cmp_eq)
        for start, stop, gate in compact:
            if start == stop:
                print(f"[{start}]:\t{str(gate)}")
            else:
                print(f"[{start}-{stop}]:\t{str(gate)}")
            pass
        pass
