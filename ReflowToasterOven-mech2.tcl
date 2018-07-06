#!/usr/local/bin/tclkit
#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Sun Mar 18 10:39:04 2018
#  Last Modified : <180706.0849>
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


eval [list lappend auto_path] [glob -nocomplain /usr/share/tcl*/tcllib*]
package require snit

snit::listtype point -minlen 3 -maxlen 3 -type snit::double
snit::listtype pointlist -minlen 3 -type point
snit::integer cval -min 0 -max 255
snit::listtype color -minlen 3 -maxlen 3 -type cval

snit::type PolySurface {
    typevariable _index 20
    option -rectangle -type snit::boolean -readonly yes -default no
    option -cornerpoint -type point -readonly yes -default {0 0 0}
    option -vec1 -type point -readonly yes -default {0 0 0}
    option -vec2 -type point -readonly yes -default {0 0 0}
    option -polypoints -type pointlist -readonly yes -default {{0 0 0} {0 0 0} {0 0 0}}
    variable index
    
    constructor {args} {
        $self configurelist $args
        set index $_index
        incr _index
    }
    method print {{fp stdout}} {
        if {$options(-rectangle)} {
            set cpp [eval [list format {P(%f,%f,%f)}] $options(-cornerpoint)]
            set v1  [eval [list format {D(%f,%f,%f)}] $options(-vec1)]
            set v2  [eval [list format {D(%f,%f,%f)}] $options(-vec2)]
            puts $fp [format {S%d = REC %s %s %s} $index $cpp $v1 $v2]
        } else {
            puts -nonewline $fp [format {S%d = POL} $index]
            foreach p $options(-polypoints) {
                puts -nonewline $fp [eval [list format { P(%f,%f,%f)}] $p]
            }
            puts $fp {}
        }
        return [format {S%d} $index]
    }
    method printPS {fp {xi 0} {yi 1} {xorg 0} {yorg 0} {xscale .001} {yscale .001}} {
        if {$options(-rectangle)} {
            set p $options(-cornerpoint)
            set x0 [lindex $p $xi]
            set y0 [lindex $p $yi]
            set v1 $options(-vec1)
            set v2 $options(-vec2)
            set dx1 [lindex $v1 $xi]
            set dx2 [lindex $v2 $xi]
            set dy1 [lindex $v1 $yi]
            set dy2 [lindex $v2 $yi]
            set dx [expr {$dx1 + $dx2}]
            set dy [expr {$dy1 + $dy2}]
            puts $fp [format {gsave %f %f translate %f %f scale} $xorg $yorg $xscale $yscale]
            puts $fp [format {newpath %f %f moveto} $x0 $y0]
            puts $fp [format {%f %f rlineto} 0 $dy]
            puts $fp [format {%f %f rlineto} $dx 0]
            puts $fp [format {%f %f rlineto} 0 [expr {0-$dy}]]
            puts $fp [format {%f %f rlineto} [expr {0-$dx}] 0]
            puts $fp {stroke grestore}
        } else {
            puts $fp [format {gsave %f %f translate %f %f scale newpath} $xorg $yorg $xscale $yscale]
            set cmd moveto
            foreach p $options(-polypoints) {
                puts $fp [format {%f %f %s} [lindex $p $xi] [lindex $p $yi] $cmd]
                set cmd lineto
            }
            puts $fp {stroke grestore}
        }
    }
    typemethod validate {obj} {
        if {[catch {$obj info type} thetype]} {
            error "Not a $type: $obj"
        } elseif {$thetype ne $type} {
            error "Not a $type: $obj"
        } else {
            return $obj
        }
    }
}

snit::type PrismSurfaceVector {
    typevariable _index 20
    option -surface -type ::PolySurface -readonly yes -default {}
    component surface
    option -vector  -type point -readonly yes -default {0 0 0}
    option -color -type color -readonly yes -default {147 147 173}
    variable index
    constructor {args} {
        $self configurelist $args
        set index $_index
        incr _index
        set surface $options(-surface)
    }
    method print {{fp stdout}} {
        puts $fp [eval [list format {DEFCOL %d %d %d}] $options(-color)]
        set s [$surface print $fp]
        set pr [format {B%d} $index]
        puts $fp [eval [list format {B%d = PRISM %s D(%f,%f,%f)} $index $s] $options(-vector)]
        return $pr
    }
    typemethod validate {obj} {
        if {[catch {$obj info type} thetype]} {
            error "Not a $type: $obj"
        } elseif {$thetype ne $type} {
            error "Not a $type: $obj"
        } else {
            return $obj
        }
    }
}

## Generic solid cylinders
snit::type cylinder {
    option -bottom -type point -readonly yes -default {0 0 0}
    option -radius -type snit::double -readonly yes -default 1
    option -height -type snit::double -readonly yes -default 1
    option -color -type color -readonly yes -default {0 0 0}
    option -direction -default Z -type {snit::enum -values {X Y Z}} -readonly yes
    variable index
    typevariable _index 20
    constructor {args} {
        $self configurelist $args
        set index $_index
        incr _index
    }
    method print {{fp stdout}} {
        puts $fp [eval [list format {DEFCOL %d %d %d}] $options(-color)]
        puts $fp [format {C%d = P (%f %f %f) VAL (%f) D%s} $index \
                        [lindex $options(-bottom) 0] \
                        [lindex $options(-bottom) 1] \
                        [lindex $options(-bottom) 2] \
                        $options(-radius) $options(-direction)]
        puts $fp [format {b%d = PRISM C%d %f} $index $index $options(-height)]
    }
    method printPS {fp {xi 0} {yi 1} {xorg 0} {yorg 0} {xscale .001} {yscale .001}} {
        set b $options(-bottom)
        set xcenter [lindex $b $xi]
        set ycenter [lindex $b $yi]
        set raduis  $options(-radius)
        puts $fp [format {gsave %f %f translate %f %f scale} $xorg $yorg $xscale $yscale]
        puts $fp [format {newpath %f %f %f 0 360 arc fill} $xcenter $ycenter $raduis]
        puts $fp {grestore}
    }
    typemethod validate {obj} {
        if {[catch {$obj info type} thetype]} {
            error "Not a $type: $obj"
        } elseif {$thetype ne $type} {
            error "Not a $type: $obj"
        } else {
            return $obj
        }
    }
}

snit::enum DimPlane -values {X Y Z P {}}

snit::type Dimension3D {
    option -point1 -type point -default {0 0 0} -readonly yes
    option -point2 -type point -default {0 0 0} -readonly yes
    option -textpoint -type point -default {0 0 0} -readonly yes
    option -dimplane -type DimPlane -default {} -readonly yes
    option -additionaltext -default {} -readonly yes
    constructor {args} {
        $self configurelist $args
        set var [format "N%d" $gindex]
        incr gindex
    }
    typevariable gindex 20
    variable var
    method print {{fp stdout}} {
        set dimplane [$self cget -dimplane]
        set p1 [eval [list format {P(%g %g %g)}] [$self cget -point1]]
        set p2 [eval [list format {P(%g %g %g)}] [$self cget -point2]]
        set tp [eval [list format {P(%g %g %g)}] [$self cget -textpoint]]
        puts $fp [format {%s=DIM3 %s %s %s %s "%s"} \
                  $var $p1 $p2 $tp $dimplane [$self cget -additionaltext]]
    }
}
    

