
from . import monitor
from . import idt
from . import gdt
from . import paging

import gdb
import traceback

qemu = monitor.Monitor(gdb)

def _active_cr3():
    i = gdb.selected_inferior()
    cr3_desc = i.architecture().registers().find('cr3')
    cr3 = gdb.selected_frame().read_register(cr3_desc)
    val = cr3.cast(gdb.lookup_type('unsigned long long'))
    return val

class PageVisualizer(gdb.Command):
    """resolves a virtual adress: vaview [<cr3>] <virtual address>"""

    def __init__(self, monitor):
        super(PageVisualizer, self).__init__("vaview", gdb.COMMAND_SUPPORT)
        self.monitor = monitor
        pass

    def invoke(self, arg, from_tty):
        args = gdb.string_to_argv(arg)
        base = None
        va = None
        if len(args) == 1:
            base = _active_cr3()
            va = gdb.parse_and_eval(args[0])
            pass
        elif len(args) == 2:
            base = gdb.parse_and_eval(args[0])
            va = gdb.parse_and_eval(args[1])
        else:
            raise gdb.GdbError("vaview [<cr3>] <virtual address>")

        try:
            base = int(base)
            if va.type.code == gdb.TYPE_CODE_FUNC:
                va = int(va.address)
            else:
                va = int(va)
                pass

            mmu = paging.MMU(self.monitor, paging.Arch.X86_64)
            page, size, offset, entries = mmu.resolve(base, va)

            parts = mmu.split_addr(va)
            print(
                f"cr3: 0x{base:x}; vaddr: 0x{va:x} = "
                f"({ '|'.join([hex(int(p)) for p in parts[0]]) })"
            )

            for e in entries:
                print(e)
            if page is not None and offset is not None:
                print(f"0x{va:x} -> 0x{page:x}:{offset:x}")
            else:
                print(f"0x{va:x} -> <unmapped>")
        except Exception as e:
            traceback.print_exc()
            raise e
        pass

def _gdtidtargs(arg, kind):
    args = gdb.string_to_argv(arg)
    if len(args) == 0:
        try:
            mapping = _active_cr3()
            cpuid = current_cpuid()
            regs = qemu.registers()[cpuid]
            base, limit = regs[kind]
        except Exception as e:
            traceback.print_exc()
            raise e
        pass
    elif len(args) == 2:
        # base, limit
        mapping = _active_cr3()
        base = gdb.parse_and_eval(args[0])
        limit = gdb.parse_and_eval(args[1])
        try:
            limit = int(limit)
            if base.type.code == gdb.TYPE_CODE_FUNC:
                base = int(base.address)
            else:
                base = int(base)
        except Exception as e:
            traceback.print_exc()
            raise e
        pass
    elif len(args) == 3:
        # mapping, cr3, limit
        mapping = gdb.parse_and_eval(args[0])
        base = gdb.parse_and_eval(args[1])
        limit = gdb.parse_and_eval(args[2])
        try:
            limit = int(limit)
            if base.type.code == gdb.TYPE_CODE_FUNC:
                base = int(base.address)
            else:
                base = int(base)
        except Exception as e:
            traceback.print_exc()
            raise e
        pass
    else:
        raise gdb.GdbError("invalid args")
    mapping = int(mapping)
    return mapping, base, limit

class GDTVisualizer(gdb.Command):
    """print the GDT: gdtview [[<mapping>] <base> <limit>]"""

    def __init__(self, monitor):
        super(GDTVisualizer, self).__init__("gdtview", gdb.COMMAND_SUPPORT)
        self.monitor = monitor
        pass

    def invoke(self, arg, from_tty):
        mapping, base, limit = _gdtidtargs(arg, 'gdt')
        mmu = paging.MMU(self.monitor, paging.Arch.X86_64)
        gdt.GDT(mmu, mapping, base, limit).print()

class InterruptGateVisualizer(gdb.Command):
    """print the IDT: idtview [[<mapping>] <base> <limit>]"""

    def __init__(self, monitor):
        super(InterruptGateVisualizer, self).__init__(
            "idtview", gdb.COMMAND_USER)
        self.monitor = monitor
        pass

    def invoke(self, args, from_tty):
        mapping, base, limit = _gdtidtargs(args, 'idt')
        mmu = paging.MMU(self.monitor, paging.Arch.X86_64)
        try:
            idt.IDT(mmu, mapping, base, limit).print()
        except Exception as e:
            traceback.print_exc()
            raise e

class CurThread(gdb.Function):
    """Fetch the current thread from `struct pcpu` at `$gs_base`"""
    def __init__(self):
        super(CurThread, self).__init__("curthread")
        pass
    def invoke(self):
        i = gdb.selected_inferior()
        a = i.architecture()
        gs_base_desc = a.registers().find('gs_base')

        frame = gdb.selected_frame()
        gs_base = frame.read_register(gs_base_desc)

        pcpu_ptr_type = gdb.lookup_type('struct pcpu').pointer()
        pcpu = gs_base.cast(pcpu_ptr_type)

        pc_curthread = pcpu.dereference().type['pc_curthread']

        curthread_ptr = pcpu.dereference()[pc_curthread]
        return curthread_ptr

def current_cpuid():
    inferior = gdb.selected_inferior()

    max_threadid = len(inferior.threads())
    assert (max_threadid > 0)
    _, threadid, _ = gdb.selected_thread().ptid
    assert (threadid > 0 and threadid <= max_threadid)

    cpuid = threadid - 1
    return cpuid
    pass

InterruptGateVisualizer(qemu)
PageVisualizer(qemu)
GDTVisualizer(qemu)
CurThread()
