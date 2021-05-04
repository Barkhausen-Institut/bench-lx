#!/usr/bin/env python3

import argparse
import traceback
from time import sleep
from datetime import datetime
import os, sys
import serial

import modids
import fpga_top
from noc import NoCmonitor
from tcu import EP, MemEP, Flags
from fpga_utils import FPGA_Error
import memory

DRAM_OFF = 0x10000000
MAX_FS_SIZE = 256 * 1024 * 1024
INIT_PMP_SIZE = 512 * 1024 * 1024
MAX_INITRD_SIZE = 64 * 1024 * 1024
PRINT_TIMEOUT = 60 # seconds

def write_file(mod, file, offset):
    print("%s: loading %u bytes to %#x" % (mod.name, os.path.getsize(file), offset))
    sys.stdout.flush()

    with open(file, "rb") as f:
        content = f.read()
    mod.mem.write_bytes_checked(offset, content, True)

def load_prog(dram, pms, i, args):
    pm = pms[i]
    print("%s: loading %s..." % (pm.name, args[0]))
    sys.stdout.flush()

    # start core
    pm.start()

    # reset TCU (clear command log and reset registers except FEATURES and EPs)
    pm.tcu_reset()

    # enable instruction trace for all PEs (doesn't cost anything)
    pm.rocket_enableTrace()

    # set features: privileged, vm, ctxsw
    pm.tcu_set_features(1, 1, 1)

    # invalidate all EPs
    for ep in range(0, 63):
        pm.tcu_set_ep(ep, EP.invalid())

    mem_begin = MAX_FS_SIZE + i * INIT_PMP_SIZE
    # install first PMP EP
    pmp_ep = MemEP()
    pmp_ep.set_pe(dram.mem.nocid[1])
    pmp_ep.set_vpe(0xFFFF)
    pmp_ep.set_flags(Flags.READ | Flags.WRITE)
    pmp_ep.set_addr(mem_begin)
    pmp_ep.set_size(INIT_PMP_SIZE)
    pm.tcu_set_ep(0, pmp_ep)

    # load ELF file
    dram.mem.write_elf(args[0], mem_begin - DRAM_OFF)
    sys.stdout.flush()

def main():
    mon = NoCmonitor()

    parser = argparse.ArgumentParser()
    parser.add_argument('--fpga', type=int)
    parser.add_argument('--reset', action='store_true')
    parser.add_argument('--pe', action='append')
    parser.add_argument('--serial', default='/dev/ttyUSB0')
    parser.add_argument('--until')
    parser.add_argument('--initrd')
    args = parser.parse_args()

    # connect to FPGA
    fpga_inst = fpga_top.FPGA_TOP(args.fpga)
    if args.reset:
        fpga_inst.eth_rf.system_reset()

    # stop all PEs
    for pe in fpga_inst.pms:
        pe.stop()

    # load programs onto PEs
    for i, peargs in enumerate(args.pe[0:len(fpga_inst.pms)], 0):
        load_prog(fpga_inst.dram1, fpga_inst.pms, i, peargs.split(' '))

    # load initrd
    if not args.initrd is None:
        write_file(fpga_inst.dram1, args.initrd, INIT_PMP_SIZE - MAX_INITRD_SIZE)

    # start PEs
    for pe in fpga_inst.pms:
        # start core (via interrupt 0)
        pe.rocket_start()

    ser = serial.Serial(port=args.serial, baudrate=115200, xonxoff=True)
    if not args.until is None:
        # wait for specific string
        while True:
            line = ser.read_until().decode().rstrip()
            print(line)
            sys.stdout.flush()
            if not args.until is None and args.until in line:
                break
    else:
        # interactive usage
        from serial.tools.miniterm import Miniterm
        miniterm = Miniterm(ser)
        miniterm.raw = True
        miniterm.set_rx_encoding('UTF-8')
        miniterm.set_tx_encoding('UTF-8')

        def key_description(character):
            """generate a readable description for a key"""
            ascii_code = ord(character)
            if ascii_code < 32:
                return 'Ctrl+{:c}'.format(ord('@') + ascii_code)
            else:
                return repr(character)

        sys.stderr.write('--- Miniterm on {p.name}  {p.baudrate},{p.bytesize},{p.parity},{p.stopbits} ---\n'.format(
            p=miniterm.serial))
        sys.stderr.write('--- Quit: {} | Menu: {} | Help: {} followed by {} ---\n'.format(
            key_description(miniterm.exit_character),
            key_description(miniterm.menu_character),
            key_description(miniterm.menu_character),
            key_description('\x08')))

        miniterm.start()
        try:
            miniterm.join(True)
        except KeyboardInterrupt:
            pass
        sys.stderr.write('\n--- exit ---\n')
        miniterm.join()
        miniterm.close()

try:
    main()
except FPGA_Error as e:
    sys.stdout.flush()
    traceback.print_exc()
except Exception:
    sys.stdout.flush()
    traceback.print_exc()
except KeyboardInterrupt:
    print("interrupt")