set brown  {165  42  42}
set tan    {210 180 140}
set white  {255 255 255}
set black  {  0   0   0}
set yellow {255 255   0}
set pink   {255 192 203}
set green  {  0 255   0}
set blue   {  0   0 255}
set red    {255   0   0}
set orange {255 165   0}

proc MM2Mils {MM} {
    return [expr {($MM/25.4)*1000}]
}

snit::type ReflowToasterOven2Board {
    typevariable boardWidth 1801.5748031496064;#[::MM2Mils 45.76]
    typemethod BoardWidth {} {return $boardWidth}
    typevariable boardLength 4500.0;#[::MM2Mils 114.30]
    typemethod BoardLength {} {return $boardLength}
    typevariable holes { {4400 500 0} {4400 1300 0} {150 150 0} 
        {150 1650 0} }
    typevariable FeatherWidth 900
    typevariable FeatherLength 2000
    typevariable FeatherYUSB_Offset 450
    typevariable FeatherYBoardOffset 500
    method StandOff {index heightMM {above no} {name %AUTO%}} {
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        if {$above} {
            set zoff [expr {$zoff + 62.5}]
        } else {
            set heightMM [expr {$heightMM * -1}]
        }
        set p [lindex $holes $index]
        lassign $p x y z
        return [cylinder create ${name}_StandOff_${index} \
                -bottom [list [expr {$x  + $xoff}] [expr {$y + $yoff}] [expr {$z + $zoff}]] \
                -radius 125 -height [expr {(double($heightMM)/25.4)*1000}] \
                -color $::white]
    }
    method USBJack {{name {}}} {
        if {$name eq {}} {set name $selfns}
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        return [PrismSurfaceVector create ${name}_USBJack \
                -surface [PolySurface create ${name}_USBJackSurf \
                          -rectangle yes \
                          -cornerpoint [list [expr {$xoff - 40}] [expr {($FeatherYBoardOffset + $FeatherYUSB_Offset) + $yoff}] \
                                        [expr {(62.5 + 196.5 + 100 + 62.5) + $zoff}]] \
                          -vec1 {300 0 0 } -vec2 {0 220 0}] \
                -vector {0 0 110} -color $::white]
    }
    method USBJackCutout {{name %AUTO%}} {
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        return [PolySurface create ${name}_USBJackSurfCutout \
                -rectangle yes \
                -cornerpoint [list [expr {$xoff - 40}] [expr {($FeatherYBoardOffset + $FeatherYUSB_Offset) + $yoff}] \
                              [expr {(62.5 + 196.5 + 100 + 62.5) + $zoff}]] \
                -vec1 {0 0 110} -vec2 {0 300 0}]
    }
    component board
    component  h1    
    component  h2
    component  feather
    component    usbjack
    component  mhole1
    component  mhole2
    component  mhole3
    component  mhole4

    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    option -color -type color -default {  0 255   0} -readonly yes    
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set color [$self cget -color]
        install board using PrismSurfaceVector ${selfns}Board \
              -surface [PolySurface create ${selfns}BoardSurf \
                        -rectangle yes \
                        -cornerpoint [list $xoff $yoff $zoff] \
                        -vec1 [list $boardLength 0 0] \
                        -vec2 [list 0 $boardWidth 0]] \
              -vector {0 0 62.5} -color $::tan
        install h1 using PrismSurfaceVector  ${selfns}h1 \
              -surface [PolySurface create ${selfns}h1Surf \
                        -rectangle yes \
                        -cornerpoint [list [expr {200 + $xoff}] \
                                      [expr {500 + $yoff}] \
                                      [expr {62.5 + $zoff}]] \
                        -vec1 {1600 0 0} -vec2 {0 100 0}] \
              -vector {0 0 296.5} -color $::black
        install h2 using PrismSurfaceVector  ${selfns}h2 \
              -surface [PolySurface create ${selfns}h2Surf \
                        -rectangle yes \
                        -cornerpoint [list [expr {600 + $xoff}] \
                                      [expr {1300 + $yoff}] \
                                      [expr {62.5 + $zoff}]] \
                        -vec1 {1200 0 0} -vec2 {0 100 0}] \
              -vector {0 0 296.5} -color $::black
        install feather using PrismSurfaceVector ${selfns}Feather \
              -surface [PolySurface create ${selfns}FeatherSurf \
                        -rectangle yes \
                        -cornerpoint [list $xoff \
                                      [expr {$yoff + $FeatherYBoardOffset}] \
                                      [expr {$zoff + (62.5 + 196.5 + 100)}]] \
                        -vec1 [list $FeatherLength 0 0] \
                        -vec2 [list 0 $FeatherWidth 0]] \
              -vector {0 0 62.5} -color $::black
        set mhole1 [$self mountingHole 0]
        set mhole2 [$self mountingHole 1]
        set mhole3 [$self mountingHole 2]
        set mhole4 [$self mountingHole 3]
        set usbjack  [$self USBJack]
    }
    method mountingHole {index {name {}} {height 62.5} {standoffMM 0}} {
        if {$name eq {}} {set name $selfns}
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set p [lindex $holes $index]
        lassign $p x y z
        return [cylinder create ${name}UnoBoard_MountingHole_${index} \
                -bottom [list [expr {$x  + $xoff}] \
                         [expr {$y + $yoff}] [expr {$z + $zoff + (($standoffMM/25.4)*1000)}]] \
                -radius 62.5 -height $height -color $::white]
    }
    method print {{fp stdout}} {
        $board print $fp
        $h1 print $fp
        $h2 print $fp
        $feather print $fp
        $mhole1 print $fp
        $mhole2 print $fp
        $mhole3 print $fp
        $mhole4 print $fp
        $usbjack  print $fp
    }
}

