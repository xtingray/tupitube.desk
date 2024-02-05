#!/usr/bin/ruby

###########################################################################
#   Project TUPI: Magia 2D                                                #
#   Project Contact: info@maefloresta.com                                 #
#   Project Website: http://www.maefloresta.com                           #
#   Project Leader: Gustav Gonzalez <info@maefloresta.com>                #
#                                                                         #
#   Developers:                                                           #
#   2010:                                                                 #
#    Gustavo Gonzalez / xtingray                                          #
#                                                                         #
#   KTooN's versions:                                                     #
#                                                                         #
#   2006:                                                                 #
#    David Cuadrado                                                       #
#    Jorge Cuadrado                                                       #
#   2003:                                                                 #
#    Fernado Roldan                                                       #
#    Simena Dinas                                                         #
#                                                                         #
#   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       #
#   License:                                                              #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program.  If not, see <http://www.gnu.org/licenses/>. #
###########################################################################

# TODO: This script must detect if every command line given is valid 
#       Currently, it just try to check if some of them are included or not

# Note: gem install os -v 0.9.6

require 'fileutils'
require_relative 'qonf/configure'
require_relative 'qonf/info'
require_relative 'qonf/defaults'

begin
    conf = RQonf::Configure.new(ARGV)

    if conf.hasArgument?("help") or conf.hasArgument?("h")
       puts <<_EOH_
Use: ./configure [options]
  options:
  --help:                   Show this message
  --prefix=[path]:          Sets installation path [/usr]
  --bindir=[path]:          Set binaries path [/usr/bin]
  --libdir=[path]:          Set library path [/usr/lib/tupitube | /usr/lib64/tupitube]
  --sharedir=[path]:        Set data path [/usr/share]
  --with-ffmpeg=[path]:     Set ffmpeg installation path [/usr]
  --with-quazip=[path]:     Set quazip installation path [/usr]
  --with-libsndfile=[path]: Set libsndfile installation path [/usr]
  --without-ffmpeg:         Disable ffmpeg support
  --without-debug:          Disable debug
  --with-qtdir=[path]:      Set Qt directory [i.e. /usr/local/qt]
  --package-build:          Option exclusive for package maintainers
  --install-headers:        Include header files as part of installation
_EOH_
        exit 0
    end

    debug = 1
    if conf.hasArgument?("without-debug")
       debug = 0
    end

    config = RQonf::Config.new

    version = "0.2"
    codeName = "Xamã"
    revision = "22"
    configVersion = "5"

    Info.info << "Compiling \033[91mTupiTube " +  version + "." + revision + "\033[0m (" +  codeName + ")" << $endl
    Info.info << "Debug support... "

    file_name = 'src/components/components.pro'
    if debug == 1
       var = open(file_name).grep(/debug/)
       if var.count == 0
          open(file_name, 'a') { |f|
               f.puts "SUBDIRS += debug"
          }
       end

       config.addOption("debug")
       config.addDefine("TUP_DEBUG")
       print "[ \033[92mON\033[0m ]\n"
    else
       var = open(file_name).grep(/debug/)
       if var.count > 0
          text = File.read(file_name)
          new_contents = text.gsub(/\nSUBDIRS \+\= debug/, "")
          File.open(file_name, "w") {|file| file.puts new_contents }
       end

       config.addOption("release")
       config.addDefine("TUP_NODEBUG")
       print "[ \033[91mOFF\033[0m ]\n"
    end

    # config.addDefine("TUP_SERVER")

    if conf.hasArgument?("with-ffmpeg") and conf.hasArgument?("without-ffmpeg")  
       Info.error << " ERROR: Options --with-ffmpeg and --without-ffmpeg are mutually exclusive\n"
       exit 0
    end

    if conf.hasArgument?("with-qtdir")
       qtdir = conf.argumentValue("with-qtdir")
       conf.verifyQtVersion("5.15.0", debug, qtdir)
    else
       conf.verifyQtVersion("5.15.0", debug, "")
    end

    if conf.hasArgument?("with-ffmpeg")
       ffmpegDir = conf.argumentValue("with-ffmpeg")
       if File.directory? ffmpegDir 
          ffmpegLib = ffmpegDir + "/lib"
          ffmpegInclude = ffmpegDir + "/include"
          config.addLib("-L" + ffmpegLib)
          config.addIncludePath(ffmpegInclude)
       else
          Info.error << " ERROR: ffmpeg directory does not exist!\n"
          exit 0
       end
    else
       if conf.hasArgument?("without-ffmpeg")
          Info.warn << "Disabling ffmpeg support: " << $endl
          conf.disableFFmpeg()
       end
    end

    if conf.hasArgument?("with-quazip")
       quazipDir = conf.argumentValue("with-quazip")
       if File.directory? quazipDir
          quazipLib = quazipDir + "/lib"
          if RUBY_PLATFORM =~ /linux/
              quazipInclude = quazipDir + "/include"
          elsif RUBY_PLATFORM =~ /darwin/
              quazipInclude = quazipDir + "/include"
          end
          config.addLib("-L" + quazipLib)
          config.addIncludePath(quazipInclude)
       else
          Info.error << " ERROR: quazip directory does not exist!\n"
          exit 0
       end
    end

    if conf.hasArgument?("with-libsndfile")
       libsndfileDir = conf.argumentValue("with-libsndfile")
       if File.directory? libsndfileDir
          libsndfileLib = libsndfileDir + "/lib"
          libsndfileInclude = libsndfileDir + "/include"
          config.addLib("-L" + libsndfileLib)
          config.addIncludePath(libsndfileInclude)
       else
          Info.error << " ERROR: libsndfile directory does not exist!\n"
          exit 0
       end
    end

    conf.createTests
    conf.setTestDir("configure.tests")
    conf.runTests(config, conf, debug)

    config.addModule("core")
    config.addModule("gui")
    config.addModule("svg")
    config.addModule("xml")
    config.addModule("network")

    config.addLib("-ltupifwgui")
    config.addLib("-ltupifwcore")
    # config.addLib("-ltupifwsound")
    
    if conf.hasArgument?("install-headers")
       config.addDefine("ADD_HEADERS");
    end

    config.addDefine('TUPITUBE_VERSION=\\\\\"' + version + '\\\\\"')
    config.addDefine('CODE_NAME=\\\\\"' + codeName + '\\\\\"')
    config.addDefine('REVISION=\\\\\"' + revision + '\\\\\"')
    config.addDefine('CONFIG_VERSION=\\\\\"' + configVersion + '\\\\\"')

    if File.exists?('/etc/canaima_version')
       config.addDefine("CANAIMA")
    end

    unix = config.addScope("unix")
    unix.addVariable("MOC_DIR", ".moc")
    unix.addVariable("UI_DIR", ".ui")
    unix.addVariable("OBJECTS_DIR", ".obj")

    config.save("tupiglobal.pri")
    conf.createMakefiles

    binaries = `find configure.tests -mindepth 1 -type d`
    array = binaries.split
    for item in array
        name = item.split("\/")
        file = item + "\/" + name[1]
        if FileTest.exists?(file)
           File.delete(file)
        end
    end
    exec('find configure.tests -iname main.o -exec rm -f {} \;')
    
rescue => err
    Info.error << "Configure failed. error was: #{err.message}\n"
    if $DEBUG
        puts err.backtrace
    end
end
