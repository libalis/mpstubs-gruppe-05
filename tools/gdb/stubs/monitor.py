
import asyncio
import re
from qemu.qmp import QMPClient

class Monitor:
    def __init__(self, gdb):
        self.gdb = gdb
        pass

    def _hmc(self, cmd):
        raw = self.gdb.execute('monitor ' + cmd, to_string=True)
        return raw

    def registers(self):
        registers = dict()

        def query_registers():
            raw = self._hmc('info registers -a')
            return raw
        raw = query_registers()

        # each paragraph of `raw` contains the registers for a logical CPU
        cpu_split = raw.split('\r\n\r\n')
        cpu_split_stripped = (x.strip() for x in cpu_split)
        cpu_split = [s for s in cpu_split_stripped if s]

        # general purpose registers
        def fetch_gpr(input):
            registers = dict()
            gprs64 = ['RAX', 'RBX', 'RCX', 'RDX', 'RSI', 'RDI',
                    'RBP', 'RSP', 'R8', 'R9', 'R10', 'R11',
                    'R12', 'R13', 'R14', 'R15', 'RIP']
            gprs32 = [ 'EAX', 'EBX', 'ECX', 'EDX',
                    'ESI', 'EDI', 'EBP', 'ESP', 'EIP']
            for gpr in gprs32:
                pattern = rf"{gpr}\s?=(?P<{gpr}>\w{{8}})"
                match = re.search(pattern, input)
                if match is not None:
                    value_raw = match.group(gpr)
                    value = int(value_raw, 16)
                    registers[gpr.lower()] = value
            for gpr in gprs64:
                pattern = rf"{gpr}\s?=(?P<{gpr}>\w{{16}})"
                match = re.search(pattern, input)
                if match is not None:
                    value_raw = match.group(gpr)
                    value = int(value_raw, 16)
                    registers[gpr.lower()] = value
            return registers

        # control registers
        def fetch_cr(input):
            registers = dict()
            for cr in ['CR0', 'CR2', 'CR3', 'CR4']:
                pattern = rf"{cr}=(?P<{cr}>\w{{8,16}})"
                match = re.search(pattern, input)
                value_raw = match.group(cr)
                value = int(value_raw, 16)
                registers[cr.lower()] = value
            return registers

        # desriptor tables
        def fetch_dt(input):
            registers = dict()
            for tbl in ['GDT', 'IDT']:
                pattern = rf"{tbl}\s*=\s*(?P<{tbl}_base>\w{{8,16}})" \
                    rf"\s+(?P<{tbl}_limit>\w{{8}})"
                match = re.search(pattern, input)
                base_raw = match.group(f"{tbl}_base")
                limit_raw = match.group(f"{tbl}_limit")
                base = int(base_raw, 16)
                limit = int(limit_raw, 16)
                registers[tbl.lower()] = (base, limit)
            return registers

        registers = dict()
        for cpuid, regstr in enumerate(cpu_split):
            assert (regstr is not None and len(regstr) > 0)
            registers[cpuid] = dict()
            registers[cpuid].update(fetch_gpr(regstr))
            registers[cpuid].update(fetch_cr(regstr))
            registers[cpuid].update(fetch_dt(regstr))

        return registers

    def virtual_memory(self, addr, size):
        # byte, word, double word, giant
        types = {1: 'b', 2: 'w', 4: 'd', 8: 'g'}
        assert (size in types)

        def query_virtual_memory():
            res = self._hmc(f"x/x{types[size]} {addr}")
            return res

        res = query_virtual_memory()
        match = re.match(r"[a-f\d]+:\s*(0x[a-f\d]+)", res)
        assert (match)
        return int(match.group(1), 16)

    def physical_memory(self, addr, size):
        # byte, word, double word, giant
        types = {1: 'b', 2: 'w', 4: 'd', 8: 'g'}
        assert (size in types)

        def query_physical_memory():
            res = self._hmc(f"xp/x{types[size]} {addr}")
            return res

        res = query_physical_memory()
        match = re.match(r"[a-f\d]+:\s*(0x[a-f\d]+)", res)
        assert (match)
        return int(match.group(1), 16)

    def gva2gpa(self, addr):
        def query_gva2gpa():
            res = self._hmc(f"gva2gpa {addr}")
            return res

        res = query_gva2gpa()
        if res == 'Unmapped\r\n':
            return None
        match = re.match(r"gpa:\s*0x([\da-f]+)", res)
        assert (match)
        return int(match.group(1), 16)