snit::type ham_1591XXTSBK {
    typevariable topouterwidth 3278.3464566929133;#[::MM2Mils 83.27]
    typevariable topouterlength 4853.149606299213;#[::MM2Mils 123.27]
    typevariable bottomouterwidth 3135.8267716535433;#[::MM2Mils [expr {75.65 + 2 + 2}]]
    typevariable bottomouterlength 4710.629921259842;#[::MM2Mils [expr {115.65 + 2 + 2}]]
    typevariable outsideheight 2214.566929133858;#[::MM2Mils 56.25]
    typevariable insidewidth 2978.3464566929138;#[::MM2Mils 75.65]
    typevariable insidelength 4553.149606299213;#[::MM2Mils 115.65]
    typevariable insideheight 2165.3543307086616;#[::MM2Mils 55.00]
    typevariable wallthickness 78.74015748031496;#[::MM2Mils 2.00]
    method InsideFrontX {} {
        set xoff [$self cget -xoff]
        return [expr {$xoff + $wallthickness}]
    }
    method InsideFrontY {} {
        set yoff [$self cget -yoff]
        return [expr {$yoff + $wallthickness}]
    }
    method OutsizeFrontX {} {
        set xoff [$self cget -xoff]
        return [expr {$xoff - (($topouterlength-$bottomouterlength)/2.0)}]
    }
    method OutsizeFrontY {} {
        set yoff [$self cget -yoff]
        return [expr {$yoff - (($topouterwidth-$bottomouterwidth)/2.0)}]
    }
    method InsideBottomZ {} {
        set zoff [$self cget -zoff]
        if {[$self cget -upsidedown]} {
            return [expr {$zoff - $wallthickness}]
        } else {
            return [expr {$zoff + $wallthickness}]
        }
    }
    method TopZ {} {
        set zoff [$self cget -zoff]
        if {[$self cget -upsidedown]} {
            return [expr {$zoff - $outsideheight}]
        } else {
            return [expr {$zoff + $outsideheight}]
        }
    }
    method InsideWidth {} {return $insidewidth}
    method InsideLength {} {return $insidelength}
    method InsideHeight {} {return $insideheight}
    method OutsideWidth {} {return $topouterwidth}
    method OutsideLength {} {return $topouterlength}
    method BottomOuterWidth {} {return $bottomouterwidth}
    method BottomOuterLength {} {return $bottomouterlength}
    method WallThickness {} {
        if {[$self cget -upsidedown]} {
            return [expr {-1*$wallthickness}]
        } else {
            return $wallthickness
        }
    }
    typevariable lidholes { {310.4330708661417 310.4330708661417 0} 
        {4542.716535433071 310.4330708661417 0}
        {4542.716535433071 2967.9133858267714 0}
        {310.4330708661417  2967.9133858267714 0} }
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    option -upsidedown -type snit::boolean -default no -readonly yes
    option -includelid -type snit::boolean -default yes -readonly yes
    component bottom
    component left
    component right
    component front
    component back
    component lid
    component lidscrew0
    component lidscrew1
    component lidscrew2
    component lidscrew3
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set upsidedown [$self cget -upsidedown]
        set includelid [$self cget -includelid]
        set lowerBackX [expr {$xoff + $bottomouterlength}]
        set upperBackX [expr {$xoff + ($bottomouterlength+(($topouterlength-$bottomouterlength)/2.0))}]
        set lowerFrontX $xoff
        set upperFrontX [expr {$xoff - (($topouterlength-$bottomouterlength)/2.0)}]
        set upperLeftY [expr {$yoff - (($topouterwidth-$bottomouterwidth)/2.0)}]
        set lowerLeftY $yoff
        set upperRightY [expr {$yoff + ($bottomouterwidth+(($topouterwidth-$bottomouterwidth)/2.0))}]
        set lowerRightY [expr {$yoff + $bottomouterwidth}]
        set lowerZ $zoff
        if {$upsidedown} {
            set upperZ [expr {$zoff - $outsideheight}]
            set wallthicknessZ [expr {-1 * $wallthickness}]
            set screwZ [expr {$upperZ - $wallthickness}]
        } else {
            set upperZ [expr {$zoff + $outsideheight}]
            set wallthicknessZ $wallthickness
            set screwZ [expr {$upperZ + $wallthickness}]
        }
        install bottom using PrismSurfaceVector ${selfns}_bottom \
              -surface [PolySurface create ${selfns}_bottomSurf \
                        -rectangle yes \
                        -cornerpoint [list $lowerFrontX $lowerLeftY $lowerZ] \
                        -vec1 [list $bottomouterlength 0 0] \
                        -vec2 [list 0 $bottomouterwidth 0]] \
              -vector [list 0 0 $wallthicknessZ] -color $::black
        set leftPoly [list \
                      [list $lowerFrontX $lowerLeftY $lowerZ] \
                      [list $lowerBackX $lowerLeftY $lowerZ] \
                      [list $upperBackX $upperLeftY $upperZ] \
                      [list $upperFrontX $upperLeftY $upperZ] \
                      [list $lowerFrontX $lowerLeftY $lowerZ]]
        install left using PrismSurfaceVector ${selfns}_left \
              -surface [PolySurface create ${selfns}_leftSurf \
                        -rectangle no \
                        -polypoints $leftPoly] \
                        -vector [list 0 $wallthickness 0] -color $::black
        set rightPoly [list \
                       [list $lowerFrontX $lowerRightY $lowerZ] \
                       [list $lowerBackX $lowerRightY $lowerZ] \
                       [list $upperBackX $upperRightY $upperZ] \
                       [list $upperFrontX $upperRightY $upperZ] \
                       [list $lowerFrontX $lowerRightY $lowerZ]]
        install right using PrismSurfaceVector ${selfns}_right \
              -surface [PolySurface create ${selfns}_rightSurf \
                        -rectangle no \
                        -polypoints $rightPoly] \
              -vector [list 0 -$wallthickness 0] -color $::black
        set frontpoly [list \
                       [list $lowerFrontX $lowerLeftY $lowerZ] \
                       [list $upperFrontX $upperLeftY $upperZ] \
                       [list $upperFrontX $upperRightY $upperZ] \
                       [list $lowerFrontX $lowerRightY $lowerZ] \
                       [list $lowerFrontX $lowerLeftY $lowerZ]]
        install front using PrismSurfaceVector ${selfns}_front \
              -surface [PolySurface create ${selfns}_frontSurf \
                        -rectangle no \
                        -polypoints $frontpoly] \
              -vector [list $wallthickness 0 0] -color $::black
                       
        set backpoly [list \
                       [list $lowerBackX $lowerLeftY $lowerZ] \
                       [list $upperBackX $upperLeftY $upperZ] \
                       [list $upperBackX $upperRightY $upperZ] \
                       [list $lowerBackX $lowerRightY $lowerZ] \
                       [list $lowerBackX $lowerLeftY $lowerZ]]
        install back using PrismSurfaceVector ${selfns}_back \
              -surface [PolySurface create ${selfns}_backSurf \
                        -rectangle no \
                        -polypoints $backpoly] \
              -vector [list -$wallthickness 0 0] -color $::black
        if {$includelid} {
            install lid using PrismSurfaceVector ${selfns}_lid \
                  -surface [PolySurface create ${selfns}_lidSurf \
                            -rectangle yes \
                            -cornerpoint [list $upperFrontX $upperLeftY $upperZ] \
                            -vec1 [list $topouterlength 0 0] \
                            -vec2 [list 0 $topouterwidth 0]] \
                  -vector [list 0 0 $wallthicknessZ] -color $::black
            for {set s 0} {$s < 4} {incr s} {
                lassign [lindex $lidholes $s] XX YY ZZ
                install lidscrew$s  using cylinder ${selfns}_lidscrew$s \
                      -bottom [list [expr {$upperFrontX + $XX}] \
                               [expr {$upperLeftY + $YY}] \
                               [expr {$screwZ +$ZZ}]] \
                      -radius 62.5 -height -$wallthicknessZ -color $::white
            }
        }
    }
    method LidHole {index {height 0} {Z 0} {name {}}} {
        if {$name eq {}} {set name ${selfns}_lidscrew$index}
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set upsidedown [$self cget -upsidedown]
        set upperFrontX [expr {$xoff - (($topouterlength-$bottomouterlength)/2.0)}]
        set upperLeftY [expr {$yoff - (($topouterwidth-$bottomouterwidth)/2.0)}]
        if {$upsidedown} {
            set upperZ [expr {$zoff - $outsideheight}]
            set wallthicknessZ [expr {-1 * $wallthickness}]
            set screwZ [expr {$upperZ - $wallthickness + $Z}]
        } else {
            set upperZ [expr {$zoff + $outsideheight}]
            set wallthicknessZ $wallthickness
            set screwZ [expr {$upperZ + $wallthickness + $Z}]
        }
        if {$height eq 0} {
            set height -$wallthicknessZ
        }
        lassign [lindex $lidholes $index] XX YY ZZ
        return [cylinder $name \
                -bottom [list [expr {$upperFrontX + $XX}] \
                         [expr {$upperLeftY + $YY}] \
                         [expr {$screwZ +$ZZ}]] \
                -radius 62.5 -height $height -color $::white]
    }
    method printPSLid {fp} {
        if {$options(-includelid)} {
            set lidsurf [$lid cget -surface]
            $lidsurf printPS $fp
            for {set s 0} {$s < 4} {incr s} {
                [set lidscrew$s] printPS $fp 
            }
        }
    }
    method printPSLeft  {fp} {
        if {[$self cget -upsidedown]} {
            set yscale -.001
        } else {
            set yscale .001
        }
        [$left cget -surface] printPS $fp 1 2 0 0 .001 $yscale
    }
    method printPSRight  {fp} {
        if {[$self cget -upsidedown]} {
            set yscale -.001
        } else {
            set yscale .001
        }
        [$right cget -surface] printPS $fp 1 2 0 0 .001 $yscale
    }
    method printPSFront {fp} {
        if {[$self cget -upsidedown]} {
            set yscale -.001
        } else {
            set yscale .001
        }
        [$front cget -surface] printPS $fp 1 2 0 0 .001 $yscale
    }
    method printPSBack {fp} {
        if {[$self cget -upsidedown]} {
            set yscale -.001
        } else {
            set yscale .001
        }
        [$back cget -surface] printPS $fp 1 2 0 0 .001 $yscale
    }
    method printPSBottom {fp} {
        [$bottom cget -surface] printPS $fp
    }
    method print {{fp stdout}} {
        $bottom print $fp
        $left print $fp
        $right print $fp
        $front print $fp
        $back print $fp
        if {[$self cget -includelid]} {
            $lid print $fp
            $lidscrew0 print $fp
            $lidscrew1 print $fp
            $lidscrew2 print $fp
            $lidscrew3 print $fp
        }
    }
}


