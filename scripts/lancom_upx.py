#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (C) 2024 J. Neuschäfer

import argparse, os, json, sys, datetime, re
from construct import Struct, Const, Byte, Bytes, Padding, Int8ub, Int16ub, Int32ub

def checksum(data):
    return ~sum(data) & 0xffff

def u16(data): return data[0] << 8 | data[1]
def u32(data): return u16(data) << 16 | u16(data[2:])


SMALL_BLOCK = 128   # for header and checksum blocks
LARGE_BLOCK = 1024  # for data blocks


def div_roundup(a, b):
    return (a + b - 1) // b


def eprint(s):
    sys.stderr.write(s)
    sys.stderr.write('\n')


UPXHeader = Struct(
    'board' / Bytes(16),
    Padding(32),
    'field30' / Int16ub,
    Const(0, Int16ub),
    'length' / Int32ub,
    'field30b' / Int16ub,
    'entry' / Int16ub,
    'magic' / Const(b'ELS'),
    'type' / Byte,
    Const(63, Int32ub),
    Const(0, Int32ub),
    'release' / Int8ub,
    'date' / Int16ub,
    Const(0, Int8ub),
    'build' / Int16ub,
    'major' / Byte,
    'dot' / Bytes(1),
    'minor' / Bytes(2),
    'model' / Bytes(43),
    'mystery' / Int8ub
)
assert UPXHeader.sizeof() == SMALL_BLOCK - 2


class SelfCheckedBlock:
    def __init__(self, data):
        if len(data) == SMALL_BLOCK - 2:
            self.data = checksum(data).to_bytes(2) + data
        else:
            self.data = data
        assert len(self.data) == SMALL_BLOCK


class HeaderBlock(SelfCheckedBlock):
    pass


class ChecksumBlock(SelfCheckedBlock):
    def __iter__(self):
        for i in range(2, len(self.data), 2):
            yield u16(self.data[i:i+2])


class DataBlock:
    def __init__(self, data):
        self.data = data


class Release:
    def __init__(self, r):
        if type(r) == int:
            self.tag = r
        else:
            if   r       ==   '':    self.tag = 0
            elif r.startswith('RC'): self.tag = int(r[2:])
            elif r       ==   'PR':  self.tag = 220
            elif r.startswith('RU'): self.tag = int(r[2:]) + 220
            elif r.startswith('SU'): self.tag = int(r[2:]) + 236
            elif r       ==   'b':   self.tag = 253
            elif r       ==   'a':   self.tag = 254
            elif r       ==   'Rel': self.tag = 255
            else: raise RuntimeError(f'Unknown release string {r!r}')

    def __str__(self):
        if   self.tag == 0:   return f''
        elif self.tag <= 219: return f'RC{self.tag}'
        elif self.tag <= 220: return f'PR'
        elif self.tag <= 236: return f'RU{self.tag-220}'
        elif self.tag <= 252: return f'SU{self.tag-236}'
        elif self.tag <= 253: return f'b'
        elif self.tag <= 254: return f'a'
        elif self.tag <= 255: return f'Rel'


