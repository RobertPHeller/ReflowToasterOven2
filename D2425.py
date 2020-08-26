#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Tue Aug 25 10:09:48 2020
#  Last Modified : <200825.1025>
#
#  Description	
#
#  Notes
#
#  History
#	
#*****************************************************************************
#
#    Copyright (C) 2020  Robert Heller D/B/A Deepwoods Software
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


import Part
from FreeCAD import Base
import FreeCAD as App
import os
import sys
sys.path.append(os.path.dirname(__file__))

from abc import ABCMeta, abstractmethod, abstractproperty

def MM2Mils(MM):
    return (MM/25.4)*1000
    
class D2425(object):
    _width = 1750
    _length = 2250
    _height = 890
    _holes = { 0 : Base.Vector(185,875,0),\
               1 : Base.Vector(2065,875,0) }
    _hradius = 190/2.0
    def MountingHole(self,index,height=890,ZPanelOffset=0):
        p = self._holes[index]
        p = self.origin.add(p).add(Base.Vector(0,0,ZPanelOffset))
        return Part.Face(Part.Wire(Part.makeCircle(self._hradius,p))\
                        ).extrude(Base.Vector(0,0,height))
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name)
        obj.Shape = self.body
        obj.Label=self.name
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.body = Part.makePlane(self._length,self._width,self.origin\
                                  ).extrude(Base.Vector(0,0,self._height))
        self.body = self.body.cut(self.MountingHole(0))
        self.body = self.body.cut(self.MountingHole(1))