snit::type TFT_Display {
    typevariable boardlength 2200
    typevariable boardwidth  1350
    typevariable holes { {100 100 0} {2100 100 0} {2100 1250 0} {100 1250 0} }
    typevariable displaylength 1400;# guess (roundup)
    typevariable displaywidth  1125;# guess (roundup)
    typevariable displayxoff    400;# guess
    typevariable displayyoff    100;# guess
    typemethod BoardLength {} {return $boardlength}
    typemethod BoardWidth  {} {return $boardwidth}
    component breakoutboard
    component display
    component mhole0
    component mhole1
    component mhole2
    component mhole3
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        install breakoutboard using PrismSurfaceVector \
              ${selfns}DisplayBreakout \
              -surface [PolySurface create ${selfns}DisplayBreakoutSurf \
                        -rectangle yes \
                        -cornerpoint [list $xoff $yoff $zoff] \
                        -vec1 [list $boardlength 0 0] \
                        -vec2 [list 0 $boardwidth 0]] \
              -vector [list 0 0 100] -color $::blue
        install display using PrismSurfaceVector ${selfns}Display \
              -surface [PolySurface create ${selfns}DisplaySurf \
                        -rectangle yes \
                        -cornerpoint [list [expr {$xoff + $displayxoff}] \
                                      [expr {$yoff + $displayyoff}] \
                                      [expr {$zoff + 100}]] \
                        -vec1 [list $displaylength 0 0] \
                        -vec2 [list 0 $displaywidth 0]] \
              -vector [list 0 0 200] -color $::white
        for {set h 0} {$h < 4} {incr h} {
            set p [lindex $holes $h]
            lassign $p x y z
            install mhole$h using cylinder ${selfns}MHole$h \
                  -bottom [list [expr {$xoff + $x}] \
                           [expr {$yoff + $y}] \
                           [expr {$zoff + $z}]] \
                  -radius 62.5 -height 100 -color $::white
        }
    }
    method print {{fp stdout}} {
        $breakoutboard print $fp
        $display print $fp
        $mhole0 print $fp
        $mhole1 print $fp
        $mhole2 print $fp
        $mhole3 print $fp
    }
    method printPSDisplayCutout {fp {xi 0} {yi 1} {xorg 0} {yorg 0} {scale .001}} {
        set displaySurf [$display cget -surface]
        $displaySurf printPS $fp $xi $yi $xorg $yorg $scale
        $mhole0 printPS $fp $xi $yi $xorg $yorg $scale
        $mhole1 printPS $fp $xi $yi $xorg $yorg $scale
        $mhole2 printPS $fp $xi $yi $xorg $yorg $scale
        $mhole3 printPS $fp $xi $yi $xorg $yorg $scale
    }
    method MountingHole {index zMount height {name %AUTO%}} {
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set p [lindex $holes $index]
        lassign $p x y z
        return [cylinder ${name} \
                -bottom [list [expr {$xoff + $x}] \
                         [expr {$yoff + $y}] \
                         [expr {$zoff + $z + $zMount}]] \
                -radius 62.5 -height $height -color $::white]
    }
}

snit::type LargePiezo {
    typevariable outerdiameter
    typevariable holes {}
    typevariable mholeradius
    typevariable totalthickness
    typevariable flangethickness
    typevariable flangewidth
    typevariable flangelength
    typevariable flangeYoffset
    typeconstructor {
        set outerdiameter [MM2Mils 30]
        set holes [list [list [expr {$outerdiameter / 2.0}] [MM2Mils -2.5] 0] \
                           [list [expr {$outerdiameter / 2.0}] [MM2Mils [expr {$outerdiameter + 2.5}]] 0] ]
        set flangewidth [MM2Mils 5]
        set flangelength [MM2Mils 41]
        set flangeYoffset [expr {($flangelength - $outerdiameter) / 2.0}]
        set flangethickness [MM2Mils 3.2]
        set totalthickness [MM2Mils 5.7]
        set mholeradius [expr {[MM2Mils 1.5] / 2.0}]
    }
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    component body
    component flange
    component mhole0
    component mhole1
    method MountingHole {index {zMount 0} {height 0} {name {}}} {
        if {$name eq {}} {
            set name ${selfns}Mhole$index
        }
        if {$height eq 0} {set height [expr {$flangethickness * -1}]}
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set p [lindex $holes $index]
        lassign $p x y z
        return [cylinder ${name} \
                -bottom [list [expr {$xoff + $x}] \
                         [expr {$yoff + $y}] \
                         [expr {$zoff + $z + $zMount}]] \
                -radius $mholeradius -height $height -color $::white]
    }
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        install body using cylinder ${selfns}Body \
              -bottom [list $xoff $yoff [expr {$zoff - $flangethickness}]] \
              -radius [expr {$outerdiameter / 2.0}] \
              -height $totalthickness -color $::black
        install flange using PrismSurfaceVector create ${selfns}Flange \
              -surface [PolySurface create ${selfns}FlangeSurf \
                        -rectangle yes \
                        -cornerpoint [list [expr {$xoff - ($flangewidth / 2.0)}] \
                                      [expr {$yoff + $flangeYoffset}] \
                                      $zoff] \
                        -vec1 [list $flangewidth 0 0] \
                        -vec2 [list 0 $flangelength 0]] \
              -vector [list 0 0 [expr {$flangethickness * -1}]] -color $::black
        set mhole0 [$self MountingHole 0]
        set mhole1 [$self MountingHole 1]
    }
    method print {{fp stdout}} {
        $body print $fp
        $flange print $fp
        $mhole0 print $fp
        $mhole1 print $fp
    }
}
        
