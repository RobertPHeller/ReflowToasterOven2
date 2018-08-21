#!/bin/bash
rm -rf ToasterConsole.{kit,vfs}
sdx qwrap ToasterConsole.tcl
sdx unwrap ToasterConsole.kit
ln -s /usr/share/tcltk/tcllib1.16/snit ToasterConsole.vfs/lib/
sdx wrap ToasterConsole -runtime /usr/local/share/tclkits/tclkit-linux-armv7l