class UPXVolume:
    EPOCH = datetime.date(year=1990, month=1, day=1)
    VERSION_RE = re.compile(r'([0-9]+)([.M])([0-9]+)\.([0-9]+)(.*)')

    def __init__(self, args):
        self.board = args.board
        self.model = args.model
        self.type = 'ELSF'
        self.date = args.date
        self.field30 = 0
        self.entry = 24
        (a, b, c, d, e) = self.VERSION_RE.match(args.version).groups()
        self.major = int(a)
        self.dot = b
        self.minor = int(c)
        self.build = int(d)
        self.release = Release(e)

    # FIXME: for reference only
    def from_json(self, j):
        self.j = j
        self.length = -1
        self.type = j['type']
        self.board = j['board']
        self.model = j['model']
        self.version = j['version']
        (a, b, c, d, e) = self.VERSION_RE.match(self.version).groups()
        self.major = int(a)
        self.dot = b
        self.minor = int(c)
        self.build = int(d)
        self.release = Release(e)
        self.date = j['date']
        self.field30 = j['field30']
        self.entry = j['entry']

    def __repr__(self):
        return f'UPXVolume(type={self.type}, board={self.board!r}, model={self.model!r}, date={self.date}, version={self.version}, len={self.length})'

    def file_size(self):
        data_blocks = div_roundup(self.h.length, LARGE_BLOCK)
        checksum_blocks = div_roundup(data_blocks, 63)
        total  = SMALL_BLOCK                    # header
        total += data_blocks * LARGE_BLOCK      # data
        total += checksum_blocks * SMALL_BLOCK  # checksums
        return total

    def num_blocks(self):
        data_blocks = div_roundup(self.h.length, LARGE_BLOCK)
        checksum_blocks = div_roundup(data_blocks, 63)
        return 1 + data_blocks + checksum_blocks

    def read_checked(self, cksum, length=LARGE_BLOCK):
        data = self.u.read(length)
        real = checksum(data)
        if real != cksum:
            b = offset = self.u.tell()
            a = offset - length
            raise RuntimeError(f'Checksum mismatch at offset {a:#x}-{b:#x}: {real:04x} != {cksum:04x}')
        return data

    def read_self_checked(self, length=SMALL_BLOCK):
        expected = u16(self.u.read(2))
        return self.read_checked(expected, length-2)

    def iter_blocks(self):
        self.u.seek(self.file_offset, os.SEEK_SET)
        yield HeaderBlock(self.read_self_checked())
        data_pos = 0
        while True:
            yield (checksums := ChecksumBlock(self.read_self_checked()))
            for i, ck in enumerate(checksums):
                yield DataBlock(self.read_checked(ck)[:self.length - data_pos])
                data_pos += LARGE_BLOCK
                if data_pos >= self.length:
                    return # We're done with this volume

    def unpack(self, b):
        for blk in self.iter_blocks():
            if type(blk) == DataBlock:
                b.write(blk.data)

    def get_metadata(self):
        return {
            'type': self.type,
            'board': self.board,
            'model': self.model,
            'version': self.version,
            'date': self.date,
            'field30': self.field30,
            'entry': self.entry,
        }

    def pack(self, binfile, upxfile):
        b = binfile
        u = upxfile

        self.length = b.seek(0, os.SEEK_END)
        b.seek(0, os.SEEK_SET)

        header = UPXHeader.build({
            'board': self.board.encode('ascii').ljust(16, b'\0'),
            'model': self.model.encode('ascii').ljust(43, b'\0'),
            'length': self.length,
            'type': ord(self.type[3]),
            'release': self.release.tag,
            'major': self.major + ord('0'),
            'dot': ord(self.dot),
            'minor': f'{self.minor:02}'.encode('ascii'),
            'build': self.build,
            'date': (datetime.date.fromisoformat(self.date) - self.EPOCH).days,
            'mystery': 1,
            'field30': self.field30,
            'field30b': self.field30,
            'entry': self.entry,
        })
        u.write(checksum(header).to_bytes(2) + header)

        while data := b.read(63 * LARGE_BLOCK):
            blocks = []
            checksums = []
            for i in range(63):
                block = data[i*LARGE_BLOCK:(i+1)*LARGE_BLOCK]
                if len(block) > 0:
                    block = block.ljust(LARGE_BLOCK, b'\xff')
                    blocks.append(block)
                checksums.append(checksum(block))

            checksum_block = b''.join(c.to_bytes(2) for c in checksums)
            assert u.write(checksum(checksum_block).to_bytes(2) + checksum_block) == SMALL_BLOCK

            for block in blocks:
                assert len(block) == LARGE_BLOCK
                u.write(block)


class UPXFile:
    def __init__(self, file):
        if type(file) == str:
            file = open(file, 'rb')
        self.f = file
        self.size = file.seek(0, os.SEEK_END)
        file.seek(0, os.SEEK_SET)

    def __iter__(self):
        self.f.seek(0, os.SEEK_SET)
        while self.f.tell() < self.size:
            v = UPXVolume(self.f)
            yield v
            self.f.seek(v.file_offset + v.file_size(), os.SEEK_SET)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Create a LANCOM UPX image')
    parser.add_argument('PAYLOAD', help='the program (e.g. u-boot-upx.bin)')
    parser.add_argument('-o', '--output', help='the UPX file to write')
    parser.add_argument('--board', help='board code name (e.g. NWAPP2)')
    parser.add_argument('--model', help='board model name or pattern', default='*')
    parser.add_argument('--version', help='firmware version number to write into the UPX header')
    parser.add_argument('--date', help='firmware build date to write into the UPX header', default='2025-01-01')
    args = parser.parse_args()

    output = open(args.output, 'wb')
    payload = open(args.PAYLOAD, 'rb')
    volume = UPXVolume(args)
    volume.pack(payload, output)