snit::type PushButtonSwitchHole {
    typevariable radius [expr {290 / 2.0}]
    component hole
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    option -height -type snit::double -default 0 -readonly yes
    option -direction -default Z -type {snit::enum -values {X Y Z}} -readonly yes
    delegate method * to hole
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set height [$self cget -height]
        set direction [$self cget -direction]
        install hole using cylinder ${selfns}Hole \
              -bottom [list $xoff $yoff $zoff] \
              -height $height -direction $direction -color $::white \
              -radius $radius
    }
}

snit::type D2425 {
    typevariable width 1750
    typevariable length 2250
    typevariable height 890
    typevariable holes { {185 875 0} {2065 875 0} }
    typevariable hradius [expr {190 / 2.0}]
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    component relay
    component mhole0
    component mhole1
    method MountingHole {index {h 890} {Z 0} {name {}}} {
        if {$name eq {}} {
            set name ${selfns}Mhole$index
        }
        set p [lindex $holes $index]
        lassign $p x y z
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set x [expr {$x + $xoff}]
        set y [expr {$y + $yoff}]
        set z [expr {$Z + $zoff + $Z}]
        return [cylinder $name \
                -bottom [list $x $y $z] \
                -height $h \
                -radius $hradius \
                -color $::white]
    }
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        install relay using PrismSurfaceVector ${selfns}_relay \
              -surface [PolySurface create ${selfns}_relaySurf \
                        -rectangle yes \
                        -cornerpoint [list $xoff $yoff $zoff] \
                        -vec1 [list $length 0 0] \
                        -vec2 [list 0 $width 0]] \
              -vector [list 0 0 $height] -color $::black
        set mhole0 [$self MountingHole 0]
        set mhole1 [$self MountingHole 1]
    }
    method print {{fp stdout}} {
        $relay print $fp
        $mhole0 print $fp
        $mhole1 print $fp
    }
}

snit::type ACPowerEntry {
    typevariable flangewidthmax 1970
    typemethod FlangeWidthMax {} {return $flangewidthmax}
    typevariable flangeheightmax 880
    typemethod FlangeHeightMax {} {return $flangeheightmax}
    typevariable flangeholes { {0 197.5 440} {0 1772.5 440} }
    typevariable flangeholeradius [expr {138.0/2.0}]
    typevariable cutoutYoff [expr {(1970-1080)/2.0}]
    typevariable cutoutZoff [expr {(880-790)/2.0}]
    typevariable cutoutwidth 1080
    typevariable cutoutheight 790
    typevariable flangeXoff 80
    typevariable flangeXdepth 120
    typevariable totaldepthX 1120
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    option -end  -type {snit::enum -values {front back}} -default front -readonly yes
    option -orientation -type {snit::enum -values {horizontal vertical}} -default horizontal -readonly yes
    component body
    component flange
    component mhole0
    component mhole1
    method MountingHole {index {d 0} {X 0} {name {}}} {
        if {$name eq {}} {
            set name ${selfns}Mhole$index
        }
        if {$d == 0} {set d $flangeXdepth}
        set p [lindex $flangeholes $index]
        lassign $p x y z
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set end  [$self cget -end]
        if {$end eq "back"} {
            set d [expr {$d * -1}]
        }
        set x [expr {$x + $xoff + $X}]
        if {[$self cget -orientation] eq "horizontal"} {
            set _y [expr {$y + $yoff}]
            set _z [expr {$z + $zoff}]
        } else {
            set _y [expr {$z + $yoff}]
            set _z [expr {$y + $zoff}]
        }
        return [cylinder $name \
                -bottom [list $x $_y $_z] \
                -height $d \
                -radius $flangeholeradius \
                -direction X \
                -color $::white]
    }
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set end  [$self cget -end]
        switch $end {
            front {
                set _flangeXoff $flangeXoff
                set _totaldepthX $totaldepthX
                set _flangeXdepth $flangeXdepth
            }
            back {
                set _flangeXoff [expr {$flangeXoff * -1}]
                set _totaldepthX [expr {$totaldepthX * -1}]
                set _flangeXdepth [expr {$flangeXdepth * -1}]
            }
        }
        switch [$self cget -orientation] {
            horizontal {
                set _cutoutYoff $cutoutYoff
                set _cutoutZoff $cutoutZoff
                set _cutoutwidth $cutoutwidth
                set _cutoutheight $cutoutheight
                set _flangewidthmax $flangewidthmax
                set _flangeheightmax $flangeheightmax
            }
            vertical {
                set _cutoutYoff $cutoutZoff
                set _cutoutZoff $cutoutYoff
                set _cutoutwidth $cutoutheight
                set _cutoutheight $cutoutwidth
                set _flangewidthmax $flangeheightmax
                set _flangeheightmax $flangewidthmax
            }
        }
        #puts stderr "*** $type create $self: _cutoutwidth = $_cutoutwidth, cutoutheight = $cutoutheight, _cutoutheight = $_cutoutheight, cutoutwidth = $cutoutwidth"
        
        install body using PrismSurfaceVector ${selfns}_body \
              -surface [PolySurface create ${selfns}_bodySurf \
                        -rectangle yes \
                        -cornerpoint [list \
                                      [expr {$xoff - $_flangeXoff}] \
                                      [expr {$yoff + $_cutoutYoff}] \
                                      [expr {$zoff + $_cutoutZoff}]] \
                        -vec1 [list 0 $_cutoutwidth 0] \
                        -vec2 [list 0 0 $_cutoutheight]] \
              -vector [list $_totaldepthX 0 0] -color $::black
        install flange using PrismSurfaceVector ${selfns}_flange \
              -surface [PolySurface create ${selfns}_flangeSurf \
                        -rectangle yes \
                        -cornerpoint [list $xoff $yoff $zoff] \
                        -vec1 [list 0 $_flangewidthmax 0] \
                        -vec2 [list 0 0 $_flangeheightmax]] \
              -vector [list $_flangeXdepth 0 0] -color $::black
        set mhole0 [$self MountingHole 0]
        set mhole1 [$self MountingHole 1]
    }
    method print {{fp stdout}} {
        $body print $fp
        $flange print $fp
        $mhole0 print $fp
        $mhole1 print $fp
    }
    method CutoutSurf {} {
        return [$body cget -surface]
    }
}

