#!/bin/bash
rm -rf ToasterConsole.{kit,vfs}
sdx qwrap ToasterConsole.tcl
sdx unwrap ToasterConsole.kit
ln -s /usr/share/tcl/tcllib1.16/snit ToasterConsole.vfs/lib/
sdx wrap ToasterConsole -runtime /usr/local/share/tclkits/tclkit-8.5.8-linux-x86_64
