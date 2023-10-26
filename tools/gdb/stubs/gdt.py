
from . import helper

class SegmentDescriptor(object):
    pass

    def __init__(self, memview):
        assert (len(memview) > 7)
        self.kind = '<generic descriptor>'

        raw = int.from_bytes(memview[0:8], byteorder='little')

        self.base = helper.bits(raw, 16, 24) \
            | helper.bits(raw, 32 + 24, 8) << 24

        self.limit = helper.bits(raw, 0, 16) \
            | helper.bits(raw, 32 + 16, 4) << 16

        self.type = helper.bits(raw, 32 + 8, 4)

        self.g = helper.bits(raw, 32 + 23, 1)
        self.p = helper.bits(raw, 32 + 15, 1)
        self.l = helper.bits(raw, 32 + 21, 1)
        self.dpl = helper.bits(raw, 32 + 13, 2)
        pass

    def create(memview):
        raw = int.from_bytes(memview[0:8], byteorder='little')
        feature = helper.bits(raw, 32 + 11, 2)
        if feature == 2:
            return DataSegmentDescriptor(memview)
        elif feature == 3:
            return CodeSegmentDescriptor(memview)
        else:
            feature = helper.bits(raw, 32 + 8, 5)
            if feature == 0x00:
                return NullDescriptor(memview)
            elif feature == 0x0f:
                return CallGateDescriptor(memview)
            else:
                return SystemSegmentDescriptor.create(memview)
        pass

    def __str__(self):
        verbose = self.str_verbose()
        return f"<{self.kind}> 0x{self.base:x}:{self.limit:x} [{verbose}]"
        if verbose:
            return f"<{self.kind}> 0x{self.base:x}:{self.limit:x} [{verbose}]"
        else:
            return f"<{self.kind}> 0x{self.base:x}:{self.limit:x}"

    def size(self):
        return None

class DataSegmentDescriptor(SegmentDescriptor):
    def __init__(self, memview):
        super(DataSegmentDescriptor, self).__init__(memview)
        self.kind = 'data'

        self.raw = int.from_bytes(memview[0:8], byteorder='little')

        self.b = helper.bits(self.raw, 32 + 22, 1)
        self.avl = helper.bits(self.raw, 32 + 20, 1)

        self.e = helper.bits(self.raw, 32 + 10, 1)
        self.w = helper.bits(self.raw, 32 + 9, 1)
        self.a = helper.bits(self.raw, 32 + 8, 1)

    def str_verbose(self):
        msg = "|".join([
            f"g{self.g}", f"b{self.b}", f"l{self.l}",
            f"avl{self.avl}", f"p{self.p}", f"dpl{self.dpl}",
            f"e{self.e}", f"w{self.w}", f"a{self.a} {self.raw:x}"
        ])
        if self.l:
            msg = msg + '"invalid l"'
            pass
        return msg

    def size(self):
        return 8

class CodeSegmentDescriptor(SegmentDescriptor):
    def __init__(self, memview):
        super(CodeSegmentDescriptor, self).__init__(memview)
        self.kind = 'code'

        self.raw = int.from_bytes(memview[0:8], byteorder='little')

        self.d = helper.bits(self.raw, 32 + 22, 1)
        self.avl = helper.bits(self.raw, 32 + 20, 1)

        self.c = helper.bits(self.raw, 32 + 10, 1)
        self.r = helper.bits(self.raw, 32 + 9, 1)
        self.a = helper.bits(self.raw, 32 + 8, 1)

    def size(self):
        return 8

    def str_verbose(self):
        return "|".join([
            f"g{self.g}", f"d{self.d}", f"l{self.l}",
            f"avl{self.avl}", f"p{self.p}", f"dpl{self.dpl}",
            f"c{self.c}", f"r{self.r}", f"a{self.a}", f"{self.raw:x}"
        ])

class SystemSegmentDescriptor(SegmentDescriptor):

    def __init__(self, memview):
        super(SystemSegmentDescriptor, self).__init__(memview)
        self.kind = 'system'
        self.raw = int.from_bytes(memview[0:16], byteorder='little')
        pass

    def create(memview):
        raw = int.from_bytes(memview[0:8], byteorder='little')
        type = helper.bits(raw, 32 + 8, 4)
        masks = [(TSSDescriptor, 0x9)]
        for ctor, mask in masks:
            if type & mask == mask:
                return ctor(memview)
            pass
        else:
            raise Exception("no matching Descriptor")
        pass

    def size(self):
        return 16

    def str_verbose(self):
        return f"raw: {self.raw}"

class TSSDescriptor(SystemSegmentDescriptor):

    def __init__(self, memview):
        super(SystemSegmentDescriptor, self).__init__(memview)
        self.kind = 'tss'
        self.raw = int.from_bytes(memview[0:16], byteorder='little')

        self.type = helper.bits(self.raw, 32 + 8, 4)

        self.base = helper.bits(self.raw, 16, 24) \
            | helper.bits(self.raw, 32 + 24, 8) << 24 \
            | helper.bits(self.raw, 64, 32) << 32

        self.avl = helper.bits(self.raw, 32 + 20, 1)

        self.null = helper.bits(self.raw, 3 * 32 + 8, 5)
        self.reserved_a = helper.bits(self.raw, 3 * 32 + 13, 32 - 13)
        self.reserved_b = helper.bits(self.raw, 3 * 32, 8)
        assert (self.null == 0)

    def str_verbose(self):
        b = helper.bits(self.raw, 32 + 22, 1)
        e = helper.bits(self.raw, 32 + 12, 1)
        msg = "|".join([
            f"rsvd: {self.reserved_a}", f"0: {self.null}",
            f"rsvd: {self.reserved_b}" f"g: {self.g}",
            f"0{b}", f"0{self.l}", f"avl{self.avl}",
            f"p{self.p}", f"dpl{self.dpl}", f"0{e}",
            f"type: {self.type:x}", f"{self.raw:x}"
        ])
        return msg

class NullDescriptor(SystemSegmentDescriptor):
    def __init__(self, memview):
        super(NullDescriptor, self).__init__(memview)
        self.kind = 'call gate'
        self.kind = 'null'
        self.raw = int.from_bytes(memview[0:8], byteorder='little')
        pass

    def size(self):
        return 8

    def str_verbose(self):
        return f"{self.raw:x}"

class CallGateDescriptor(SystemSegmentDescriptor):
    def __init__(self, mems):
        super(CallGateDescriptor, self).__init__(mems)
        self.kind = 'call gate'
        pass
    pass

class GDT:
    def __init__(self, mmu, mapping, base, limit):
        self.mmu = mmu
        self.mapping = mapping
        self.limit = limit
        self.base = base
        pass

    def print(self):
        offset = 0
        print(f"base: 0x{self.base:x}, limit: 0x{self.limit:x}")
        while offset < self.limit + 1:

            desc_bytes = self.mmu.linear_bytes(
                self.mapping,
                self.base + offset,
                16
            )

            segment = SegmentDescriptor.create(desc_bytes)
            print(f"[0x{offset:x}]: {str(segment)}")
            offset += segment.size()
            pass
        pass