snit::type ACReceptacle {
    typevariable totalwidth 1062.992125984252
    typemethod TotalWidth {} {return $totalwidth}
    typevariable totalheight 1062.992125984252
    typemethod TotalHeight {} {return $totalheight}
    typevariable totaldepth 1290.1574803149608
    typevariable bodywidth 1023.6220472440945
    typevariable bodyheight 866.1417322834645
    typevariable bodyYoff 19.68503937007874
    typevariable bodyZoff 157.48031496062993
    typevariable flangeXoffset -141.73228346456693
    typevariable holepolygon {}
    typeconstructor {
        set cutoutWidth [MM2Mils 26]
        set cutoutHeight [MM2Mils 22]
        set lowerTabWidth [MM2Mils 19]
        set upperTabWidth [MM2Mils 9.1]
        set middleTabHeight [MM2Mils 10.6]
        set lowerTabHeight [MM2Mils 5.75]
        set upperTabHeight [MM2Mils [expr {22 - (10.6 + 5.75)}]]
        #puts stderr "*** $type typeconstructor: cutoutWidth = $cutoutWidth, cutoutHeight = $cutoutHeight"
        #puts stderr "*** $type typeconstructor: lowerTabWidth = $lowerTabWidth, upperTabWidth = $upperTabWidth"
        #puts stderr "*** $type typeconstructor: middleTabHeight = $middleTabHeight"
        #puts stderr "*** $type typeconstructor: lowerTabHeight = $lowerTabHeight"
        #puts stderr "*** $type typeconstructor: upperTabHeight = $upperTabHeight"
        set holepolygon [list]
        set dy1 [expr {($totalwidth-$lowerTabWidth)/2.0}]
        set dz1 [expr {($totalheight-$cutoutHeight)/2.0}]
        #puts stderr "*** $type typeconstructor: dy1 = $dy1, dz1 = $dz1"
        lappend holepolygon [list 0 $dy1 $dz1]
        set dz2 [expr {$dz1 + $lowerTabHeight}]
        #puts stderr "*** $type typeconstructor: dz2 = $dz2"
        lappend holepolygon [list 0 $dy1 $dz2]
        set dy2 [expr {($totalwidth-$cutoutWidth) / 2.0}]
        #puts stderr "*** $type typeconstructor: dy2 = $dy2"
        lappend holepolygon [list 0 $dy2 $dz2]
        set dz3 [expr {$dz2 + $middleTabHeight}]
        #puts stderr "*** $type typeconstructor: dz3 = $dz3"
        lappend holepolygon [list 0 $dy2 $dz3]
        set dy3 [expr {($totalwidth-$upperTabWidth) / 2.0}]
        #puts stderr "*** $type typeconstructor: dy3 = $dy3"
        lappend holepolygon [list 0 $dy3 $dz3]
        set dz4 [expr {$dz3 + $upperTabHeight}]
        #puts stderr "*** $type typeconstructor: dz4 = $dz4"
        lappend holepolygon [list 0 $dy3 $dz4]
        set dy4 [expr {$dy3 + $upperTabWidth}]
        #puts stderr "*** $type typeconstructor: dy4 = $dy4"
        lappend holepolygon [list 0 $dy4 $dz4]
        lappend holepolygon [list 0 $dy4 $dz3]
        set dy5 [expr {$dy2 + $cutoutWidth}]
        #puts stderr "*** $type typeconstructor: dy5 = $dy5"
        lappend holepolygon [list 0 $dy5 $dz3]
        lappend holepolygon [list 0 $dy5 $dz2]
        set dy6 [expr {$dy1 + $lowerTabWidth}]
        #puts stderr "*** $type typeconstructor: dy6 = $dy6"
        lappend holepolygon [list 0 $dy6 $dz2]
        lappend holepolygon [list 0 $dy6 $dz1]
        lappend holepolygon [list 0 $dy1 $dz1]
    }
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    option -end  -type {snit::enum -values {front back}} -default front -readonly yes
    component body
    component flange
    method MountHole {{d 0} {X 0} {color {}} {name {}}} {
        if {$name eq {}} {
            set name ${selfns}Mhole
        }
        if {$color eq {}} {
            set color $::white
        }
        set mpoly [list]
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set end  [$self cget -end]
        if {$end eq "back"} {
            set d [expr {$d * -1}]
        }
        foreach p $holepolygon {
            lassign $p x y z
            lappend mpoly [list [expr {$x + $xoff + $X}] \
                           [expr {$y + $yoff}] \
                           [expr {$z + $zoff}]]
        }
        return [PrismSurfaceVector $name \
                -surface [PolySurface create ${name}Surf \
                          -rectangle no \
                          -polypoints $mpoly] \
                -vector [list $d 0 0] -color $color]
    }
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set end  [$self cget -end]
        switch $end {
            front {
                set _flangeXoffset [expr {$flangeXoffset * -1}]
                set _totaldepth $totaldepth
            }
            back {
                set _flangeXoffset $flangeXoffset
                set _totaldepth [expr {$totaldepth * -1}]
            }
        }
        install body using PrismSurfaceVector ${selfns}Body \
              -surface [PolySurface create ${selfns}BodySurf \
                        -rectangle yes \
                        -cornerpoint [list $xoff \
                                      [expr {$yoff + $bodyYoff}] \
                                      [expr {$zoff + $bodyZoff}]] \
                        -vec1 [list 0 $bodywidth 0] \
                        -vec2 [list 0 0 $bodyheight]] \
              -vector [list [expr {$_totaldepth - $_flangeXoffset}] 0 0] \
              -color  $::black
        install flange using PrismSurfaceVector ${selfns}Flange \
              -surface [PolySurface create ${selfns}FlangeSurf \
                        -rectangle yes \
                        -cornerpoint [list [expr {$xoff - $_flangeXoffset}] \
                                      $yoff $zoff] \
                        -vec1 [list 0 $totalwidth 0] \
                        -vec2 [list 0 0 $totalheight]] \
              -vector [list $_flangeXoffset 0 0] \
              -color  $::black
    }
    method print {{fp stdout}} {
        $body print $fp
        $flange print $fp
    }
}

snit::type LargePiezo {
    typevariable outerdiameter
    typevariable holespacing
    typevariable holes {}
    typevariable mholeradius
    typevariable totalthickness
    typevariable flangethickness
    typevariable flangewidth
    typevariable flangelength
    typevariable flangeYoffset
    typeconstructor {
        set outerdiameter [MM2Mils 30]
        set holespacing [MM2Mils 35]
        set holes [list [list 0 [expr {-($holespacing/2.0)}] 0] \
              [list 0 [expr {($holespacing/2.0)}] 0]]
        set flangewidth [MM2Mils 5]
        set flangelength [MM2Mils 41]
        set flangeYoffset [expr {(-($outerdiameter/2.0))-(($flangelength - $outerdiameter) / 2.0)}]
        set flangethickness [MM2Mils 3.2]
        set totalthickness [MM2Mils 5.7]
        set mholeradius [expr {[MM2Mils 1.5] / 2.0}]
    }
    option -xoff -type snit::double -default 0 -readonly yes
    option -yoff -type snit::double -default 0 -readonly yes
    option -zoff -type snit::double -default 0 -readonly yes
    component body
    component flange
    component mhole0
    component mhole1
    method MountingHole {index {zMount 0} {height 0} {name {}}} {
        if {$name eq {}} {
            set name ${selfns}Mhole$index
        }
        if {$height eq 0} {set height [expr {$flangethickness * -1}]}
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        set p [lindex $holes $index]
        lassign $p x y z
        return [cylinder ${name} \
                -bottom [list [expr {$xoff + $x}] \
                         [expr {$yoff + $y}] \
                         [expr {$zoff + $z + $zMount}]] \
                -radius $mholeradius -height $height -color $::white]
    }
    constructor {args} {
        $self configurelist $args
        set xoff [$self cget -xoff]
        set yoff [$self cget -yoff]
        set zoff [$self cget -zoff]
        install body using cylinder ${selfns}Body \
              -bottom [list $xoff $yoff [expr {$zoff - $flangethickness}]] \
              -radius [expr {$outerdiameter / 2.0}] \
              -height $totalthickness -color $::black
        install flange using PrismSurfaceVector create ${selfns}Flange \
              -surface [PolySurface create ${selfns}FlangeSurf \
                        -rectangle yes \
                        -cornerpoint [list [expr {$xoff - ($flangewidth / 2.0)}] \
                                      [expr {$yoff + $flangeYoffset}] \
                                      $zoff] \
                        -vec1 [list $flangewidth 0 0] \
                        -vec2 [list 0 $flangelength 0]] \
              -vector [list 0 0 [expr {$flangethickness * -1}]] -color $::black
        set mhole0 [$self MountingHole 0]
        set mhole1 [$self MountingHole 1]
    }
    method print {{fp stdout}} {
        $body print $fp
        $flange print $fp
        $mhole0 print $fp
        $mhole1 print $fp
    }
    method printPS {fp {xi 0} {yi 1} {xorg 0} {yorg 0} {scale .001}} {
        $body printPS $fp $xi $yi $xorg $yorg $scale
        $mhole0 printPS $fp $xi $yi $xorg $yorg $scale
        $mhole1 printPS $fp $xi $yi $xorg $yorg $scale
    }
        
}
        
