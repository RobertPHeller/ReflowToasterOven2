#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Fri Aug 10 11:03:32 2018
#  Last Modified : <180810.1153>
#
#  Description	
#
#  Notes
#
#  History
#	
#*****************************************************************************
#
#    Copyright (C) 2018  Robert Heller D/B/A Deepwoods Software
#			51 Locke Hill Road
#			Wendell, MA 01379-9728
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# 
#
#*****************************************************************************


package require snit

set argv0 [file join [file dirname [info nameofexecutable]] [file rootname [file tail [info script]]]]

snit::type ToasterConsole {
    pragma -hastypeinfo    no
    pragma -hastypedestroy no
    pragma -hasinstances   no
    typevariable filemap -array {
        PWMLog ManualPWM.log
        TempLOG ManualTemp.log
        AutoGoLOG AutoGo.log
        Profile Profile.dat
        Settings Settings.dat
    }
    typevariable fp {}
    typevariable outfp {}
    typevariable infp {}
    typevariable state idle
    typevariable outputFileStartPattern {^([^>]+)>}
    typevariable outputFileEndPattern {^([^<]+)<}
    typevariable inputFileStartPattern {^([^?]+)\?}
    typeconstructor {
        global argv
        global argv0
        global argc
        set port [from argv -port /dev/ttyACM0]
        set fp [open $port  w+]
        fconfigure $fp -mode 115200,n,8,2 -blocking no -buffering none \
              -handshake none -translation {crlf crlf}
        fileevent $fp readable [mytypemethod _ReadPort]
    }
    typemethod _ReadPort {} {
        foreach {in out} [fconfigure $fp -queue] {break}
        if {$in > 0} {
            set buffer [gets $fp]
            #puts stderr "*** _ReadPort: buffer = '$buffer'"
            switch $state {
                idle {
                    if {[regexp $outputFileStartPattern $buffer => outname] > 0} {
                        set outfp [open $filemap($outname) w]
                        set state output
                    } elseif {[regexp $inputFileStartPattern $buffer => inname] > 0} {
                        set infp [open $filemap($inname) r]
                        while {[gets $infp line] >= 0} {
                            puts $fp $line
                            foreach {in out} [fconfigure $fp -queue] {break}
                            if {$in > 0} {
                                set buffer [gets $fp]
                                puts $buffer
                            }
                        }
                        close $infp
                    } else {
                        puts $buffer
                    }
                }
                output {
                    if {[regexp $outputFileEndPattern $buffer => out] > 0} {
                        close $outfp
                        set state idle
                    } else {
                        if {$buffer ne {}} {puts $outfp $buffer}
                    }
                }
            }
        }
    }
}

vwait forever
