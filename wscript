#!/usr/bin/python3

import glob
from os import path
import git
import time

APPNAME = 'debian-demo'
VERSION = '0.0.0'
MAINTAINER = 'Wlanxww <xueweiwujxw@outlook.com>'
ARCH = 'amd64'
DESCRIPTION = 'Debian or Ubuntu Demo Service'


def options(opt):
    opt.load('compiler_cxx')
    repo = git.Repo(search_parent_directories=True)
    sha = repo.git.rev_parse(repo.head.commit.hexsha, short=8)
    stamp = int(time.time())
    opt.add_option('--hash', action='store', default=sha,
                   help='git commit hash value, default: current commit hash')
    opt.add_option('--debug', action='store_true',
                   default=False, help='enable debug mode, default: False')
    opt.add_option('--stamp', action='store',
                   default=stamp, help='configure time, default: current timestamp')
    opt.add_option('--mode', action='store',
                   default='develop', help='test: test mode, develop: development mode, product: production mode, default: develop')


def build(bld):
    defines = [
        f'VERSION="{VERSION}"',
        f'STAMP="{bld.env.stamp}"',
        f'HASH="{bld.env.hash}"',
        f'APPNAME="{APPNAME.upper()}"'
    ]
    if bld.env.debug:
        defines.append('DEBUG')

    includepath = [src_dir for src_dirs in ['src', 'lib']
                   for src_dir in glob.glob(f'{src_dirs}/**/', recursive=True)]

    bld.shlib(
        source=glob.glob('src/**/*.c*', recursive=True) +
        glob.glob('lib/**/*.c*', recursive=True),
        lib=['pthread'],
        includes=includepath,
        vnum=VERSION,
        defines=defines,
        target='ddemo'
    )
    apptargets = glob.glob('app/*.c*')
    testargets = glob.glob('test/*.c*')
    for app in apptargets:
        appname = path.splitext(path.basename(app))[0]
        bld.program(
            source=app,
            use=['ddemo'],
            includes=includepath,
            rpath='$ORIGIN',
            lib=['pthread'],
            target=appname,
            defines=defines
        )
    if bld.env.mode == 'test':
        for test in testargets:
            appname = path.splitext(path.basename(test))[0]
            bld.program(
                source=test,
                use=['ddemo'],
                includes=includepath,
                rpath='$ORIGIN',
                lib=['pthread'],
                target=appname,
                defines=defines
            )
    bld.install_files(bld.path.abspath()+'/out/DEBIAN',
                      glob.glob('DEBIAN/*'), chmod=0o775)
    bld.install_as(bld.path.abspath()+f'/out/lib/systemd/system/{APPNAME}.service',
                   f'assets/{APPNAME}.service')
    bld.install_as(bld.path.abspath()+'/out/etc/logrotate.d/debian-demo',
                   glob.glob('assets/debian-demo-logrotate'))
    bld.install_as(bld.path.abspath()+'/out/etc/rsyslog.d/debian-demo.conf',
                   glob.glob('assets/debian-demo-rsyslog'))
    bld.install_as(bld.path.abspath()+f'/out/DEBIAN/control',
                   '.control_info')


def configure(ctx):
    ctx.load('compiler_cxx')
    ctx.env.target = 'host'
    ctx.env.hash = ctx.options.hash
    ctx.env.debug = ctx.options.debug
    ctx.env.stamp = ctx.options.stamp
    ctx.env.mode = ctx.options.mode
    cxxflags = ['-Wall', '-std=c++17']
    if ctx.env.debug:
        cxxflags.append('-g')
    else:
        cxxflags.append('-O2')
    if ctx.env.COMPILER_CXX == 'clang++':
        ctx.env.append_value('CXXFLAGS', cxxflags)
    else:
        ctx.env.append_value('CXXFLAGS', cxxflags)
    ctx.exec_command(
        f'echo "Package: {APPNAME}\nMaintainer: {MAINTAINER}\nVersion: {VERSION}-{ctx.env.stamp}-{ctx.env.hash}\nArchitecture: {ARCH}\nDescription: {DESCRIPTION}" > '+ctx.path.abspath()+'/.control_info')
    ctx.exec_command(
        f'echo "dpkg -b ./out {APPNAME}_{VERSION}-{ctx.env.stamp}-{ctx.env.hash}_amd64.deb" > '+ctx.path.abspath()+'/package.sh')
    ctx.exec_command(
        f'echo "echo \"building package finished\"" >> '+ctx.path.abspath()+'/package.sh')
    ctx.exec_command('chmod a+x '+ctx.path.abspath()+'/package.sh')