snit::type PostScriptFile {
    pragma -hastypeinfo    no
    pragma -hastypedestroy no
    pragma -hasinstances   no
    typevariable pageno 0
    typevariable psName {}
    typevariable psFP   {}
    typevariable psTitle {}
    typevariable psCreator {}
    typevariable psPages 0
    typemethod open {} {
        set psName  [file rootname [info script]].ps
        set psFP    [open $psName w]
        set psTitle [file rootname [file tail [info script]]]
        set psCreator [file tail [info script]]
        set pageno 0
        set psPages 0
        puts $psFP "%!PS-Adobe-3.0"
        puts $psFP [format {%%Title: %s} $psTitle]
        puts $psFP [format {%%Creator: %s} $psCreator]
        puts $psFP {%%BoundingBox: 0 0 612 792}
        puts $psFP {%%Pages: (atend)}
        puts $psFP {%%EndComments}
        puts $psFP {%%BeginProlog}
        puts $psFP {/inch {72 mul} def}
        puts $psFP {%%EndProlog}
        puts $psFP {}
    }
    typemethod newPage {{pageTitle {}}} {
        if {$pageno > 0} {
            puts $psFP {showpage}
        }
        incr pageno
        puts $psFP [format {%%%%Page: %d %d} $pageno $pageno]
        puts $psFP {1.5 inch 1.5 inch translate 1 inch 1 inch scale}
        puts $psFP {}
        puts $psFP [format {gsave 0 -.75 moveto /NewCenturySchlbk-Bold findfont .25 scalefont setfont 0 0 0 setrgbcolor (%s) show grestore} $pageTitle]
    }
    typemethod fp {} {return $psFP}
    typemethod close {} {
        if {$pageno > 0} {
            puts $psFP {showpage}
        }
        puts $psFP {%%Trailer}
        puts $psFP [format {%%%%Pages: %d} $pageno]
        puts $psFP {%%EOF}
        close $psFP
        set psFP {}
    }
}



puts {MODSIZ 15000 0.2 2
DEFTX -200 -200 1 0
DEFCOL 255 0 0
VIEW 0.57735 -0.57735 0.57735 858.438369 1040.080534 194.155527 5.163929 -0.408248 0.408248 0.816497
CONST_PL P(0 0) DX DY DZ 0 RZ
:DYNAMIC_DATA
DEFCOL 0 0 0}


set upperbox [ham_1591XXTSBK create UpperBox -includelid yes]
$upperbox print

set upperboxCenterY [expr {[$upperbox InsideFrontY] + ([$upperbox InsideWidth]/2.0)}]
set MOY [expr {$upperboxCenterY - ([ReflowToasterOven2Board BoardWidth] / 2.0)}]
set MOX [$upperbox InsideFrontX]
set MOZ [expr {[$upperbox InsideBottomZ] + [MM2Mils 6.0]}]

set mo [ReflowToasterOven2Board create mo -xoff $MOX -yoff $MOY -zoff $MOZ]
$mo print

for {set s 0} {$s < 4} {incr s} {
    set moStandoff$s [$mo StandOff $s 6]
    [set moStandoff$s] print
}

for {set h 0} {$h < 4} {incr h} {
    set upperMoMountingHole$h [$mo mountingHole $h ${upperbox}_MoMountingHole [expr {[$upperbox WallThickness] * -1}] -6]
    [set upperMoMountingHole$h] print
}

set lowerbox [ham_1591XXTSBK create LowerBox -includelid no -upsidedown yes]
$lowerbox print

for {set h 0} {$h < 4} {incr h} {
    set lowerMoMountingHole$h [$mo mountingHole $h ${lowerbox}_MoMountingHole [$lowerbox WallThickness] -8]
    [set lowerMoMountingHole$h] print
}

set boxLidCenterY [expr {[$upperbox OutsizeFrontY] + ([$upperbox OutsideWidth]/2.0)}]
set boxInnerRightX [expr {[$upperbox InsideFrontX] + [$upperbox InsideLength]}]
set displayX [expr {$boxInnerRightX - [TFT_Display BoardLength]}]
set displayY [expr {$boxLidCenterY - ([TFT_Display BoardWidth]/2.0)}]
set displayZ [expr {[$upperbox TopZ] - 100}]

set display [TFT_Display Display -xoff $displayX -yoff $displayY -zoff $displayZ]
$display print

for {set h 0} {$h < 4} {incr h} {
    set displaymhole$h [$display MountingHole $h 100 [$upperbox WallThickness] ${upperbox}DisplayMHole$h]
    [set displaymhole$h] print
}

#-----------

set switchesZ [$upperbox TopZ]
set switchesHeight [$upperbox WallThickness]
set switchesX [expr {$displayX - 500}]
set switch1Y [expr {$boxLidCenterY + 750}]
set switch2Y $boxLidCenterY
set switch3Y [expr {$boxLidCenterY - 750}]

set switch1 [PushButtonSwitchHole Switch1 -xoff $switchesX -yoff $switch1Y \
             -zoff $switchesZ -height $switchesHeight]
$switch1 print
set switch2 [PushButtonSwitchHole Switch2 -xoff $switchesX -yoff $switch2Y \
             -zoff $switchesZ -height $switchesHeight]
$switch2 print
set switch3 [PushButtonSwitchHole Switch3 -xoff $switchesX -yoff $switch3Y \
             -zoff $switchesZ -height $switchesHeight]
$switch3 print

set buzzer [LargePiezo buzzer -yoff $boxLidCenterY -zoff [$upperbox TopZ] \
            -xoff [expr {[$upperbox InsideFrontX] + ([MM2Mils 32]/2.0)}]]
$buzzer print

set buzzerMHole0 [$buzzer MountingHole 0 0 [MM2Mils -2] buzzerMHole0]
$buzzerMHole0 print
set buzzerMHole1 [$buzzer MountingHole 1 0 [MM2Mils -2] buzzerMHole1]
$buzzerMHole1 print

set fanRelayWireHole [cylinder FanRelayWireHole \
                       -bottom [list \
                                [expr {$MOX + ([ReflowToasterOven2Board BoardLength] / 2.0)}] \
                                $boxLidCenterY \
                                [$upperbox WallThickness]] \
                       -radius [expr {375 / 2.0}] \
                       -height [expr {[$upperbox WallThickness] * -2.0}] \
                       -color  $white]
$fanRelayWireHole print

set fanWireHole [cylinder FanWireHole \
                 -bottom [list \
                          [expr {$MOX + ([ReflowToasterOven2Board BoardLength] / 2.0)}] \
                          [expr {$boxLidCenterY + 750}] \
                          [$lowerbox TopZ]] \
                 -radius [expr {250/2.0}] \
                 -height -125 -color  $white]
