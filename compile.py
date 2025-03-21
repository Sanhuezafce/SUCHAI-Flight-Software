#!/usr/bin/env python
import os
import argparse
import src.system.include.configure as configure


def get_parameters():
    """
    Parse script arguments
    """
    parser = argparse.ArgumentParser(prog='compile.py')
    # config.h template parameters
    parser.add_argument('os', type=str, default="LINUX")
    parser.add_argument('arch', type=str, default="NANOMIND")
    parser.add_argument('--log_lvl', type=str, default="LOG_LVL_INFO")
    parser.add_argument('--name', type=str, default="SUCHAI-DEV")
    parser.add_argument('--id',   type=str, default="0")
    parser.add_argument('--version',   type=str, default=configure.call_git_describe())
    parser.add_argument('--comm', type=str, default="1")
    parser.add_argument('--fp', type=str, default="1")
    parser.add_argument('--hk', type=str, default="1")
    parser.add_argument('--test', type=str, default="0")
    parser.add_argument('--node', type=str, default="1")
    parser.add_argument('--zmq_in', type=str, default="tcp://127.0.0.1:8001")
    parser.add_argument('--zmq_out', type=str, default="tcp://127.0.0.1:8002")
    parser.add_argument('--st_mode', type=str, default="1")
    parser.add_argument('--st_triple_wr', type=str, default="1")
    # Build parameters
    parser.add_argument('--drivers', action="store_true", help="Install platform drivers")
    parser.add_argument('--ssh', action="store_true", help="Use ssh for git clone")
    # Force clean
    parser.add_argument('--clean', action="store_true", help="Clean before build")
    # Program
    parser.add_argument('--program', action="store_true", help="Compile and program")
    parser.add_argument('--console', type=int, default=4, help="Console to use. 2=Nanomind-USB-SERIAL, 4=FFP-USB")
    # Skip config
    parser.add_argument('--no-config', action="store_true", help="Skip configure, do not generate a new config.h")

    return parser.parse_args()


if __name__ == "__main__":
    # Parse parameters
    args = get_parameters()

    cwd_root = os.getcwd()

    # Generate config file
    if not args.no_config:
        configure.make_config(args,
                              'src/system/include/config_template.h',
                              'src/system/include/config.h')

    result = 0

    # Build
    if args.os == "LINUX":
        # Install CSP drivers
        if args.drivers:
            os.chdir('src/drivers/Linux/libcsp')
            os.system('sh install_csp.sh')
            os.chdir(cwd_root)

        os.system('rm -rf build_linux')
        os.system('mkdir build_linux')
        os.chdir('build_linux')
        os.system('cmake ..')
        result = os.system('make')

    else:  # args.os = FREERTOS
        if args.arch == "ESP32":
            if args.drivers:
                os.chdir('src/drivers/esp32')
                os.system('sh install.sh')
                os.chdir(cwd_root)
            result = os.system('make')

        if args.arch == "AVR32":
            os.chdir('src/drivers/atmel')
            # Install ASF
            if args.drivers:
                os.system('sh install.sh')
            os.chdir('xdk-asf-3.33.0/avr32/applications/suchai/xplained/gcc')
            result = os.system('sh build.sh')
            os.chdir(cwd_root)

        if args.arch == "NANOMIND":
            os.chdir('src/drivers/nanomind')
            # Install Nanomind SDK and LibCSP
            if args.drivers:
                if not args.ssh:
                    result = os.system('sh install.sh')
                else:
                    result = os.system('sh install.sh --ssh')
            elif args.clean:
                result = os.system('sh build.sh clean')
            elif args.program:
                result = os.system('sh build.sh program' + ' ' + str(args.console))
            else:
                result = os.system('sh build.sh')

    if result != 0:
        exit(1)