$fanWireHole print

set thermocoupleHole [cylinder ThermocoupleHole \
                      -bottom [list \
                               [expr {[$upperbox InsideFrontX] + [$upperbox InsideLength] + ([$upperbox WallThickness] / 2.0)}] \
                               $boxLidCenterY \
                               [expr {[$upperbox TopZ] / 2.0}]] \
                      -radius 125 \
                      -height [$upperbox WallThickness] \
                      -direction X \
                      -color  $white]
$thermocoupleHole print

set relayX [expr {[$lowerbox InsideFrontX] + [MM2Mils 28.288] + 185 - 500}]
set relayY [expr {[$lowerbox InsideFrontY] + [MM2Mils 32.085] - 875 - 250}]
set relay [D2425 Relay -xoff $relayX -yoff $relayY -zoff [$lowerbox TopZ]]
$relay print

set hs_relaymounthole0 [$relay MountingHole 0 -125 0 HS_RelayMountHole0]
$hs_relaymounthole0 print
set hs_relaymounthole1 [$relay MountingHole 1 -125 0 HS_RelayMountHole1]
$hs_relaymounthole1 print

for {set l 0} {$l < 4} {incr l} {
    set hs_lidhole$l [$lowerbox LidHole $l -125 [MM2Mils 2] HS_LidHole$l]
    [set hs_lidhole$l] print
}

lassign [$hs_relaymounthole0 cget -bottom] \
      hs_relaymounthole0_x hs_relaymounthole0_y hs_relaymounthole0_z

set heatsinkpanel [PrismSurfaceVector create HeatsinkPanel \
                   -surface [PolySurface create HeatsinkPanelSurf \
                             -rectangle yes \
                             -cornerpoint [list \
                                           [expr {$hs_relaymounthole0_x - [MM2Mils 28.288]}] \
                                           [expr {$hs_relaymounthole0_y - [MM2Mils 32.085]}] \
                                           [$lowerbox TopZ]] \
                             -vec1 [list [MM2Mils 120] 0 0] \
                             -vec2 [list 0 [MM2Mils [expr {200.938 - (2*41.235)}]] 0]] \
                   -vector {0 0 -125} -color $::yellow]
$heatsinkpanel print

set acpowerentry [ACPowerEntry ACPower -xoff [$lowerbox InsideFrontX] \
                  -yoff [expr {[$lowerbox InsideFrontY] + [$lowerbox InsideWidth] - [ACPowerEntry FlangeHeightMax] - 62.5}] \
                  -zoff [expr {[$lowerbox cget -zoff] + [$lowerbox WallThickness] - [ACPowerEntry FlangeWidthMax] - 62.5}] \
                  -orientation vertical]
$acpowerentry print

set acpowerentry_m1 [$acpowerentry MountingHole 0 [$lowerbox WallThickness] 0 acpowerentry_m1] 
set acpowerentry_m2 [$acpowerentry MountingHole 1 [$lowerbox WallThickness] 0 acpowerentry_m2] 

$acpowerentry_m1 print
$acpowerentry_m2 print

set backrecept [ACReceptacle BackReeptacle \
                -xoff [expr {[$lowerbox InsideFrontX] + [$lowerbox InsideLength] - [$lowerbox WallThickness]}] \
                -yoff [expr {[$lowerbox InsideFrontY] + 62.5}] \
                -zoff [expr {[$lowerbox cget -zoff] + [$lowerbox WallThickness] - [ACReceptacle TotalHeight] - 62.5}] \
                -end back]
$backrecept print
set backreceptMHole [$backrecept MountHole [expr {[$lowerbox WallThickness] * -1}] 0 $::white BackReeptacleMountHole]
$backreceptMHole print


PostScriptFile open

PostScriptFile newPage {Upper Lid}

$upperbox printPSLid [PostScriptFile fp]
$display printPSDisplayCutout [PostScriptFile fp]
$switch1 printPS [PostScriptFile fp]
$switch2 printPS [PostScriptFile fp]
$switch3 printPS [PostScriptFile fp]
$buzzer printPS [PostScriptFile fp]

PostScriptFile newPage {Upper Box Right End}

$upperbox printPSFront [PostScriptFile fp]
set usb_cutout [$mo USBJackCutout] 
$usb_cutout printPS [PostScriptFile fp] 1 2 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] 0 -.001

PostScriptFile newPage {Upper Box Left End}

$upperbox printPSBack [PostScriptFile fp]

$thermocoupleHole printPS [PostScriptFile fp] 1 2

PostScriptFile newPage {Upper Box Bottom}

$upperbox printPSBottom [PostScriptFile fp]

$fanRelayWireHole printPS [PostScriptFile fp] 0 1 0 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] .001 -.001
$upperMoMountingHole0 printPS [PostScriptFile fp] 0 1 0  [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] .001 -.001
$upperMoMountingHole1 printPS [PostScriptFile fp] 0 1 0 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] .001 -.001
$upperMoMountingHole2 printPS [PostScriptFile fp] 0 1 0 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] .001 -.001
$upperMoMountingHole3 printPS [PostScriptFile fp] 0 1 0 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] .001 -.001

PostScriptFile newPage {Lower Box Bottom}

$lowerbox printPSBottom [PostScriptFile fp]

$fanRelayWireHole printPS [PostScriptFile fp] 
$upperMoMountingHole0 printPS [PostScriptFile fp] 
$upperMoMountingHole1 printPS [PostScriptFile fp] 
$upperMoMountingHole2 printPS [PostScriptFile fp] 
$upperMoMountingHole3 printPS [PostScriptFile fp] 

PostScriptFile newPage {Lower Box Right End}

$lowerbox printPSFront [PostScriptFile fp]

[$acpowerentry CutoutSurf] printPS [PostScriptFile fp] 1 2 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] 0 -.001 -.001
$acpowerentry_m1 printPS [PostScriptFile fp] 1 2 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] 0 -.001 -.001
$acpowerentry_m2 printPS [PostScriptFile fp] 1 2 [expr {([$upperbox cget -yoff]+[$upperbox BottomOuterWidth])/1000.0}] 0 -.001 -.001

PostScriptFile newPage {Lower Box Left End}

$lowerbox printPSBack [PostScriptFile fp]
[$backreceptMHole cget -surface]  printPS [PostScriptFile fp] 1 2 0 0 .001 -.001

PostScriptFile newPage {Heat Sink Holes}

[$heatsinkpanel cget -surface] printPS [PostScriptFile fp]

$hs_relaymounthole0 printPS [PostScriptFile fp] 0 1 [expr {([$lowerbox cget -xoff]+[$lowerbox BottomOuterLength])/1000.0}] 0 -.001
$hs_relaymounthole1 printPS [PostScriptFile fp] 0 1 [expr {([$lowerbox cget -xoff]+[$lowerbox BottomOuterLength])/1000.0}] 0 -.001
for {set l 0} {$l < 4} {incr l} {
    [set hs_lidhole$l] printPS [PostScriptFile fp] 0 1 [expr {([$lowerbox cget -xoff]+[$lowerbox BottomOuterLength])/1000.0}] 0 -.001
}
$fanWireHole printPS [PostScriptFile fp] 0 1 [expr {([$lowerbox cget -xoff]+[$lowerbox BottomOuterLength])/1000.0}] 0 -.001

PostScriptFile close

